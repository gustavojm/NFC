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
	if (!status.upLimit) {
		relay_lift_dir(LIFT_DIRECTION_UP);
		relay_lift_pwr(1);
	} else {
		lDebug(Warn, "lift: limit switch reached, cannot go up \n");
	}
}

static void lift_down()
{
	if (!status.downLimit) {
		relay_lift_dir(LIFT_DIRECTION_DOWN);
		relay_lift_pwr(1);
	} else {
		lDebug(Warn, "lift: limit switch reached, cannot go down \n");
	}
}

static void lift_stop()
{
	relay_lift_pwr(0);
}

static void lift_task(void *par)
{
	struct lift_msg *msg_rcv;

	while (1) {

		if (xQueueReceive(lift_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			lDebug(Info, "lift: command received \n");

			if (msg_rcv->ctrlEn) {
				status.upLimit = din_zs1_lift_state();
				status.downLimit = din_zs2_lift_state();

				switch (msg_rcv->type) {
				case LIFT_TYPE_UP:
					if (status.type == LIFT_TYPE_DOWN) {
						lift_stop();
						vTaskDelay(
								pdMS_TO_TICKS(LIFT_DIRECTION_CHANGE_DELAY_MS));
					}
					status.type = LIFT_TYPE_UP;
					lift_up();
					lDebug(Info, "lift: UP \n");
					break;
				case LIFT_TYPE_DOWN:
					if (status.type == LIFT_TYPE_UP) {
						lift_stop();
						vTaskDelay(
								pdMS_TO_TICKS(LIFT_DIRECTION_CHANGE_DELAY_MS));
					}
					status.type = LIFT_TYPE_DOWN;
					lift_down();
					lDebug(Info, "lift: DOWN \n");
					break;
				default:
					status.type = LIFT_TYPE_STOP;
					lift_stop();
					lDebug(Info, "lift: STOP \n");
					break;
				}
			} else {
				lDebug(Warn, "lift: command received with control disabled \n");
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
		lift_stop();

		//Determinar cual de los límites se accionó

		if (status.upLimit) {
			lDebug(Info, "lift: upper limit reached \n");
		}

		if (status.downLimit) {
			lDebug(Info, "lift: lower limit reached \n");
		}
	}
}

//IRQHandler for ZS1_LIFT
void GPIO0_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
	status.upLimit = 1;
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
	status.downLimit = 1;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(lift_interrupt_counting_semaphore,
			&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void lift_init()
{
	lift_queue = xQueueCreate(5, sizeof(struct lift_msg*));

	status.type = LIFT_TYPE_STOP;
	status.upLimit = 0;
	status.downLimit = 0;

	//Configurar GPIO LIMIT SWITCH DE LIFT como entradas digitales que generan interrupción por flanco descendiente;
	//Install the handler for the software interrupt.
	din_init();

	lift_interrupt_counting_semaphore = xSemaphoreCreateCounting(10, 0);

	if (lift_interrupt_counting_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(lift_limit_switches_handler_task, "LSHandler",
		configMINIMAL_STACK_SIZE, NULL, 3, NULL);
		lDebug(Info, "lift: limit switches task created \n");
	}

	xTaskCreate(lift_task, "Lift", configMINIMAL_STACK_SIZE, NULL,
	LIFT_TASK_PRIORITY, NULL);
	lDebug(Info, "lift: task created \n");
}

struct lift_status lift_status_get(void)
{
	return status;
}
