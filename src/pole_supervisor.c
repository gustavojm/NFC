#include <pole_tmr.h>
#include "pole.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdint.h"
#include "relay.h"

#define CWLIMIT 	5000
#define CCWLIMIT 	5

#define POLE_SUPERVISOR_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

SemaphoreHandle_t pole_supervisor_semaphore;

extern bool stall_detection;

static void pole_supervisor_task(void *par)
{
	int32_t pos;
	static int32_t last_pos = 0;
	int32_t stall_threshold = 10;

	while (1) {
		xSemaphoreTake(pole_supervisor_semaphore, portMAX_DELAY);

		pos = pole_read_position();
		if (pos > CWLIMIT) {
			pole_set_limit_cw(1);
			pole_tmr_stop();
		}

		if (pos < CCWLIMIT) {
			pole_set_limit_ccw(1);
			pole_tmr_stop();
		}
		if (stall_detection) {
			if (abs((abs(pos) - abs(last_pos))) < stall_threshold) {
				//Pole stalled
				pole_set_stalled(1);
				pole_tmr_stop();
				relay_main_pwr(0);
			}
		}
		last_pos = pos;
	}
}

void pole_supervisor_init()
{
	pole_supervisor_semaphore = xSemaphoreCreateBinary();

	if (pole_supervisor_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(pole_supervisor_task, "PoleSupervisor", configMINIMAL_STACK_SIZE,
				NULL, POLE_SUPERVISOR_TASK_PRIORITY, NULL);
	}
}

