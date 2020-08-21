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
	bool ctrl_en;
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
	} dir;
	bool dirPole;
	int32_t	posCmdPole;
	int32_t posActPole;
	uint32_t velPole;
	bool cwLimitPole;
	bool ccwLimitPole;
};

void pole_init();

pole_status pole_status_get(void);

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
