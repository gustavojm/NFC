#ifndef LIFT_H_
#define LIFT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lift_msg {
	enum {
		LIFT_MSG_TYPE_UP, LIFT_MSG_TYPE_DOWN, LIFT_MSG_TYPE_STOP
	} type;
};

void lift_init();

void lift_up();

void lift_down();

void lift_stop();

#ifdef __cplusplus
}
#endif

#endif /* LIFT_H_ */
