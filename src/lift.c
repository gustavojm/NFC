#include "board.h"
#include "din.h"
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
#include "debug.h"

#define LIFT_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t lift_queue = NULL;
SemaphoreHandle_t lift_interrupt_counting_semaphore;

static struct lift_status status;

static void lift_up()
{
	if (!status.limitUp) {
		relay_lift_dir(1);
		relay_lift_pwr(1);
	} else {
		lDebug(Warn, "lift: limit switch reached, cannot go up \n");
	}
}

static void lift_down()
{
	if (!status.limitDown) {
		relay_lift_dir(0);
		relay_lift_pwr(1);
	} else {
		lDebug(Warn, "lift: limit switch reached, cannot go down \n");
	}
}

static void lift_task(void *par)
{
	struct lift_msg *msg_rcv;

	while (1) {

		if (xQueueReceive(lift_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			lDebug(Info, "lift: command received");

			if (msg_rcv->ctrlEn) {
				status.limitUp = din_zs1_lift_state();
				status.limitDown = din_zs2_lift_state();

				switch (msg_rcv->type) {
				case LIFT_MSG_TYPE_UP:
					lDebug(Info, "lift: UP \n");
					status.dir = LIFT_STATUS_UP;
					lift_up();
					break;
				case LIFT_MSG_TYPE_DOWN:
					lDebug(Info, "lift: DOWN \n");
					status.dir = LIFT_STATUS_DOWN;
					lift_down();
					break;
				default:
					lDebug(Info, "lift: STOP \n");
					status.dir = LIFT_STATUS_STOP;
					lift_stop();
					break;
				}
			} else {
				lDebug(Warn, "lift: command received with control disabled");
			}

			free(msg_rcv);
		}
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

		if (status.limitUp) {
			lDebug(Info, "lift: limit switch superior alcanzado \n");
		}

		if (status.limitDown) {
			lDebug(Info, "lift: limit switch inferior alcanzado \n");
		}
	}
}

//IRQHandler for ZS1_LIFT
void GPIO0_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
	status.limitUp = 1;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(lift_interrupt_counting_semaphore,
			&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//IRQHandler for ZS2_LIFT
void GPIO1_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(1));
	status.limitDown = 1;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(lift_interrupt_counting_semaphore,
			&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void lift_init()
{
	lift_queue = xQueueCreate(5, sizeof(struct lift_msg*));

	status.dir = LIFT_STATUS_STOP;
	status.limitUp = 0;
	status.limitDown = 0;

	//Configurar GPIO LIMIT SWITCH DE LIFT como entradas digitales que generan interrupción por flanco descendiente;
	//Install the handler for the software interrupt.
	din_init();

	lift_interrupt_counting_semaphore = xSemaphoreCreateCounting(10, 0);

	if (lift_interrupt_counting_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(lift_limit_switches_handler_task, "LSHandler",
		configMINIMAL_STACK_SIZE, NULL, 3, NULL);


	}

	xTaskCreate(lift_task, "Lift", configMINIMAL_STACK_SIZE, NULL,
	LIFT_TASK_PRIORITY, NULL);
}

struct lift_status lift_status_get(void)
{
	return status;
}

void lift_stop()
{
	relay_lift_pwr(0);
}
