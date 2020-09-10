#include "mot_pap.h"
#include "pid.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"

int32_t freq_calculate(struct pid *pid, int32_t setpoint, int32_t pos)
{
	int32_t cout;
	int32_t freq;

	cout = pid_controller_calculate(pid, setpoint, pos);
	lDebug(Warn, "----COUT---- %i", cout);
	freq = abs(cout) * MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER;
	lDebug(Warn, "----FREQ---- %i", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;

	if (freq < MOT_PAP_MIN_FREQ)
		return MOT_PAP_MIN_FREQ;

	return freq;
}

int32_t shortest_signed_distance_between_circular_values(int32_t origin,
		int32_t target)
{

  int32_t signedDiff = 0.0;
  int32_t raw_diff = origin > target ? origin - target : target - origin;
  int32_t mod_diff = (raw_diff % MAX_VALUE); //equates rollover values. E.g 0 == 360 degrees in circle

  if(mod_diff > (MAX_VALUE/2) ){
    //There is a shorter path in opposite direction
    signedDiff = (MAX_VALUE - mod_diff);
    if(target>origin) signedDiff = signedDiff * -1;
  } else {
    signedDiff = mod_diff;
    if(origin>target) signedDiff = signedDiff * -1;
  }

  return signedDiff;
}
#include "mot_pap.h"
#include "pid.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"

int32_t freq_calculate(struct pid *pid, int32_t setpoint, int32_t pos)
{
	int32_t cout;
	int32_t freq;

	cout = pid_controller_calculate(pid, setpoint, pos);
	lDebug(Warn, "----COUT---- %i", cout);
	freq = abs(cout) * MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER;
	lDebug(Warn, "----FREQ---- %i", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;

	if (freq < MOT_PAP_MIN_FREQ)
		return MOT_PAP_MIN_FREQ;

	return freq;
}

int32_t shortest_signed_distance_between_circular_values(int32_t origin,
		int32_t target)
{

  int32_t signedDiff = 0.0;
  int32_t raw_diff = origin > target ? origin - target : target - origin;
  int32_t mod_diff = (raw_diff % MAX_VALUE); //equates rollover values. E.g 0 == 360 degrees in circle

  if(mod_diff > (MAX_VALUE/2) ){
    //There is a shorter path in opposite direction
    signedDiff = (MAX_VALUE - mod_diff);
    if(target>origin) signedDiff = signedDiff * -1;
  } else {
    signedDiff = mod_diff;
    if(origin>target) signedDiff = signedDiff * -1;
  }

  return signedDiff;
}
