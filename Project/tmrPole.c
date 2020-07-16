/*
 * tmrPole.c
 *
 *  Created on: 1 jul. 2020
 *      Author: gustavo
 */
#include "stdio.h"
#include "math.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "pid.h"

#define LEFT	0
#define RIGHT	1

struct move_msg {
	uint32_t setpoint;
};

struct tmr_id {
	uint8_t	direction;
};

extern rdc_pole;

TimerHandle_t tmr_pole;
//static struct tmr_msg tmr_msg;

static struct tmr_id tmr_id_pole;

void pole_task(void *par)
{
	//Crear mensaje para poner en la cola
	//struct tmr_msg *tmr_msg = (struct tmr_msg *) malloc(sizeof(struct tmr_msg ));

	struct move_msg *rcv_pole_msg;
	int32_t setpoint, error, actual, thresshold;

	while (1) {
		if (xQueueReceive(queue_pole, (struct move_msg *) &rcv_pole_msg,
				(TickType_t) 10) != pdPASS) {
			printf("No Data received \n");
		} else {
			printf("Data received \n");			

			setpoint = rcv_pole_msg->setpoint;
			
			tmr_pole = xTimerCreate("TimerPole", pdMS_TO_TICKS(1), pdTRUE, (void*) &tmr_id_pole,
					tmr_pole_callback);
			free(rcv_pole_msg);
		}

		//obtener posición actual del RDC,
		actual = ad2s1210_read_position(rdc_pole);

		//calcular error de posición
		error = setpoint - actual;

		uint8_t llegamos;
		llegamos = (abs(error) < thresshold);

		if (( tmr_pole != NULL ) && llegamos) {
			xTimerStop(tmr_pole, 0);
			xTimerDelete(tmr_pole, 0);
		}
		

		if( tmr_pole != NULL ) {
			uint8_t direction;

			direction = error < 0 ? RIGHT : LEFT;
			tmr_id_pole.direction = direction;

			vTimerSetTimerID(tmr_pole, (void *) &tmr_id_pole);

			float cout;
			int32_t period;

			//calcular velocidad con PID(error)
			cout = pid_controller_calculate(pid_pole, setpoint, actual);
			period = (int32_t) nearbyintf(1.0f / cout);

			xTimerChangePeriod(tmr_pole, period, 0);

			if( xTimerIsTimerActive( tmr_pole ) == pdFALSE ) {
				if( xTimerStart( tmr_pole, 0 ) != pdPASS ) {
					//log que no anduvo
				}
			}

		}
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void tmr_pole_callback(TimerHandle_t tmr_handle)
{
	/* Optionally do something if the pxTimer parameter is NULL. */
	configASSERT(tmr_handle);

	/* Get the number of remaining pulses. */
	struct tmr_id *tmr_id_pole = (struct tmr_id *) pvTimerGetTimerID(tmr_handle);

	// Set the GPIO to indicate direction of movement
	// GPIO(GPIO_DIR_POLE, tmrIdPole->direction);

	// Toggle GPIO to generate waveform
	// GPIO_toggle(GPIO_PULSE_POLE);
}

