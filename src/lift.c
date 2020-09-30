#include <stdlib.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "lift.h"
#include "relay.h"
#include "debug.h"
#include "board.h"
#include "din.h"

#define LIFT_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t lift_queue = NULL;
static SemaphoreHandle_t lift_interrupt_counting_semaphore;

static struct lift_status status;

/**
 * @brief 	commands lift to go up if upLimit is not active.
 * @return	nothing
 */
static void lift_up()
{
	if (!status.upLimit) {
		relay_lift_dir(LIFT_DIRECTION_UP);
		relay_lift_pwr(true);
	} else {
		lDebug(Warn, "lift: limit switch reached, cannot go up");
	}
}

/**
 * @brief 	commands lift to go down if downLimit is not active.
 * @return	nothing
 */
static void lift_down()
{
	if (!status.downLimit) {
		relay_lift_dir(LIFT_DIRECTION_DOWN);
		relay_lift_pwr(true);
	} else {
		lDebug(Warn, "lift: limit switch reached, cannot go down");
	}
}

/**
 * @brief 	commands lift stop
 * @return	nothing
 */
static void lift_stop()
{
	relay_lift_pwr(false);
}

/**
 * @brief 	handles the Lift movement.
 * @param 	par		: unused
 * @return	never
 * @note	Receives commands from lift_queue
 */
static void lift_task(void *par)
{
	struct lift_msg *msg_rcv;

	while (true) {

		if (xQueueReceive(lift_queue, &msg_rcv, portMAX_DELAY) == pdPASS) {
			lDebug(Info, "lift: command received");

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
					lDebug(Info, "lift: UP");
					break;
				case LIFT_TYPE_DOWN:
					if (status.type == LIFT_TYPE_UP) {
						lift_stop();
						vTaskDelay(
								pdMS_TO_TICKS(LIFT_DIRECTION_CHANGE_DELAY_MS));
					}
					status.type = LIFT_TYPE_DOWN;
					lift_down();
					lDebug(Info, "lift: DOWN");
					break;
				default:
					status.type = LIFT_TYPE_STOP;
					lift_stop();
					lDebug(Info, "lift: STOP");
					break;
				}
			} else {
				lDebug(Warn, "lift: command received with control disabled");
			}

			free(msg_rcv);
		}
	}
}

/**
 * @brief 	ZS1_LIFT and ZS2_LIFT interrupt handlers defer execution to this task.
 * @param 	par	: unused
 * @return	never
 * @note	stops lift movement and prints which limit was reached
 */
static void lift_limit_switches_handler_task(void *par)
{
	/* As per most tasks, this task is implemented within an infinite loop. */
	while (true) {
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
			lDebug(Info, "lift: upper limit reached");
		}

		if (status.downLimit) {
			lDebug(Info, "lift: lower limit reached");
		}
	}
}

/**
 * @brief	IRQHandler for ZS1_LIFT.
 */
void GPIO0_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
	status.upLimit = true;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(lift_interrupt_counting_semaphore,
			&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief	IRQHandler for ZS2_LIFT.
 */
void GPIO1_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(1));
	status.downLimit = true;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(lift_interrupt_counting_semaphore,
			&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief 	creates the queues, semaphores and endless tasks to handle lift movements.
 * @return	nothing
 */
void lift_init()
{
	lift_queue = xQueueCreate(5, sizeof(struct lift_msg*));

	status.type = LIFT_TYPE_STOP;
	status.upLimit = false;
	status.downLimit = false;

	//Configurar GPIO LIMIT SWITCH DE LIFT como entradas digitales que generan interrupción por flanco ascendente;
	//Install the handler for the software interrupt.
	din_init();

	lift_interrupt_counting_semaphore = xSemaphoreCreateCounting(10, 0);

	if (lift_interrupt_counting_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(lift_limit_switches_handler_task, "LSHandler",
		configMINIMAL_STACK_SIZE, NULL, 3, NULL);
		lDebug(Info, "lift: limit switches task created");
	}

	xTaskCreate(lift_task, "Lift", configMINIMAL_STACK_SIZE, NULL,
	LIFT_TASK_PRIORITY, NULL);
	lDebug(Info, "lift: task created");
}

/**
 * @brief	returns status of the lift task.
 * @return 	copy of status structure of the task
 */
struct lift_status lift_status_get(void)
{
	return status;
}
