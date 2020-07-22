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

struct pole_msg {
	uint32_t setpoint;
	bool	 stall_detection;
};

struct pole_tmr_id {
	bool	 direction;
	bool	 stall_detection;
};

void pole_init();

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
