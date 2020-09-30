#ifndef MOT_PAP_H_
#define MOT_PAP_H_

#include <stdint.h>
#include <stdbool.h>

#include "pid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOT_PAP_MAX_FREQ						150000
#define MOT_PAP_MIN_FREQ						100
#define MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER  	( MOT_PAP_MAX_FREQ / 100 )
#define MOT_PAP_MAX_SPEED_FREE_RUN				8
#define MOT_PAP_FREE_RUN_FREQ_MULTIPLIER    	( MOT_PAP_MAX_FREQ / MOT_PAP_MAX_SPEED_FREE_RUN )
#define MOT_PAP_COMPUMOTOR_MAX_FREQ				300000
#define MOT_PAP_DIRECTION_CHANGE_DELAY_MS		500

#define MOT_PAP_SUPERVISOR_RATE    				2	//2 means one step
#define MOT_PAP_POS_THRESHOLD 					1
#define MOT_PAP_STALL_THRESHOLD 				1


enum mot_pap_direction {
	MOT_PAP_DIRECTION_CW, MOT_PAP_DIRECTION_CCW,
};

enum mot_pap_type {
	MOT_PAP_TYPE_FREE_RUNNING,
	MOT_PAP_TYPE_CLOSED_LOOP,
	MOT_PAP_TYPE_STOP
};

/**
 * @struct 	mot_pap_msg
 * @brief	messages to POLE or ARM tasks.
 */
struct mot_pap_msg {
	bool ctrlEn;
	enum mot_pap_type type;
	enum mot_pap_direction free_run_direction;
	uint32_t free_run_speed;
	uint16_t closed_loop_setpoint;
};

/**
 * @struct 	mot_pap_status
 * @brief	POLE or ARM task status.
 */
struct mot_pap_status {
	enum mot_pap_type type;
	enum mot_pap_direction dir;
	uint16_t posCmd;
	uint16_t  posAct;
	uint32_t freq;
	uint16_t cwLimit;
	uint16_t ccwLimit;
	volatile bool cwLimitReached;
	volatile bool ccwLimitReached;
	volatile bool stalled;
};

inline enum mot_pap_direction direction_calculate(int32_t error)
{
	return error > 0 ? MOT_PAP_DIRECTION_CW : MOT_PAP_DIRECTION_CCW;
}

inline bool free_run_speed_ok(uint32_t speed)
{
	return ((speed > 0) && (speed <= MOT_PAP_MAX_SPEED_FREE_RUN));
}

static inline bool movement_allowed(enum mot_pap_direction dir,
bool cwLimitReached, bool ccwLimitReached)
{
	return ((dir == MOT_PAP_DIRECTION_CW && !cwLimitReached)
			|| (dir == MOT_PAP_DIRECTION_CCW && !ccwLimitReached));
}

int32_t freq_calculate(struct pid *pid, uint32_t setpoint, uint32_t pos);


#ifdef __cplusplus
}
#endif

#endif /* MOT_PAP_H_ */
