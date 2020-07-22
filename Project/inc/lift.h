/*
 * lift.h
 *
 *  Created on: 13 jul. 2020
 *      Author: gustavo
 */

#ifndef LIFT_H_
#define LIFT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lift_msg {
	uint32_t direction;
};

void lift_init();

#ifdef __cplusplus
}
#endif

#endif /* LIFT_H_ */
