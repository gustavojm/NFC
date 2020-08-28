#include "mot_pap.h"
#include "pid.h"
#include "stdio.h"
#include "stdlib.h"


float freq_calculate(struct pid *pid, int32_t setpoint, int32_t pos)
{
	float cout;
	float freq;

	cout = pid_controller_calculate(pid, setpoint, pos);
	printf("----COUT---- %f \n", cout);
	freq = abs(cout) * MOT_PAP_FREQ_MULTIPLIER;
	printf("----FREQ---- %f \n", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;
	return freq;
}
