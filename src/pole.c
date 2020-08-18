/*
 * pole.c
 *
 *  Created on: 1 jul. 2020
 *      Author: gustavo
 */

#include "pole.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "stdint.h"
#include "ad2s1210.h"
#include "pid.h"

#define POLE_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )
#define FREQ_MULTIPLIER  400

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

static float period_calculate(int32_t setpoint, int32_t pos)
{
	float cout, period;
	cout = pid_controller_calculate(&pole_pid, setpoint, pos);
	printf ("----COUT---- %f \n", cout);
	period = (1.0f / cout) * FREQ_MULTIPLIER;
	printf ("----PERIOD---- %f \n", period);
	return period;
}

static void pole_tmr_callback(TimerHandle_t tmr_handle)
{
	int32_t pos;
	static int32_t last_pos = 0;
	int32_t stall_threshold = 10;

	/* Optionally do something if the pxTimer parameter is NULL. */
	configASSERT(tmr_handle);

	struct pole_tmr_id *tmr_id_pole = (struct pole_tmr_id*) pvTimerGetTimerID(
			tmr_handle);

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

	struct pole_msg *msg_rcv;
	int32_t error, pos, threshold, setpoint = INT32_MAX;
	bool llegamos, direction;

	threshold = 5;

	while (1) {

		if (pole_tmr == NULL) {		// Si no hay un timer moviendo el motor
			if (xQueueReceive(pole_queue, &msg_rcv,
					(TickType_t) 10) == pdPASS) {
				printf("pole: command received \n");

				setpoint = msg_rcv->setpoint;

				pole_tmr_id.stall_detection = msg_rcv->stall_detection;

				pole_tmr = xTimerCreate("TimerPole", pdMS_TO_TICKS(1), pdTRUE,
						(void*) &pole_tmr_id, pole_tmr_callback);
				free(msg_rcv);

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
					direction = direction_calculate(error);
					// Set the GPIO to indicate direction of movement
					// GPIO(GPIO_DIR_POLE, direction);
					//vTaskDelay(pdMS_TO_TICKS(0.08));	//80us required by parker compumotor

					vTimerSetTimerID(pole_tmr, (void*) &pole_tmr_id);

					printf("AAAATICKSAAA %i \n", pdMS_TO_TICKS(period_calculate(setpoint, pos)));
					printf("configTICK_RATE_HZ %i" ,configTICK_RATE_HZ);
					xTimerChangePeriod(pole_tmr,
							pdMS_TO_TICKS(period_calculate(setpoint, pos)), 0);

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
	pole_queue = xQueueCreate(5, sizeof(struct pole_msg *));

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

