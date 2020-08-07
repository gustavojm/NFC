/*
 * pid.h
 *
 *  Created on: 13 jul. 2020
 *      Author: gustavo
 */

#ifndef PID_H_
#define PID_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


struct pid {
	float kp, ki, kd;
	float errors[3];
	float setpoint, actual, limit, rate;
	float prop_out, int_out, der_out, out;
};

void pid_controller_init(struct pid *pid, float kp, float sample_time,
		float ti, float td, float limit, float rate);

float pid_controller_calculate(struct pid *pid, float setpoint, float actual);

#ifdef __cplusplus
}
#endif

#endif /* PID_H_ */
