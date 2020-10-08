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
 * @param 	me			: pointer to struct pid
 * @param 	kp			: proportional constant
 * @param 	sample_time	: sample time
 * @param 	ti			: integrative time
 * @param 	td			: derivative time
 * @param 	limit		: output limiter value
 */
void pid_controller_init(struct pid *me, double kp, int32_t sample_time,
		double ti, double td, int32_t limit)
{
	me->kp = kp;
	me->ki = kp * sample_time / ti;
	me->kd = kp * td / sample_time;
	me->sample_time_in_ticks = pdMS_TO_TICKS(sample_time);
	me->limit = limit;
	me->errors[0] = 0;
	me->errors[1] = 0;
	me->errors[2] = 0;
	me->setpoint = 0;
	me->prop_out = 0;
	me->int_out = 0;
	me->der_out = 0;
	me->output = 0;
}

/**
 * @brief	calculates the incremental PID algorithm.
 * @param 	me			: pointer to struct pid
 * @param 	setpoint	: the position to reach
 * @param 	input		: the current position
 * @return	the calculated PID output or the specified limits for output
 * @note 	if this function was called before the sample time has elapsed will return the
 * 			previously calculated value, unless a setpoint change is detected.
 */
int32_t pid_controller_calculate(struct pid *me, int32_t setpoint,
		int32_t input)
{

	TickType_t now = xTaskGetTickCount();
	TickType_t elapsed = now - me->last_time_in_ticks;

	if ((me->setpoint != setpoint) | (elapsed > me->sample_time_in_ticks)) {
		me->setpoint = setpoint;
		me->errors[0] = me->setpoint - input;
		me->prop_out = me->kp * (me->errors[0] - me->errors[1]);
		me->int_out = me->ki * me->errors[0];
		me->der_out = me->kd
				* (me->errors[0] - 2 * me->errors[1] + me->errors[2]);
		me->output += (me->prop_out + me->int_out + me->der_out);

		me->output = abs_limit(me->output, me->limit);

		me->errors[2] = me->errors[1];
		me->errors[1] = me->errors[0];

		me->last_time_in_ticks = now;
	}
	return (int32_t) floor(me->output);
}
