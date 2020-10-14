#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "pole.h"
#include "ad2s1210.h"
#include "board.h"
#include "pid.h"
#include "dout.h"
#include "debug.h"
#include "tmr.h"

#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )
#define POLE_SUPERVISOR_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t pole_queue = NULL;

SemaphoreHandle_t pole_supervisor_semaphore;

static struct mot_pap pole;
static struct ad2s1210 rdc;
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

	while (true) {
		if (xQueueReceive(pole_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			lDebug(Info, "pole: command received");

			mot_pap_init_limits(&pole);

			switch (msg_rcv->type) {
			case MOT_PAP_TYPE_FREE_RUNNING:
				mot_pap_move_free_run(&pole, msg_rcv->free_run_direction,
						msg_rcv->free_run_speed);
				break;

			case MOT_PAP_TYPE_CLOSED_LOOP:
				mot_pap_move_closed_loop(&pole, msg_rcv->closed_loop_setpoint);
				break;

			default:
				mot_pap_stop(&pole);
				break;
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
	while (true) {
		xSemaphoreTake(pole_supervisor_semaphore, portMAX_DELAY);
		mot_pap_supervise(&pole);
	}
}

/**
 * @brief 	creates the queues, semaphores and endless tasks to handle pole movements.
 * @return	nothing
 */
void pole_init()
{
	pole_queue = xQueueCreate(5, sizeof(struct mot_pap_msg*));

	pole.name = "pole";
	pole.type = MOT_PAP_TYPE_STOP;
	pole.cwLimit = 65535;
	pole.ccwLimit = 0;
	pole.last_dir = MOT_PAP_DIRECTION_CW;
	pole.half_pulses = 0;
	pole.offset = 0;

	rdc.gpios.reset = &poncho_rdc_reset;
	rdc.gpios.sample = &poncho_rdc_sample;
	rdc.gpios.wr_fsync = &poncho_rdc_pole_wr_fsync;
	rdc.resolution = 16;
	ad2s1210_init(&rdc);

	pole.rdc = &rdc;

	pid_controller_init(&pid, 10, 20, 20, 20, 100);

	pole.pid = &pid;

	pole.gpios.direction = &dout_pole_dir;
	pole.gpios.pulse = &dout_pole_pulse;

	pole.tmr.started = false;
	pole.tmr.lpc_timer = LPC_TIMER0;
	pole.tmr.rgu_timer_rst = RGU_TIMER0_RST;
	pole.tmr.clk_mx_timer = CLK_MX_TIMER0;
	pole.tmr.timer_IRQn = TIMER0_IRQn;

	tmr_init(&pole.tmr);

	pole_supervisor_semaphore = xSemaphoreCreateBinary();
	pole.supervisor_semaphore = pole_supervisor_semaphore;

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
 * @brief	handle interrupt from 32-bit timer to generate pulses for the stepper motor drivers
 * @return	nothing
 * @note 	calls the supervisor task every x number of generated steps
 */
void TIMER0_IRQHandler(void)
{
	if (tmr_match_pending(&(pole.tmr))) {
		mot_pap_isr(&pole);
	}
}

/**
 * @brief	gets pole RDC position
 * @return	RDC position
 */
uint16_t pole_get_RDC_position()
{
	return ad2s1210_read_position(pole.rdc);
}


void pole_set_offset(uint16_t offset)
{
	pole.offset = offset;
}

/**
 * @brief	sets pole CW limit
 * @param 	pos		: RDC position where the limit is reached
 * @return	nothing
 */
void pole_set_cwLimit(uint16_t pos)
{
	pole.cwLimit = pos;
}

/**
 * @brief	sets pole CCW limit
 * @param 	pos		: RDC position where the limit is reached
 * @return	nothing
 */
void pole_set_ccwLimit(uint16_t pos)
{
	pole.ccwLimit = pos;
}

/**
 * @brief	returns status of the pole task.
 * @return 	copy of status structure of the task
 */
struct mot_pap pole_get_status(void)
{
	return pole;
}

