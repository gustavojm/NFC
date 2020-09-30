#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "pid.h"

/**
 * @brief	constrains the out value to limit
 * @param 	out
 * @param 	limit
 * @return	out value
 * @return	limit if out > limit
 * @return	-limit if out < limit
 */
static double abs_limit(const double out, const int32_t limit)
{
	if (out > (double) limit)
		return (double) limit;
	if (out < (double) -limit)
		return (double) -limit;
	return out;
}

/**
 * @brief				: initializes the PID structure.
 * @param 	pid			: pointer to struct pid
 * @param 	kp			: proportional constant
 * @param 	sample_time	: sample time
 * @param 	ti			: integrative time
 * @param 	td			: derivative time
 * @param 	limit		: output limiter value
 */
void pid_controller_init(struct pid *pid, double kp, int32_t sample_time,
		double ti, double td, int32_t limit)
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

/**
 * @brief	calculates the incremental PID algorithm.
 * @param 	pid			: pointer to struct pid
 * @param 	setpoint	: the position to reach
 * @param 	input		: the current position
 * @return	the calculated PID output or the specified limits for output
 * @note 	if this function was called before the sample time has elapsed will return the
 * 			previously calculated value, unless a setpoint change is detected.
 */
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
