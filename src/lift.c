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

#define LIFT_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

QueueHandle_t lift_queue = NULL;

static struct lift lift;

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

			switch (msg_rcv->type) {
			case LIFT_TYPE_UP:
				if (lift.type == LIFT_TYPE_DOWN) {
					relay_lift_pwr(false);
					vTaskDelay(
							pdMS_TO_TICKS(LIFT_DIRECTION_CHANGE_DELAY_MS));
				}
				lift.type = LIFT_TYPE_UP;
				relay_lift_dir(LIFT_DIRECTION_UP);
				relay_lift_pwr(true);
				lDebug(Info, "lift: UP");
				break;
			case LIFT_TYPE_DOWN:
				if (lift.type == LIFT_TYPE_UP) {
					relay_lift_pwr(false);
					vTaskDelay(
							pdMS_TO_TICKS(LIFT_DIRECTION_CHANGE_DELAY_MS));
				}
				lift.type = LIFT_TYPE_DOWN;
				relay_lift_dir(LIFT_DIRECTION_DOWN);
				relay_lift_pwr(true);
				lDebug(Info, "lift: DOWN");
				break;
			default:
				lift.type = LIFT_TYPE_STOP;
				relay_lift_pwr(false);
				lDebug(Info, "lift: STOP");
				break;
			}

			vPortFree(msg_rcv);
		}
	}
}

/**
 * @brief 	creates the queues, semaphores and endless tasks to handle lift movements.
 * @return	nothing
 */
void lift_init()
{
	lift_queue = xQueueCreate(5, sizeof(struct lift_msg*));

	lift.type = LIFT_TYPE_STOP;
	lift.upLimit = false;
	lift.downLimit = false;

	xTaskCreate(lift_task, "Lift", configMINIMAL_STACK_SIZE, NULL,
	LIFT_TASK_PRIORITY, NULL);
	lDebug(Info, "lift: task created");
}

/**
 * @brief	returns status of the lift task.
 * @return 	copy of status structure of the task
 */
struct lift lift_status_get(void)
{
	return lift;
}
