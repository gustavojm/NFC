#include <stdlib.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "debug.h"
#include "dout.h"
#include "dout.h"

static void blink_task(void *par)
{
	while (true) {
		dout_arm_pulse();
		lDebug(Info, "BBBBBBBBBBBBBBB");
		vTaskDelay(2000);
	}
}

/**
 * @brief 	creates the blinking task
 * @return	nothing
 */
void blink_init(void)
{
	dout_init();
	xTaskCreate(blink_task, "Blink", 128, NULL,
	3, NULL);
	lDebug(Info, "blink: task created");

}
