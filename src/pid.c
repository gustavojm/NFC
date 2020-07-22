/*
 * pid.c
 *
 *  Created on: 13 jul. 2020
 *      Author: gustavo
 */

#include "pid.h"
#include <stddef.h>

float abs_limit(const float out, const float limit)
{
	if (out > limit)
		return limit;
	if (out < -limit)
		return -limit;
	return out;
}

void pid_controller_init(struct pid *pid, float kp, float sample_time, float ti,
		float td, float limit)
{
	pid->kp = kp;
	pid->ki = kp * sample_time / ti;
	pid->kd = kp * td / sample_time;
	pid->limit = limit;
	pid->errors[0] = 0.0f;
	pid->errors[1] = 0.0f;
	pid->errors[2] = 0.0f;
	pid->actual = 0.0f;
	pid->setpoint = 0.0f;
	pid->prop_out = 0.0f;
	pid->int_out = 0.0f;
	pid->der_out = 0.0f;
	pid->out = 0.0f;
}

float pid_controller_calculate(struct pid *pid, float setpoint, float actual)
{
	pid->actual = actual;
	pid->setpoint = setpoint;
	pid->errors[0] = pid->setpoint - pid->actual;
	pid->prop_out = pid->kp * (pid->errors[0] - pid->errors[1]);
	pid->int_out = pid->ki * pid->errors[0];
	pid->der_out = pid->kd
			* (pid->errors[0] - 2 * pid->errors[1] + pid->errors[2]);
	pid->out += (pid->prop_out + pid->int_out + pid->der_out);

	pid->out = abs_limit(pid->out, pid->limit);

	pid->errors[2] = pid->errors[1];
	pid->errors[1] = pid->errors[0];

	return pid->out;
}
