#ifndef MOT_PAP_H_
#define MOT_PAP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mot_pap_msg {
	bool ctrlEn;
	enum {
		MOT_PAP_MSG_TYPE_FREE_RUNNING,
		MOT_PAP_MSG_TYPE_CLOSED_LOOP,
		MOT_PAP_MSG_TYPE_STOP
	} type;
	bool free_run_direction;
	uint32_t free_run_speed;
	uint32_t closed_loop_setpoint;
};

struct mot_pap_status {
	enum {
		MOT_PAP_STATUS_UP, MOT_PAP_STATUS_DOWN, MOT_PAP_STATUS_STOP
	} dir;
	int32_t	posCmd;
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
