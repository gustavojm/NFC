#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "mot_pap.h"
#include "pid.h"
#include "debug.h"

/**
 * @brief 	calculates the frequency to drive the stepper motors based on a PID algorithm.
 * @param 	pid		 : pointer to pid structure
 * @param 	setpoint : desired resolver value to reach
 * @param 	pos		 : current resolver value
 * @return	the calculated frequency or the limited value to MAX and MIN frequency.
 */
int32_t freq_calculate(struct pid *pid, uint32_t setpoint, uint32_t pos)
{
	int32_t cout;
	int32_t freq;

	cout = pid_controller_calculate(pid, (int32_t) setpoint, (int32_t) pos);
	lDebug(Info, "----COUT---- %i", cout);
	freq = (int32_t) abs((int) cout) * MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER;
	lDebug(Info, "----FREQ---- %u", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;

	if (freq < MOT_PAP_MIN_FREQ)
		return MOT_PAP_MIN_FREQ;

	return freq;
}
