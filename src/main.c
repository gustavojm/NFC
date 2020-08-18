#include "board.h"
#include "stdint.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pole.h"
#include "lift.h"
#include "comm.h"

int main(void)
{

	pole_init();
	lift_init();
	comm_init();

	/* Start the scheduler itself. */
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/
void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
	//volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

	taskENTER_CRITICAL();
	{
		printf("[ASSERT] %s:%lu\n", pcFileName, ulLine);
        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
		//while( ulSetToNonZeroInDebuggerToContinue == 0 )
		//{
		//}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/
