#ifndef LIFT_H_
#define LIFT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lift_msg {
	bool ctrlEn;
	enum {
		LIFT_MSG_TYPE_UP, LIFT_MSG_TYPE_DOWN, LIFT_MSG_TYPE_STOP
	} type;
};

struct lift_status {
	enum {
		LIFT_STATUS_UP, LIFT_STATUS_DOWN, LIFT_STATUS_STOP
	} dir;
	bool limitUp;
	bool limitDown;
};


void lift_init();

static void lift_up();

static void lift_down();

void lift_stop();

lift_status lift_status_get(void);

#ifdef __cplusplus
}
#endif

#endif /* LIFT_H_ */
