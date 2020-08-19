#include "pole.h"
#include "tmr_pole.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdint.h"
#include "ad2s1210.h"
#include "pid.h"
#include "relay.h"

#define POLE_STALL_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

SemaphoreHandle_t pole_stalled_semaphore;

extern struct pid pole_pid;
extern struct ad2s1210_state pole_rdc;

static void pole_stall_task(void *par)
{
	int32_t pos;
	static int32_t last_pos = 0;
	int32_t stall_threshold = 10;

	while (1) {
		xSemaphoreTake(pole_stalled_semaphore, portMAX_DELAY);

		pos = ad2s1210_read_position(&pole_rdc);
		if (abs((abs(pos) - abs(last_pos))) < stall_threshold) {
			//Pole stalled
			pole_tmr_stop();
			relay_main_pwr(0);
		}
		last_pos = pos;
	}
}

void pole_stall_init()
{
	pole_stalled_semaphore = xSemaphoreCreateBinary();

	if (pole_stalled_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(pole_stall_task, "PoleStall",
		configMINIMAL_STACK_SIZE, NULL, POLE_STALL_TASK_PRIORITY, NULL);
	}
}
/*-----------------------------------------------------------*/
