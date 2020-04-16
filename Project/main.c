
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tareas.h"

static QueueHandle_t xPrintQueue;

/*-----------------------------------------------------------*/

void vInicializar( void )
{
const unsigned portBASE_TYPE uxQueueSize = 20;

	/* Create the queue on which errors will be reported. */
	xPrintQueue = xQueueCreate( uxQueueSize, ( unsigned portBASE_TYPE ) sizeof( char * ) );
}
/*-----------------------------------------------------------*/

void vTarea1( void * par)
{
	(void) par;	//stop compiler complaining
	for(;;){
		printf("tarea1 \n");
		vTaskDelay((TickType_t) (1000 / portTICK_PERIOD_MS));
	}
}
/*-----------------------------------------------------------*/

void vTarea2 ( void * par)
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
    TaskHandle_t xTarea1Task;
    TaskHandle_t xTarea2Task;

	/* Crear las tareas. */
	xTaskCreate(vTarea1, "Tarea1", configMINIMAL_STACK_SIZE, NULL, TAREA1_PRIORITY, &xTarea1Task );
	xTaskCreate(vTarea2, "Tarea2", configMINIMAL_STACK_SIZE, NULL, TAREA2_PRIORITY, &xTarea2Task );

	/* Start the scheduler itself. */
	vTaskStartScheduler();

	vTaskDelete(xTarea1Task);

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
