/*
 * pole.h
 *
 *  Created on: 13 jul. 2020
 *      Author: gustavo
 */

#ifndef POLE_H_
#define POLE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LEFT	0
#define RIGHT	1

enum msg_types {
	FREE_RUNNING,
	CLOSED_LOOP,
	STOP
};

struct pole_msg {
	enum  msg_types type;
	uint32_t setpoint;
	bool free_run_direction;
	uint32_t free_run_speed;
};

struct pole_tmr_id {
	bool direction;
	bool stall_detection;
};

void pole_init();

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
