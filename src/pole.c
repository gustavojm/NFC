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

#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )
#define FREQ_MULTIPLIER  400

QueueHandle_t pole_queue = NULL;
SemaphoreHandle_t pole_lock;

extern bool stall_detection;

struct ad2s1210_state pole_rdc;
struct pid pole_pid;

static bool direction_calculate(int32_t error) {
	return error < 0 ? RIGHT : LEFT;
}

static float freq_calculate(int32_t setpoint, int32_t pos) {
	float cout, freq;
	cout = pid_controller_calculate(&pole_pid, setpoint, pos);
	printf("----COUT---- %f \n", cout);
	freq = (1.0f / cout) * FREQ_MULTIPLIER;
	printf("----FREQ---- %f \n", freq);
	return freq;
}

static void pole_task(void *par) {

	struct pole_msg *msg_rcv, *cmd_ptr = NULL;
	struct pole_msg cmd;
	int32_t error, pos, threshold, setpoint = INT32_MAX;
	bool llegamos, direction;

	while (1) {

		if (xQueueReceive(pole_queue, &msg_rcv, (TickType_t) 10) == pdPASS) {
			printf("pole: command received \n");

			cmd_ptr = &cmd;
			*cmd_ptr = *msg_rcv;	//Create local copy of msg_rcv
			free(msg_rcv);

		} else {
			printf("pole: no command received \n");
		}

		if (cmd_ptr != NULL) {
			switch (cmd_ptr->type) {
				case FREE_RUNNING:
					// GPIO(GPIO_DIR_POLE, cmd_ptr->free_run_direction);
					pole_tmr_set_freq(cmd_ptr->free_run_speed * FREQ_MULTIPLIER);
					pole_tmr_start();

					break;

				case CLOSED_LOOP:	//PID
					//obtener posición del RDC,
					pos = ad2s1210_read_position(&pole_rdc);

					//calcular error de posición
					error = cmd_ptr->setpoint - pos;
					llegamos = (abs(error) < threshold);

					if (llegamos) {
						pole_tmr_stop();
						cmd_ptr = NULL;
					} else {
						direction = direction_calculate(error);
						// Set the GPIO to indicate direction of movement
						// GPIO(GPIO_DIR_POLE, direction);
						//vTaskDelay(pdMS_TO_TICKS(0.08));	//80us required by parker compumotor

						pole_tmr_set_freq(freq_calculate(setpoint, pos));

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

void pole_init() {
	pole_queue = xQueueCreate(5, sizeof(struct pole_msg*));

	pid_controller_init(&pole_pid, 1, 200, 1, 1, 100, 5);

	//Configurar GPIO0,1,3 como salidas digitales;

	//rdc_pole.gpios.reset = GPIO0;
	//rdc_pole.gpios.sample = GPIO1;
	//rdc_pole.gpios.wr_fsync = GPIO3;
	pole_rdc.lock = pole_lock;
	pole_rdc.resolution = 12;

	ad2s1210_init(&pole_rdc);

	xTaskCreate(pole_task, "Pole", configMINIMAL_STACK_SIZE, NULL,
	POLE_TASK_PRIORITY, NULL);

}

