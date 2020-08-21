#include "lift.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "stdbool.h"
#include "relay.h"

#define LIFT_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t lift_queue = NULL;
SemaphoreHandle_t lift_interrupt_counting_semaphore;

static struct lift_status lift_status;

static void lift_task(void *par)
{
	struct lift_msg *msg_rcv;

	while (1) {

		if (xQueueReceive(lift_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			printf("lift: command received %s", (msg_rcv->type) ? "DOWN" : "UP");

			if (msg_rcv->ctrl_en) {
				switch (msg_rcv->type) {
				case LIFT_MSG_TYPE_UP:
					printf("UP \n");
					lift_status.dir = LIFT_STATUS_UP;
					lift_up();
					break;
				case LIFT_MSG_TYPE_DOWN:
					printf("DOWN \n");
					lift_status.dir = LIFT_STATUS_DOWN;
					lift_down();
					break;
				default:
					printf("STOP \n");
					lift_status.dir = LIFT_STATUS_STOP;
					lift_stop();
					break;
				}
			} else {
				printf("lift: command received with control disabled");
			}

			free(msg_rcv);

		} else {
			printf("lift: no command received \n");
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
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

		// Detener LIFT
		relay_lift_pwr(0);

		//Determinar cual de los límites se accionó

		//status.ls_up = 1;
		//status.ls_down = 1;

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
	lift_queue = xQueueCreate(5, sizeof(struct lift_msg*));

	lift_status.dir = LIFT_STATUS_STOP;
	lift_status.limitUp = 0;
	lift_status.limitDown = 0;

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

lift_status lift_status_get(void) {
	return lift_status;
}

/*-----------------------------------------------------------*/

static void lift_up()
{
	if (!lift_status.ls_up) {
		relay_lift_dir(1);
		relay_lift_pwr(1);
	} else {
		printf("lift: limit switch reached, cannot go up \n");
	}
}

static void lift_down()
{
	if (!lift_status.ls_down) {
	relay_lift_dir(0);
	relay_lift_pwr(1);
	} else {
		printf("lift: limit switch reached, cannot go down \n");
	}
}

void lift_stop()
{
	relay_lift_pwr(0);
}
