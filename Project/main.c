/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pole.h"
#include "lift.h"

int main(void)
{

	pole_init();
	lift_init();

	/* Start the scheduler itself. */
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/
void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
	taskENTER_CRITICAL();
	{
		printf("[ASSERT] %s:%lu\n", pcFileName, ulLine);
		fflush(stdout);
	}
	taskEXIT_CRITICAL();
	exit(-1);
}
/*-----------------------------------------------------------*/
