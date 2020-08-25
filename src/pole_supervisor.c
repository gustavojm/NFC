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
	bool cw_limit_reached = 0, ccw_limit_reached = 0;

	while (1) {
		xSemaphoreTake(pole_supervisor_semaphore, portMAX_DELAY);

		pos = pole_read_position();

		if ((pole_get_direction() == MOT_PAP_DIRECTION_CW) && (pos > CWLIMIT)) {
			cw_limit_reached = 1;
			pole_tmr_stop();
		}

		if ((pole_get_direction() == MOT_PAP_DIRECTION_CCW) && (pos < CCWLIMIT)) {
			ccw_limit_reached = 1;
			pole_tmr_stop();
		}

		pole_set_cw_limit_reached(cw_limit_reached);
		pole_set_ccw_limit_reached(ccw_limit_reached);

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

