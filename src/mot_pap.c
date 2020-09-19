#include "mot_pap.h"
#include "pid.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"

int32_t freq_calculate(struct pid *pid, uint32_t setpoint, uint32_t pos)
{
	int32_t cout;
	uint32_t freq;

	cout = pid_controller_calculate(pid, setpoint, pos);
	lDebug(Warn, "----COUT---- %i", cout);
	freq = abs(cout) * MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER;
	lDebug(Warn, "----FREQ---- %u", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;

	if (freq < MOT_PAP_MIN_FREQ)
		return MOT_PAP_MIN_FREQ;

	return freq;
}
