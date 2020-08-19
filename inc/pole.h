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
	enum {
		POLE_MSG_TYPE_FREE_RUNNING,
		POLE_MSG_TYPE_CLOSED_LOOP,
		POLE_MSG_TYPE_STOP
	} type;
	bool free_run_direction;
	uint32_t free_run_speed;
	uint32_t closed_loop_setpoint;
};

struct pole_tmr_id {
	bool direction;
	bool stall_detection;
};

void pole_init();

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
