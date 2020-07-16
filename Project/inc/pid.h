/*
 * pid_discreto.h
 *
 *  Created on: 13 jul. 2020
 *      Author: gustavo
 */

#ifndef PID_H_
#define PID_H_

#include <stdint.h>

typedef struct {
	float kp, ki, kd;
	float errors[3];
	float setpoint, actual, limit;
	float prop_out, int_out, der_out, out;
} pid_t;

#ifdef __cplusplus
extern "C" {
#endif

void pid_controller_init(pid_t *pid, float kp, float sample_time,
		float ti, float td, float limit);

float pid_controller_calculate(pid_t *pid, float setpoint, float actual);

#ifdef __cplusplus
}
#endif

#endif /* PID_H_ */
