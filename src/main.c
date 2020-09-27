#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "pole.h"
#include "lift.h"
#include "debug.h"

#include "RTUcomHMI.h"
#include "board.h"

#ifdef TEST_GUI
#include "gui.h"
#endif

#ifdef TEST_GUI
void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif

int main(void)
{
	debugSetLevel(Info);
#ifdef TEST_GUI
    signal(SIGSEGV, handler);   // install our handler
#endif

    pole_init();
//	arm_init();
	lift_init();

#ifdef TEST_GUI
	gui_init();
#else
	comm_init();
#endif

	/* Start the scheduler itself. */
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/
/**
 * @brief	configASSERT callback function
 * @param 	ulLine		: line where configASSERT was called
 * @param 	pcFileName	: file where configASSERT was called
 */
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
