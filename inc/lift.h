#ifndef LIFT_H_
#define LIFT_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum lift_direction {
	LIFT_DIRECTION_UP = 0, LIFT_DIRECTION_DOWN = 1,
};

struct lift_msg {
	bool ctrlEn;
	enum {
		LIFT_MSG_TYPE_UP = 0, LIFT_MSG_TYPE_DOWN = 1, LIFT_MSG_TYPE_STOP
	} type;
};

struct lift_status {
	enum {
		LIFT_STATUS_UP, LIFT_STATUS_DOWN, LIFT_STATUS_STOP
	} dir;
	volatile bool limitUp;
	volatile bool limitDown;
};

void lift_init();

void lift_stop();

struct lift_status lift_status_get(void);

#ifdef __cplusplus
}
#endif

#endif /* LIFT_H_ */
