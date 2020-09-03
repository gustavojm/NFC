#ifndef PID_H_
#define PID_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pid {
	int32_t kp, ki, kd;
	int32_t errors[3];
	int32_t setpoint, input, limit, rate;
	int32_t prop_out, int_out, der_out, output;
};

void pid_controller_init(struct pid *pid, int32_t kp, int32_t sample_time,
		int32_t ti, int32_t td, int32_t limit, int32_t rate);

int32_t pid_controller_calculate(struct pid *pid, int32_t setpoint, int32_t input);

#ifdef __cplusplus
}
#endif

#endif /* PID_H_ */
