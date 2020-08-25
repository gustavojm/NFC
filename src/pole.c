#include <pole_tmr.h>
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

#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )
#define FREQ_MULTIPLIER  400

QueueHandle_t pole_queue = NULL;
SemaphoreHandle_t lock;

static struct mot_pap_status status;

extern bool stall_detection;

struct ad2s1210_state rdc;
struct pid pid;

static enum mot_pap_direction direction_calculate(int32_t error)
{
	return error < 0 ? MOT_PAP_DIRECTION_CW : MOT_PAP_DIRECTION_CCW;
}

static float freq_calculate(int32_t setpoint, int32_t pos)
{
	float cout;
	float freq;

	cout = pid_controller_calculate(&pid, setpoint, pos);
	printf("----COUT---- %f \n", cout);
	freq = cout * FREQ_MULTIPLIER;
	printf("----FREQ---- %f \n", freq);
	return freq;
}

static void pole_task(void *par)
{

	struct mot_pap_msg *msg_rcv, *cmd_ptr = NULL;
	struct mot_pap_msg cmd;
	int32_t error, threshold = 10;
	bool llegamos;

	while (1) {
		if (xQueueReceive(pole_queue, &msg_rcv, (TickType_t) 10) == pdPASS) {
			printf("pole: command received \n");

			cmd_ptr = &cmd;
			*cmd_ptr = *msg_rcv;	//Create local copy of msg_rcv
			free(msg_rcv);

			if (!cmd_ptr->ctrlEn) {
				printf("pole: no command received with control disabled\n");
			}
		} else {
			printf("pole: no command received \n");
		}

		//obtener posición del RDC
		status.posAct = pole_read_position();

		if (cmd_ptr != NULL) {
			switch (cmd_ptr->type) {
			case MOT_PAP_MSG_TYPE_FREE_RUNNING:
				status.dir = cmd_ptr->free_run_direction;
				dout_pole_dir(status.dir);
				status.vel = cmd_ptr->free_run_speed * FREQ_MULTIPLIER;
				pole_tmr_set_freq(status.vel);
				pole_tmr_start();
				break;

			case MOT_PAP_MSG_TYPE_CLOSED_LOOP:	//PID
				//calcular error de posición
				error = cmd_ptr->closed_loop_setpoint - status.posAct;
				llegamos = (abs(error) < threshold);

				if (llegamos) {
					pole_tmr_stop();
					cmd_ptr = NULL;
				} else {
					status.dir = direction_calculate(error);
					dout_pole_dir(status.dir);
					//vTaskDelay(pdMS_TO_TICKS(0.08));	//80us required by parker compumotor

					status.vel = freq_calculate(cmd_ptr->closed_loop_setpoint,
							status.posAct);
					pole_tmr_set_freq(status.vel);

					if (!pole_tmr_started()) {
						pole_tmr_start();
					}
				}
				break;

			default:			//STOP
				pole_tmr_stop();
				cmd_ptr = NULL;
				break;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(200));
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
}

inline void pole_set_cw_limit_reached(bool state)
{
	status.cwLimit = state;
}

inline void pole_set_ccw_limit_reached(bool state)
{
	status.ccwLimit = state;
}

void pole_set_stalled(bool state)
{
	status.stalled = state;
}

enum mot_pap_direction pole_get_direction(void)
{
	return status.dir;
}

struct mot_pap_status pole_get_status(void)
{
	return status;
}

int32_t pole_read_position(void)
{
	return ad2s1210_read_position(&rdc);
}
