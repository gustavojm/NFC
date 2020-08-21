#ifndef POLE_H_
#define POLE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LEFT	0
#define RIGHT	1

struct pole_msg {
	bool ctrlEn;
	enum {
		POLE_MSG_TYPE_FREE_RUNNING,
		POLE_MSG_TYPE_CLOSED_LOOP,
		POLE_MSG_TYPE_STOP
	} type;
	bool free_run_direction;
	uint32_t free_run_speed;
	uint32_t closed_loop_setpoint;
};

struct pole_status {
	enum {
		POLE_STATUS_UP, POLE_STATUS_DOWN, POLE_STATUS_STOP
	} dirPole;
	int32_t	posCmdPole;
	int32_t posActPole;
	uint32_t velPole;
	volatile bool cwLimitPole;
	volatile bool ccwLimitPole;
	volatile bool stalled;
};

void pole_init();

pole_status pole_status_get(void);

void pole_set_limit_cw(bool state);

void pole_set_limit_ccw(bool state);

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
