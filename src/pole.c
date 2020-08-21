#include "pole.h"
#include "tmr_pole.h"
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
SemaphoreHandle_t pole_lock;
static volatile uint32_t last_pole_moved_tick;

static struct pole_status pole_status;

extern bool stall_detection;

struct ad2s1210_state pole_rdc;
struct pid pole_pid;

static bool direction_calculate(int32_t error)
{
	return error < 0 ? RIGHT : LEFT;
}

static float freq_calculate(int32_t setpoint, int32_t pos)
{
	float cout;
	float freq;

	cout = pid_controller_calculate(&pole_pid, setpoint, pos);
	printf("----COUT---- %f \n", cout);
	freq = cout * FREQ_MULTIPLIER;
	printf("----FREQ---- %f \n", freq);
	return freq;
}

static void pole_task(void *par)
{

	struct pole_msg *msg_rcv, *cmd_ptr = NULL;
	struct pole_msg cmd;
	int32_t error, pos, threshold = 10;
	bool llegamos;

	while (1) {
		if (xQueueReceive(pole_queue, &msg_rcv, (TickType_t) 10) == pdPASS) {
			printf("pole: command received \n");

			cmd_ptr = &cmd;
			*cmd_ptr = *msg_rcv;	//Create local copy of msg_rcv
			free(msg_rcv);

			if (!cmd_ptr->ctr_en) {
				printf("pole: no command received with control disabled\n");
			}
		} else {
			printf("pole: no command received \n");
		}

		//obtener posición del RDC
		status.posActPole = ad2s1210_read_position(&pole_rdc);

		if (cmd_ptr != NULL) {
			switch (cmd_ptr->type) {
			case POLE_MSG_TYPE_FREE_RUNNING:
				pole_status.dirPole = cmd_ptr->free_run_direction;
				dout_pole_dir(pole_status.dirPole);
				pole_status.velPole = cmd_ptr->free_run_speed * FREQ_MULTIPLIER;
				pole_tmr_set_freq(pole_status.velPole);
				pole_tmr_start();
				break;

			case POLE_MSG_TYPE_CLOSED_LOOP:	//PID
				//calcular error de posición
				error = cmd_ptr->closed_loop_setpoint - pole_status.posActPole;
				llegamos = (abs(error) < threshold);

				if (llegamos) {
					pole_tmr_stop();
					cmd_ptr = NULL;
				} else {
					pole_status.dirPole = direction_calculate(error);
					dout_pole_dir(pole_status.dirPole);
					//vTaskDelay(pdMS_TO_TICKS(0.08));	//80us required by parker compumotor

					pole_status.velPole = freq_calculate(cmd_ptr->closed_loop_setpoint, pole_status.posActPole);
					pole_tmr_set_freq(pole_status.velPole);

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
	pole_queue = xQueueCreate(5, sizeof(struct pole_msg*));

	pid_controller_init(&pole_pid, 1, 200, 1, 1, 100, 5);

	pole_status.dirPole = POLE_STATUS_STOP;
	pole_status.posCmdPole = 0;
	pole_status.posActPole = 0;
	pole_status.velPole = 0;
	pole_status.cwLimitPole = 0;
	pole_status.ccwLimitPole = 0;


	pole_rdc.gpios.reset = poncho_rdc_reset;
	pole_rdc.gpios.sample = poncho_rdc_sample;
	pole_rdc.gpios.wr_fsync = poncho_rdc_pole_wr_fsync;
	pole_rdc.lock = pole_lock;
	pole_rdc.resolution = 12;

	ad2s1210_init(&pole_rdc);

	xTaskCreate(pole_task, "Pole", configMINIMAL_STACK_SIZE, NULL,
	POLE_TASK_PRIORITY, NULL);

}

pole_status pole_status_get(void) {
	return pole_status;
}


