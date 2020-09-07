#include "pid.h"
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"

static double abs_limit(const double out, const int32_t limit)
{
	if (out > limit)
		return limit;
	if (out < -limit)
		return -limit;
	return out;
}

void pid_controller_init(struct pid *pid, float kp, int32_t sample_time,
		float ti, float td, int32_t limit)
{
	pid->kp = kp;
	pid->ki = kp * sample_time / ti;
	pid->kd = kp * td / sample_time;
	pid->sample_time_in_ticks = pdMS_TO_TICKS(sample_time);
	pid->limit = limit;
	pid->errors[0] = 0;
	pid->errors[1] = 0;
	pid->errors[2] = 0;
	pid->setpoint = 0;
	pid->prop_out = 0;
	pid->int_out = 0;
	pid->der_out = 0;
	pid->output = 0;
}

int32_t pid_controller_calculate(struct pid *pid, int32_t setpoint,
		int32_t input)
{

	TickType_t now = xTaskGetTickCount();
	TickType_t elapsed = now - pid->last_time_in_ticks;

	if ((pid->setpoint != setpoint) | (elapsed > pid->sample_time_in_ticks)) {
		pid->setpoint = setpoint;
		pid->errors[0] = pid->setpoint - input;
		pid->prop_out = pid->kp * (pid->errors[0] - pid->errors[1]);
		pid->int_out = pid->ki * pid->errors[0];
		pid->der_out = pid->kd
				* (pid->errors[0] - 2 * pid->errors[1] + pid->errors[2]);
		pid->output += (pid->prop_out + pid->int_out + pid->der_out);

		pid->output = abs_limit(pid->output, pid->limit);

		pid->errors[2] = pid->errors[1];
		pid->errors[1] = pid->errors[0];

		pid->last_time_in_ticks = now;
	}
	return (int32_t) floor(pid->output);
}
