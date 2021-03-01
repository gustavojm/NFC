#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "pid.h"
#include "debug.h"

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
void pid_controller_init(struct pid *me, double kp, double ki, double kd,
		double kb, int32_t sample_time, int32_t min_limit, int32_t max_limit,
		int32_t sp_increments, int32_t input_span)
{
	me->kp = kp * 100 / input_span;
	me->ki = ki * 100 / input_span;
	me->kd = kd * 100 / input_span;

	// Anti wind-up only if ki was set
	me->kb = 0;
	if (fabs(ki) > 0.0001) {
		me->kb = kb;
	}

	me->sample_time_in_ticks = pdMS_TO_TICKS(sample_time);
	me->min_limit = min_limit;
	me->max_limit = max_limit;
	me->sp_increments = sp_increments;
	me->setpoint = 0;
	me->sloped_setpoint = 0;
	me->integral_term = 0;
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
	double error;
	TickType_t now = xTaskGetTickCount();
	TickType_t elapsed = now - me->last_time_in_ticks;

	if (setpoint != me->setpoint) {
		// Slope calculation
		me->slope = MAX(abs((setpoint - input) / me->sp_increments), 1);
		me->setpoint = setpoint;
		me->sloped_setpoint = input; // Start increasing setpoint from current input value (bump less)
		me->integral_term = 0;
	}

	if ((elapsed > me->sample_time_in_ticks)) {
		lDebug(Info, "///// SLOPE: %i", me->slope);

		if (me->slope != 0) {
			if (setpoint > me->sloped_setpoint) {
				me->sloped_setpoint += me->slope;
				if (me->sloped_setpoint > setpoint) {
					me->sloped_setpoint = setpoint;
					me->slope = 0;
				}
			} else {
				me->sloped_setpoint -= me->slope;
				if (me->sloped_setpoint < setpoint) {
					me->sloped_setpoint = setpoint;
					me->slope = 0;
				}
			}
		}

		lDebug(Info, "///// SETPOINT: %i", me->sloped_setpoint);

		// Compute error and normalize it to 100%
		error = abs(me->sloped_setpoint - input);	// * (double) 100 / 65536;
		lDebug(Info, "///// ERROR: %f", error);

		// Compute integral term
		me->integral_term += me->ki * error;

		// Compute differential on input
		double dinput = (input - me->last_input);	// * (double) 100 / 65535;
		lDebug(Info, "///// DINPUT: %f", dinput);

		// Compute PID output
		me->output = me->kp * error + me->integral_term - me->kd * dinput;

		lDebug(Info, "///// UNLIMITED: %f", me->output);

		// Anti wind-up, if ki was set
		if (me->output > me->max_limit) {
			me->integral_term -= (me->output - me->max_limit) * me->kb;
			me->output = me->max_limit;
		} else if (me->output < me->min_limit) {
			me->integral_term += (me->min_limit - me->output) * me->kb;
			me->output = me->min_limit;
		}

		me->last_input = input;
		me->last_time_in_ticks = now;

		lDebug(Info, "///// Proporcional: %f", me->kp * error);
		lDebug(Info, "///// Integrativo: %f", me->integral_term);
		lDebug(Info, "///// Derivativo: %f", me->kd * dinput);
	}
	return (int32_t) floor(me->output);
}
