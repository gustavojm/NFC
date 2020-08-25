#ifndef MOT_PAP_H_
#define MOT_PAP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* MOT_PAP_H_ */
