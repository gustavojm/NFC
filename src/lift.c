/*
 * lift.c
 *
 *  Created on: 1 jul. 2020
 *      Author: gustavo
 */

#include "../../inc/lift.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "stdbool.h"

#include "../../inc/pid.h"

#define UP		0
#define DOWN	1
#define LIFT_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t lift_queue = NULL;
SemaphoreHandle_t lift_interrupt_counting_semaphore;

static void lift_task(void *par)
{
	struct lift_msg *rcv_lift_msg;

	while (1) {

		if (xQueueReceive(lift_queue, (struct lift_msg*) &rcv_lift_msg,
				(TickType_t) 10) == pdPASS) {
			printf("lift: command received \n");

			free(rcv_lift_msg);

		} else {
			printf("lift: no command received \n");
		}
	}

	vTaskDelay(pdMS_TO_TICKS(200));
}

static void lift_limit_switches_handler_task(void *pvParameters)
{
	/* As per most tasks, this task is implemented within an infinite loop. */
	while (1) {
		/* Use the semaphore to wait for the event.  The semaphore was created
		 before the scheduler was started so before this task ran for the first
		 time.  The task blocks indefinitely meaning this function call will only
		 return once the semaphore has been successfully obtained - so there is
		 no need to check the returned value. */
		xSemaphoreTake(lift_interrupt_counting_semaphore, portMAX_DELAY);

		// GPIO detener LIFT

		/* To get here the event must have occurred.  Process the event (in this
		 case just print out a message). */
		printf("lift: limit switch alcanzado \n");
	}
}

//static uint32_t lift_limit_switches_interrupt_handler(void)
//{
//	BaseType_t xHigherPriorityTaskWoken;
//	xHigherPriorityTaskWoken = pdFALSE;
//	xSemaphoreGiveFromISR(lift_interrupt_counting_semaphore, &xHigherPriorityTaskWoken);
//	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//}

void lift_init()
{
	lift_queue = xQueueCreate(5, sizeof(struct lift_msg));

	//Configurar GPIO RELES DE LIFT como salidas digitales;
	//Configurar GPIO LIMIT SWITCH DE LIFT como entradas digitales;

	lift_interrupt_counting_semaphore = xSemaphoreCreateCounting(10, 0);

	if (lift_interrupt_counting_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(lift_limit_switches_handler_task, "LSHandler",
				configMINIMAL_STACK_SIZE, NULL, 3, NULL);

		/* Install the handler for the software interrupt.  The syntax necessary
		 to do this is dependent on the FreeRTOS port being used.  The syntax
		 shown here can only be used with the FreeRTOS Windows port, where such
		 interrupts are only simulated. */
		//vPortSetInterruptHandler(mainINTERRUPT_NUMBER, lift_limit_switches_interrupt_handler);
	}

	xTaskCreate(lift_task, "Lift", configMINIMAL_STACK_SIZE, NULL,
			LIFT_TASK_PRIORITY, NULL);
}
/*-----------------------------------------------------------*/

