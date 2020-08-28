#include <sys/times.h>
#include "lift.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "stdbool.h"
#include "pid.h"
#include "pole.h"

#define UP		0
#define DOWN	1
#define COMM_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )

extern QueueHandle_t lift_queue;
extern QueueHandle_t queue_pole;

static void RTUcomHMI_task(void *par)
{
	enum lift_direction dir = LIFT_DIRECTION_UP;
	struct lift_msg *lift_msg_snd;
	struct mot_pap_msg *pole_msg_snd;

	while (1) {

		//Observar que cada mensaje enviado genera una nueva alocación de memoria.
		//La tarea de destino es la que se encarga de liberar la memoria una vez procesado el mensaje

		// Generar un mensaje para lift con la dirección invertida en cada llamada
		dir = !dir;
		lift_msg_snd = (struct lift_msg*) malloc(sizeof(struct lift_msg));
		lift_msg_snd->ctrlEn = 1;
		lift_msg_snd->type = dir;
		if (xQueueSend(lift_queue, &lift_msg_snd,
				(TickType_t) 10) == pdPASS) {
			printf("comm: lift command sent \n");
		} else {
			printf("comm: unable to send lift command \n");
		}
		vTaskDelay(pdMS_TO_TICKS(1000));

		// Generar un mensaje para pole con movimiento FREERUN en sentido CW y velocidad 5
		pole_msg_snd = (struct mot_pap_msg*) malloc(
				sizeof(struct mot_pap_msg*));
		pole_msg_snd->ctrlEn = 1;
		pole_msg_snd->type = MOT_PAP_MSG_TYPE_FREE_RUNNING;
		pole_msg_snd->free_run_direction = MOT_PAP_DIRECTION_CW;
		pole_msg_snd->free_run_speed = 5;
		if (xQueueSend(queue_pole, &pole_msg_snd,
				(TickType_t) 10) == pdPASS) {
			printf("comm: pole command sent \n");
		} else {
			printf("comm: unable to send pole command \n");
		}
		vTaskDelay(pdMS_TO_TICKS(1000));

		// Generar un mensaje para pole con movimiento closed loop y set point aleatorio
		struct tms time;
		srandom(times(&time));

		pole_msg_snd = (struct mot_pap_msg*) malloc(
				sizeof(struct mot_pap_msg*));
		pole_msg_snd->ctrlEn = 1;
		pole_msg_snd->type = MOT_PAP_MSG_TYPE_CLOSED_LOOP;
		pole_msg_snd->closed_loop_setpoint = random() % ((2 ^ 16) - 1);
		if (xQueueSend(queue_pole, &pole_msg_snd,
				(TickType_t) 10) == pdPASS) {
			printf("comm: pole command sent \n");
		} else {
			printf("comm: unable to send pole command \n");
		}
		vTaskDelay(pdMS_TO_TICKS(1000));

		// Generar un mensaje de detención para pole
		pole_msg_snd = (struct mot_pap_msg*) malloc(
				sizeof(struct mot_pap_msg*));
		pole_msg_snd->ctrlEn = 1;
		pole_msg_snd->type = MOT_PAP_MSG_TYPE_STOP;
		if (xQueueSend(queue_pole, &pole_msg_snd,
				(TickType_t) 10) == pdPASS) {
			printf("comm: pole command sent \n");
		} else {
			printf("comm: unable to send pole command \n");
		}

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void comm_init()
{
	xTaskCreate(RTUcomHMI_task, "RTUcomHMI", configMINIMAL_STACK_SIZE, NULL,
	COMM_TASK_PRIORITY, NULL);
}

void task_status_get_all()
{

	//Forma de pedir el estado de cada tarea. Ver cómo poner esos valores en los mensajes TCP/IP para el HMI

//	union status {
//		struct mot_pap_status arm_st;
//		struct mot_pap_status pole_st;
//		struct lift_status lift_st;
//	} status;

//	status.arm_st = arm_status_get();
//	status.arm_st.dir;
//	status.arm_st.posCmd;
//	status.arm_st.posAct;
//	status.arm_st.vel;
//	status.arm_st.cwLimit;
//	status.arm_st.ccwLimit;
//	status.arm_st.stalled;

//	status.pole_st = pole_get_status();
//	status.pole_st.dir;
//	status.pole_st.posCmd;
//	status.pole_st.posAct;
//	status.pole_st.vel;
//	status.pole_st.cwLimit;
//	status.pole_st.ccwLimit;
//	status.pole_st.stalled;

//	status.lift_st = lift_status_get();
//	status.lift_st.dir;
//	status.lift_st.limitUp;
//	status.lift_st.limitDown;

}
