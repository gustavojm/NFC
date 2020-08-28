#include "pole_tmr.h"
#include "pole.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdint.h"
#include "ad2s1210.h"
#include "pid.h"
#include "dout.h"
#include "relay.h"

#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )
#define POLE_SUPERVISOR_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t pole_queue = NULL;
SemaphoreHandle_t lock;

SemaphoreHandle_t pole_supervisor_semaphore;

static struct mot_pap_status status;
enum mot_pap_direction dir;

extern bool stall_detection;

struct ad2s1210_state rdc;
struct pid pid;

static void pole_task(void *par)
{
	struct mot_pap_msg *msg_rcv;
	int32_t error, threshold = 10;
	bool already_there;
	bool allowed, speed_ok;

	while (1) {
		if (xQueueReceive(pole_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			printf("pole: command received \n");

			if (msg_rcv->ctrlEn) {

				//obtener posición del RDC
				status.posAct = ad2s1210_read_position(&rdc);

				if (status.posAct > CWLIMIT) {
					status.cwLimit = 1;
				}

				if (status.posAct < CCWLIMIT) {
					status.ccwLimit = 1;
				}

				switch (msg_rcv->type) {
				case MOT_PAP_MSG_TYPE_FREE_RUNNING:
					allowed = movement_allowed(msg_rcv->free_run_direction,
							status.cwLimit, status.ccwLimit);
					speed_ok = free_run_speed_ok(msg_rcv->free_run_speed);

					if (allowed && speed_ok) {
						status.dir = msg_rcv->free_run_direction;
						dout_pole_dir(status.dir);
						status.vel = msg_rcv->free_run_speed
								* MOT_PAP_FREE_RUN_FREQ_MULTIPLIER;
						pole_tmr_set_freq(status.vel);
						pole_tmr_start();
					} else {
						if (!allowed)
							printf("pole: movement out of bounds %s",
									msg_rcv->free_run_direction
											== MOT_PAP_DIRECTION_CW ?
											"CW" : "CCW");
						if (!speed_ok)
							printf("pole: chosen speed out of bounds %i",
									msg_rcv->free_run_speed);
					}
					break;

				case MOT_PAP_MSG_TYPE_CLOSED_LOOP:	//PID
					status.posCmd = msg_rcv->closed_loop_setpoint;
					//calcular error de posición
					error = status.posCmd - status.posAct;
					already_there = (abs(error) < threshold);

					if (already_there) {
						pole_tmr_stop();
					} else {
						dir = direction_calculate(error);
						if (movement_allowed(dir, status.cwLimit,
								status.ccwLimit)) {
							status.dir = dir;
							dout_pole_dir(status.dir);
							//vTaskDelay(pdMS_TO_TICKS(0.08));	//80us required by parker compumotor
							status.vel = freq_calculate(&pid, status.posCmd,
									status.posAct);
							pole_tmr_set_freq(status.vel);

							if (!pole_tmr_started()) {
								pole_tmr_start();
							}
						} else {
							printf("pole: movement out of bounds %s",
									dir == MOT_PAP_DIRECTION_CW ? "CW" : "CCW");
						}
					}
					break;

				default:			//STOP
					pole_tmr_stop();
					break;
				}

			} else {
				printf("pole: command received with control disabled\n");
			}

			free(msg_rcv);
		} else {
			printf("pole: no command received \n");
		}
	}
}

static void supervisor_task(void *par)
{
	static int32_t last_pos = 0;
	int32_t stall_threshold = 10;
	int32_t error, threshold = 10;
	bool already_there;

	while (1) {
		xSemaphoreTake(pole_supervisor_semaphore, portMAX_DELAY);

		status.posAct = ad2s1210_read_position(&rdc);

		status.cwLimit = 0;
		status.ccwLimit = 0;

		if (((enum mot_pap_direction) status.dir == MOT_PAP_DIRECTION_CW)
				&& (status.posAct > CWLIMIT)) {
			status.cwLimit = 1;
			pole_tmr_stop();
			printf("pole: limit CW reached");
			continue;
		}

		if (((enum mot_pap_direction) status.dir == MOT_PAP_DIRECTION_CCW)
				&& (status.posAct < CCWLIMIT)) {
			status.ccwLimit = 1;
			pole_tmr_stop();
			printf("pole: limit CCW reached");
			continue;
		}

		if (stall_detection) {
			if (abs((abs(status.posAct) - abs(last_pos))) < stall_threshold) {
				//Pole stalled
				status.stalled = 1;
				pole_tmr_stop();
				relay_main_pwr(0);
				printf("pole: stalled");
				continue;
			}
		}
		last_pos = status.posAct;

		error = status.posCmd - status.posAct;
		already_there = (abs(error) < threshold);

		if (already_there) {
			pole_tmr_stop();
			printf("pole: llegamos");
		} else {
			status.dir = direction_calculate(error);
			dout_pole_dir(status.dir);
			//vTaskDelay(pdMS_TO_TICKS(0.08));	//80us required by parker compumotor
			status.vel = freq_calculate(&pid, status.posCmd, status.posAct);
			pole_tmr_set_freq(status.vel);
		}
	}
}

void pole_init()
{
	pole_queue = xQueueCreate(5, sizeof(struct mot_pap_msg*));

	pid_controller_init(&pid, 1, 200, 1, 1, 100, 5);

	status.dir = MOT_PAP_STATUS_STOP;
	status.posCmd = 0;
	status.posAct = 0;
	status.vel = 0;
	status.cwLimit = 0;
	status.ccwLimit = 0;

	rdc.gpios.reset = poncho_rdc_reset;
	rdc.gpios.sample = poncho_rdc_sample;
	rdc.gpios.wr_fsync = poncho_rdc_pole_wr_fsync;
	rdc.lock = lock;
	rdc.resolution = 12;

	ad2s1210_init(&rdc);

	pole_tmr_init();

	xTaskCreate(pole_task, "Pole", configMINIMAL_STACK_SIZE, NULL,
	POLE_TASK_PRIORITY, NULL);

	pole_supervisor_semaphore = xSemaphoreCreateBinary();

	if (pole_supervisor_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(supervisor_task, "PoleSupervisor",
		configMINIMAL_STACK_SIZE,
		NULL, POLE_SUPERVISOR_TASK_PRIORITY, NULL);
	}
}

struct mot_pap_status pole_get_status(void)
{
	return status;
}

