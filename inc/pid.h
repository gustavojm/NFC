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
	double kp, ki, kd;
	int32_t sample_time_in_ticks;
	int32_t errors[3];
	int32_t setpoint, limit;
	double prop_out, int_out, der_out, output;
	TickType_t last_time_in_ticks;
};

void pid_controller_init(struct pid *pid, float kp, int32_t sample_time,
		float ti, float td, int32_t limit);

int32_t pid_controller_calculate(struct pid *pid, int32_t setpoint, int32_t input);

#ifdef __cplusplus
}
#endif

#endif /* PID_H_ */
