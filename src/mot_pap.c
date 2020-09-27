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
	uint32_t freq;

	cout = pid_controller_calculate(pid, setpoint, pos);
	lDebug(Info, "----COUT---- %i", cout);
	freq = abs(cout) * MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER;
	lDebug(Info, "----FREQ---- %u", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;

	if (freq < MOT_PAP_MIN_FREQ)
		return MOT_PAP_MIN_FREQ;

	return freq;
}

/**
 * @brief	corrects possible offsets of RDC alignment.
 * @param 	pos
 * @param 	offset
 * @return	the offset corrected position
 */
uint16_t offset_correction(uint16_t pos, uint16_t offset)
{
	int32_t corrected = pos - offset;
	if (corrected < 0)
		corrected = corrected + (int32_t) pow(2, 16);
	return (uint16_t) corrected;
}
