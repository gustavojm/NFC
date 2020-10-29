#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "pole.h"
#include "lift.h"
#include "debug.h"
#include "board.h"

#include "RTUcomHMI.h"
#include "board.h"

#ifdef TEST_GUI
#include <execinfo.h>
#include <unistd.h>
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

#if defined(__FPU_PRESENT) && __FPU_PRESENT == 1
	fpuInit();
#endif

	SystemCoreClockUpdate();
	Board_SystemInit();
	Board_Init();


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

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
    volatile signed char *name;
    volatile xTaskHandle *pxT;

    name = pcTaskName;
    pxT  = pxTask;

    (void)name;
    (void)pxT;

    while(1);
}
#endif


/*-----------------------------------------------------------*/
/**
 * @brief	configASSERT callback function
 * @param 	ulLine		: line where configASSERT was called
 * @param 	pcFileName	: file where configASSERT was called
 */
void vAssertCalled(unsigned long ulLine, const char *const pcFileName)
{
	volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

	taskENTER_CRITICAL();
	{
		printf("[ASSERT] %s:%lu\n", pcFileName, ulLine);
		/* You can step out of this function to debug the assertion by using
		 the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
		 value. */
		while( ulSetToNonZeroInDebuggerToContinue == 0 )
		{
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    (void) r0;
    (void) r1;
    (void) r2;
    (void) r3;
    (void) r12;
    (void) lr;
    (void) pc;
    (void) psr;

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

