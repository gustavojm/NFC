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

#define CWLIMIT 	5000
#define CCWLIMIT 	5

#define POLE_STALL_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

SemaphoreHandle_t pole_supervisor_semaphore;

extern struct pid pole_pid;
extern struct ad2s1210_state pole_rdc;
extern struct pole_status pole_status;
extern bool stall_detection;

static void pole_stall_task(void *par)
{
	int32_t pos;
	static int32_t last_pos = 0;
	int32_t stall_threshold = 10;

	while (1) {
		xSemaphoreTake(pole_supervisor_semaphore, portMAX_DELAY);

		pos = ad2s1210_read_position(&pole_rdc);
		if (pos > CWLIMIT) {
			pole_status.cwLimitPole = 1;
			pole_tmr_stop();
		}

		if (pos < CCWLIMIT) {
			pole_status.ccwLimitPole = 1;
			pole_tmr_stop();
		}
		if (stall_detection) {
			if (abs((abs(pos) - abs(last_pos))) < stall_threshold) {
				//Pole stalled
				pole_tmr_stop();
				relay_main_pwr(0);
			}
		}
		last_pos = pos;
	}
}

void pole_stall_init()
{
	pole_supervisor_semaphore = xSemaphoreCreateBinary();

	if (pole_supervisor_semaphore != NULL) {
		// Create the 'handler' task, which is the task to which interrupt processing is deferred
		xTaskCreate(pole_stall_task, "PoleStall", configMINIMAL_STACK_SIZE,
				NULL, POLE_STALL_TASK_PRIORITY, NULL);
	}
}

