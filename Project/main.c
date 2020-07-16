
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tareas.h"
#include "ad2s1210.h"

static QueueHandle_t xPrintQueue;

/*-----------------------------------------------------------*/

void inicializar( void )
{
const unsigned portBASE_TYPE qsize = 20;

	/* Create the queue on which errors will be reported. */
	xPrintQueue = xQueueCreate( qsize, ( unsigned portBASE_TYPE ) sizeof( char * ) );
}
/*-----------------------------------------------------------*/

void tarea1( void * par)
{
	(void) par;	//stop compiler complaining
	for(;;){
		printf("tarea1 \n");
		vTaskDelay((TickType_t) (1000 / portTICK_PERIOD_MS));
	}
}
/*-----------------------------------------------------------*/

void tarea2 ( void * par)
{
	(void) par;	//stop compiler complaining
	for(;;){
		printf("tarea2 \n");
		vTaskDelay((TickType_t) (5000 / portTICK_PERIOD_MS));
	}
}

/* Priorities at which the tasks are created. */
#define TAREA1_PRIORITY			( configMAX_PRIORITIES - 2 )
#define TAREA2_PRIORITY			( tskIDLE_PRIORITY + 1 )


/*-----------------------------------------------------------*/


int main ( void )
{
    TaskHandle_t tarea1Handle;
    TaskHandle_t tarea2Handle;

//    struct ad2s1210_state rdc_arm;
//    int x = ad2s1210_init(&rdc_arm);
//    printf("%x", x);
//

	/* Crear las tareas. */
	xTaskCreate(tarea1, "Tarea1", configMINIMAL_STACK_SIZE, NULL, TAREA1_PRIORITY, &tarea1Handle );
	xTaskCreate(tarea2, "Tarea2", configMINIMAL_STACK_SIZE, NULL, TAREA2_PRIORITY, &tarea2Handle );

	/* Start the scheduler itself. */
	vTaskStartScheduler();

	vTaskDelete(tarea1Handle);

	return 0;
}

 /*-----------------------------------------------------------*/

 void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
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
