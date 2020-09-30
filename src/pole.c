#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pole.h"
#include "ad2s1210.h"
#include "pid.h"
#include "dout.h"
#include "relay.h"
#include "debug.h"
#include "pole_tmr.h"

#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )
#define POLE_SUPERVISOR_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t pole_queue = NULL;

SemaphoreHandle_t pole_supervisor_semaphore;

static struct mot_pap_status status;

extern bool stall_detection;

static struct ad2s1210_state rdc;
static struct pid pid;

/**
 * @brief 	handles the Pole movement.
 * @param 	par		: unused
 * @return	never
 * @note	Receives commands from pole_queue
 */
static void pole_task(void *par)
{
	struct mot_pap_msg *msg_rcv;
	int32_t error, threshold = 10;
	bool already_there;
	bool allowed, speed_ok;
	enum mot_pap_direction dir;

	ad2s1210_init(&rdc);

	while (true) {
		if (xQueueReceive(pole_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			lDebug(Info, "pole: command received");

			if (msg_rcv->ctrlEn) {
				status.stalled = false;	// If a new command was received with ctrlEn=1 assume we are not stalled

				//obtener posición del RDC
				status.posAct = ad2s1210_read_position(&rdc);
				status.cwLimitReached = false;
				status.ccwLimitReached = false;

				if (status.posAct >= status.cwLimit) {
					status.cwLimitReached = true;
				}

				if (status.posAct <= status.ccwLimit) {
					status.ccwLimitReached = true;
				}

				switch (msg_rcv->type) {
				case MOT_PAP_TYPE_FREE_RUNNING:
					allowed = movement_allowed(msg_rcv->free_run_direction,
							status.cwLimitReached, status.ccwLimitReached);
					speed_ok = free_run_speed_ok(msg_rcv->free_run_speed);

					if (allowed && speed_ok) {
						if ((status.dir != msg_rcv->free_run_direction)
								&& (status.type != MOT_PAP_TYPE_STOP)) {
							pole_tmr_stop();
							vTaskDelay(
									pdMS_TO_TICKS(
											MOT_PAP_DIRECTION_CHANGE_DELAY_MS));
						}
						status.type = MOT_PAP_TYPE_FREE_RUNNING;
						status.dir = msg_rcv->free_run_direction;
						dout_pole_dir(status.dir);
						status.freq = msg_rcv->free_run_speed
								* MOT_PAP_FREE_RUN_FREQ_MULTIPLIER;
						pole_tmr_set_freq(status.freq);
						pole_tmr_start();
						lDebug(Info, "pole: FREE RUN, speed: %u, direction: %s",
								status.freq,
								status.dir == MOT_PAP_DIRECTION_CW ?
										"CW" : "CCW");
					} else {
						if (!allowed)
							lDebug(Warn, "pole: movement out of bounds %s",
									msg_rcv->free_run_direction
											== MOT_PAP_DIRECTION_CW ?
											"CW" : "CCW");
						if (!speed_ok)
							lDebug(Warn, "pole: chosen speed out of bounds %u",
									msg_rcv->free_run_speed);
					}
					break;

				case MOT_PAP_TYPE_CLOSED_LOOP:	//PID
					if ((msg_rcv->closed_loop_setpoint > status.cwLimit)
							| (msg_rcv->closed_loop_setpoint < status.ccwLimit)) {
						lDebug(Warn, "pole: movement out of bounds");
					} else {
						status.posCmd = msg_rcv->closed_loop_setpoint;
						lDebug(Info, "pole: CLOSED_LOOP posCmd: %u posAct: %u",
								status.posCmd, status.posAct);

						//calcular error de posición
						error = status.posCmd - status.posAct;
						already_there = (abs(error) < threshold);

						if (already_there) {
							pole_tmr_stop();
							lDebug(Info, "pole: already there");
						} else {
							dir = direction_calculate(error);
							if (movement_allowed(dir, status.cwLimitReached,
									status.ccwLimitReached)) {
								if ((status.dir != dir)
										&& (status.type != MOT_PAP_TYPE_STOP)) {
									pole_tmr_stop();
									vTaskDelay(
											pdMS_TO_TICKS(
													MOT_PAP_DIRECTION_CHANGE_DELAY_MS));
								}
								status.type = MOT_PAP_TYPE_CLOSED_LOOP;
								status.dir = dir;
								dout_pole_dir(status.dir);
								status.freq = freq_calculate(&pid,
										status.posCmd, status.posAct);
								pole_tmr_set_freq(status.freq);
								lDebug(Info,
										"pole: CLOSED LOOP, speed: %u, direction: %s",
										status.freq,
										status.dir == MOT_PAP_DIRECTION_CW ?
												"CW" : "CCW");
								if (!pole_tmr_started()) {
									pole_tmr_start();
								}
							} else {
								lDebug(Warn, "pole: movement out of bounds %s",
										dir == MOT_PAP_DIRECTION_CW ?
												"CW" : "CCW");
							}
						}
					}
					break;

				default:			//STOP
					status.type = MOT_PAP_TYPE_STOP;
					pole_tmr_stop();
					lDebug(Info, "pole: STOP");
					break;
				}

			} else {
				lDebug(Warn, "pole: command received with control disabled");
			}

			free(msg_rcv);
		}
	}
}

/**
 * @brief	checks if soft limits are reached, if stalled and if position reached in closed loop.
 * @param 	par	: unused
 * @return	never
 */
static void supervisor_task(void *par)
{
	static uint16_t last_pos = 0;
	int32_t error;
	bool already_there;
	enum mot_pap_direction dir;

	while (true) {
		xSemaphoreTake(pole_supervisor_semaphore, portMAX_DELAY);

		status.posAct = ad2s1210_read_position(&rdc);

		status.cwLimitReached = false;
		status.ccwLimitReached = false;

		if ((status.dir == MOT_PAP_DIRECTION_CW)
				&& (status.posAct >= (int32_t) status.cwLimit)) {
			status.cwLimitReached = true;
			pole_tmr_stop();
			lDebug(Warn, "pole: limit CW reached");
			goto cont;
		}

		if ((status.dir == MOT_PAP_DIRECTION_CCW)
				&& (status.posAct <= (int32_t) status.ccwLimit)) {
			status.ccwLimitReached = true;
			pole_tmr_stop();
			lDebug(Warn, "pole: limit CCW reached");
			goto cont;
		}

		if (stall_detection) {
			lDebug(Info, "STALL DETECTION posAct: %u, last_pos: %u",
					status.posAct, last_pos);
			if (abs((int) (status.posAct - last_pos)) < MOT_PAP_STALL_THRESHOLD) {
				status.stalled = true;
				pole_tmr_stop();
				relay_main_pwr(0);
				lDebug(Warn, "pole: stalled");
				goto cont;
			}
		}

		if (status.type == MOT_PAP_TYPE_CLOSED_LOOP) {
			error = status.posCmd - status.posAct;
			already_there = (abs((int) error) < MOT_PAP_POS_THRESHOLD);

			if (already_there) {
				status.type = MOT_PAP_TYPE_STOP;
				pole_tmr_stop();
				lDebug(Info, "pole: position reached");
			} else {
				dir = direction_calculate(error);
				if (status.dir != dir) {
					pole_tmr_stop();
					vTaskDelay(
							pdMS_TO_TICKS(MOT_PAP_DIRECTION_CHANGE_DELAY_MS));
					status.dir = dir;
					dout_pole_dir(status.dir);
					pole_tmr_start();
				}
				status.freq = freq_calculate(&pid, status.posCmd,
						status.posAct);
				pole_tmr_set_freq(status.freq);
			}
		}
cont:
		last_pos = status.posAct;
	}
}

/**
 * @brief 	creates the queues, semaphores and endless tasks to handle pole movements.
 * @return	nothing
 */
void pole_init()
{
	pole_queue = xQueueCreate(5, sizeof(struct mot_pap_msg*));

	pid_controller_init(&pid, 10, 20, 20, 20, 100);

	status.type = MOT_PAP_TYPE_STOP;
	status.cwLimit = 65535;
	status.ccwLimit = 0;

	rdc.gpios.reset = poncho_rdc_reset;
	rdc.gpios.sample = poncho_rdc_sample;
	rdc.gpios.wr_fsync = poncho_rdc_pole_wr_fsync;
	rdc.resolution = 16;

	pole_tmr_init();

	pole_supervisor_semaphore = xSemaphoreCreateBinary();

	if (pole_supervisor_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(supervisor_task, "PoleSupervisor",
		configMINIMAL_STACK_SIZE,
		NULL, POLE_SUPERVISOR_TASK_PRIORITY, NULL);
		lDebug(Info, "pole: supervisor task created");
	}

	xTaskCreate(pole_task, "Pole", configMINIMAL_STACK_SIZE, NULL,
	POLE_TASK_PRIORITY, NULL);

	lDebug(Info, "pole: task created");
}

/**
 * @brief	gets pole RDC position
 * @return	RDC position
 */
uint16_t pole_get_RDC_position()
{
	return ad2s1210_read_position(&rdc);
}

/**
 * @brief	sets pole CW limit
 * @param 	pos		: RDC position where the limit is reached
 * @return	nothing
 */
void pole_set_cwLimit(uint16_t pos)
{
	status.cwLimit = pos;
}

/**
 * @brief	sets pole CCW limit
 * @param 	pos		: RDC position where the limit is reached
 * @return	nothing
 */
void pole_set_ccwLimit(uint16_t pos)
{
	status.ccwLimit = pos;
}

/**
 * @brief	returns status of the pole task.
 * @return 	copy of status structure of the task
 */
struct mot_pap_status pole_get_status(void)
{
	return status;
}

