#include "pid.h"
#include <stddef.h>

int32_t abs_limit(const int32_t out, const int32_t limit)
{
	if (out > limit)
		return limit;
	if (out < -limit)
		return -limit;
	return out;
}

void pid_controller_init(struct pid *pid, int32_t kp, int32_t sample_time, int32_t ti,
		int32_t td, int32_t limit, int32_t rate)
{
	pid->kp = kp;
	pid->ki = kp * sample_time / ti;
	pid->kd = kp * td / sample_time;
	pid->limit = limit;
	pid->rate = rate;
	pid->errors[0] = 0.0f;
	pid->errors[1] = 0.0f;
	pid->errors[2] = 0.0f;
	pid->input = 0.0f;
	pid->setpoint = 0.0f;
	pid->prop_out = 0.0f;
	pid->int_out = 0.0f;
	pid->der_out = 0.0f;
	pid->output = 0.0f;
}

static int32_t rate_limit(struct pid *pid, int32_t setpoint)
{
	// Implementar rate limiter;
	return setpoint;
}

int32_t pid_controller_calculate(struct pid *pid, int32_t setpoint, int32_t input)
{
	if (pid->setpoint != setpoint) {
		pid->setpoint = rate_limit(pid, setpoint);
	}

	pid->input = input;
	pid->errors[0] = pid->setpoint - pid->input;
	pid->prop_out = pid->kp * (pid->errors[0] - pid->errors[1]);
	pid->int_out = pid->ki * pid->errors[0];
	pid->der_out = pid->kd
			* (pid->errors[0] - 2 * pid->errors[1] + pid->errors[2]);
	pid->output += (pid->prop_out + pid->int_out + pid->der_out);

	pid->output = abs_limit(pid->output, pid->limit);

	pid->errors[2] = pid->errors[1];
	pid->errors[1] = pid->errors[0];

	return pid->output;
}
