#include <sys/times.h>
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
#include "pid.h"
#include "pole.h"

#define UP		0
#define DOWN	1
#define COMM_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

extern QueueHandle_t lift_queue;
extern QueueHandle_t pole_queue;

static void comm_task(void *par)
{
	bool dir = UP;
	struct lift_msg *lift_msg_snd;
	struct pole_msg *pole_msg_snd;

	while (1) {
		dir = !dir;
		lift_msg_snd = (struct lift_msg*) malloc(sizeof(struct lift_msg));
		lift_msg_snd->type = dir;
		if (xQueueSend(lift_queue, &lift_msg_snd,
				(TickType_t) 10) == pdPASS) {
			printf("comm: lift command sent \n");
		} else {
			printf("comm: unable to send lift command \n");
		}

		struct tms time;
		srandom(times(&time));

		pole_msg_snd = (struct pole_msg*) malloc(sizeof(struct pole_msg*));
		pole_msg_snd->closed_loop_setpoint = random() % ((2 ^ 16) - 1);
		if (xQueueSend(pole_queue, &pole_msg_snd,
				(TickType_t) 10) == pdPASS) {
			printf("comm: pole command sent \n");
		} else {
			printf("comm: unable to send pole command \n");
		}

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void comm_init()
{
	xTaskCreate(comm_task, "Comm", configMINIMAL_STACK_SIZE, NULL,
	COMM_TASK_PRIORITY, NULL);
}
/*-----------------------------------------------------------*/

