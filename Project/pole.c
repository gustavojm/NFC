/*
 * pole.c
 *
 *  Created on: 1 jul. 2020
 *      Author: gustavo
 */

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "pid.h"
#include "pole.h"
#include "ad2s1210.h"
#include "stdint.h"

#define LEFT	0
#define RIGHT	1
#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t pole_queue = NULL;
SemaphoreHandle_t pole_lock;
TimerHandle_t pole_tmr = NULL;

struct ad2s1210_state pole_rdc;
struct pid pole_pid;
static struct pole_tmr_id pole_tmr_id;

static bool direction_calculate(int32_t error)
{
	return error < 0 ? RIGHT : LEFT;
}

static int32_t period_calculate(int32_t setpoint, int32_t pos)
{
	float cout;
	cout = pid_controller_calculate(&pole_pid, setpoint, pos);
	return (int32_t) nearbyint(1.0f / cout);
}

static void tmr_pole_callback(TimerHandle_t tmr_handle)
{
	int32_t pos;
	static int32_t last_pos = 0;
	int32_t stall_threshold = 10;

	/* Optionally do something if the pxTimer parameter is NULL. */
	configASSERT(tmr_handle);

	/* Get the number of remaining pulses. */
	struct pole_tmr_id *tmr_id_pole = (struct pole_tmr_id*) pvTimerGetTimerID(
			tmr_handle);

	// Set the GPIO to indicate direction of movement
	// GPIO(GPIO_DIR_POLE, tmrIdPole->direction);

	// Toggle GPIO to generate waveform
	// GPIO_toggle(GPIO_PULSE_POLE);

	if (tmr_id_pole->stall_detection) {
		pos = ad2s1210_read_position(&pole_rdc);
		if (abs((abs(pos) - abs(last_pos))) < stall_threshold) {
			//Pole stalled
			xTimerStop(pole_tmr, 0);
			xTimerDelete(pole_tmr, 0);
			pole_tmr = NULL;

			// GPIO(GPIO_MAIN_RELE, OFF);
		}
		last_pos = pos;
	}

}

static void pole_task(void *par)
{
	//Crear mensaje para poner en la cola
	//struct tmr_msg *tmr_msg = (struct tmr_msg *) malloc(sizeof(struct tmr_msg ));

	struct pole_msg *rcv_pole_msg;
	int32_t error, pos, threshold, setpoint = INT32_MAX;
	bool llegamos;
	bool stall_detection;

	threshold = 5;

	while (1) {

		if (pole_tmr == NULL) {		// Si no hay un timer moviendo el motor
			if (xQueueReceive(pole_queue, (struct pole_msg*) &rcv_pole_msg,
					(TickType_t) 10) == pdPASS) {
				printf("pole: command received \n");

				setpoint = rcv_pole_msg->setpoint;
				stall_detection = rcv_pole_msg->stall_detection;

				pole_tmr = xTimerCreate("TimerPole", pdMS_TO_TICKS(1), pdTRUE,
						(void*) &pole_tmr_id, tmr_pole_callback);
				free(rcv_pole_msg);

			} else {
				printf("pole: no command received \n");
			}
		}

		if (setpoint != INT32_MAX) {
			//obtener posición del RDC,
			pos = ad2s1210_read_position(&pole_rdc);

			//calcular error de posición
			error = setpoint - pos;
			llegamos = (abs(error) < threshold);

			if (pole_tmr != NULL) {
				if (llegamos) {
					xTimerStop(pole_tmr, 0);
					xTimerDelete(pole_tmr, 0);
					pole_tmr = NULL;
					setpoint = INT32_MAX;
				} else {
					pole_tmr_id.direction = direction_calculate(error);
					pole_tmr_id.stall_detection = stall_detection;

					vTimerSetTimerID(pole_tmr, (void*) &pole_tmr_id);
					xTimerChangePeriod(pole_tmr,
							period_calculate(setpoint, pos), 0);

					if (xTimerIsTimerActive(pole_tmr) == pdFALSE) {
						if ( xTimerStart( pole_tmr, 0 ) != pdPASS) {
							printf("pole: unable to start timer \n");
						}
					}
				}
			}
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

	void pole_init()
	{
		pole_queue = xQueueCreate(5, sizeof(struct pole_msg));

		pole_pid.kp = 1;
		pole_pid.ki = 1;
		pole_pid.kd = 1;
		pole_pid.limit = 100;

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

