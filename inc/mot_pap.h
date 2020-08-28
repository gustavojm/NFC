#ifndef MOT_PAP_H_
#define MOT_PAP_H_

#include "stdint.h"
#include "stdbool.h"
#include "pid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CWLIMIT 							5000
#define CCWLIMIT 							5
#define MOT_PAP_MAX_FREQ					150000
#define MOT_PAP_FREQ_MULTIPLIER  			( MOT_PAP_MAX_FREQ / 100 )							// Multiplier for closed loop operation
#define MOT_PAP_MAX_SPEED_FREE_RUN			8
#define MOT_PAP_FREE_RUN_FREQ_MULTIPLIER    ( MOT_PAP_MAX_FREQ / MOT_PAP_MAX_SPEED_FREE_RUN )	// Multiplier for free run operation =
#define COMPUMOTOR_MAX_FREQ					300000

enum mot_pap_direction {
	MOT_PAP_DIRECTION_CW = 0, MOT_PAP_DIRECTION_CCW = 1,
};

struct mot_pap_msg {
	bool ctrlEn;
	enum {
		MOT_PAP_MSG_TYPE_FREE_RUNNING,
		MOT_PAP_MSG_TYPE_CLOSED_LOOP,
		MOT_PAP_MSG_TYPE_STOP
	} type;
	enum mot_pap_direction free_run_direction;
	uint32_t free_run_speed;
	uint32_t closed_loop_setpoint;
};

struct mot_pap_status {
	enum {
		MOT_PAP_STATUS_CW = 0, MOT_PAP_STATUS_CCW = 1, MOT_PAP_STATUS_STOP
	} dir;
	int32_t posCmd;
	int32_t posAct;
	uint32_t vel;
	volatile bool cwLimit;
	volatile bool ccwLimit;
	volatile bool stalled;
};

inline enum mot_pap_direction direction_calculate(int32_t error)
{
	return error < 0 ? MOT_PAP_DIRECTION_CW : MOT_PAP_DIRECTION_CCW;
}

inline bool free_run_speed_ok(int32_t speed)
{
	return ((speed >= 0) && (speed <= MOT_PAP_MAX_SPEED_FREE_RUN));
}

static inline bool movement_allowed(enum mot_pap_direction dir,
bool cwLimitReached, bool ccwLimitReached)
{
	if ((dir == MOT_PAP_DIRECTION_CW && !cwLimitReached)
			|| (dir == MOT_PAP_DIRECTION_CCW && !ccwLimitReached))
		return 1;
	return 0;
}

float freq_calculate(struct pid *pid, int32_t setpoint, int32_t pos);

#ifdef __cplusplus
}
#endif

#endif /* MOT_PAP_H_ */
