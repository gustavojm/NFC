#ifndef PID_H_
#define PID_H_

#include <stdint.h>

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct 	pid
 * @brief	PID instance structure.
 */
struct pid {
	double kp, ki, kd, kb;
	TickType_t sample_time_in_ticks;
	int32_t sloped_setpoint, setpoint, min_limit, max_limit, slope, sp_increments;
	int32_t last_input;
	double prop_out, integral_term, der_out, output;
	TickType_t last_time_in_ticks;
};

void pid_controller_init(struct pid *me, double kp, double ki, double kd,
		double kb, int32_t sample_time, int32_t min_limit, int32_t max_limit,
		int32_t sp_increments, int32_t input_span);

int32_t pid_controller_calculate(struct pid *me, int32_t setpoint,
		int32_t input);

#ifdef __cplusplus
}
#endif

#endif /* PID_H_ */
