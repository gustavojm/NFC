#ifndef POLE_H_
#define POLE_H_

#include <stdint.h>
#include <stdbool.h>
#include <mot_pap.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LEFT	0
#define RIGHT	1

void pole_init();

struct mot_pap_status pole_status_get(void);

void pole_set_limit_cw(bool state);

void pole_set_limit_ccw(bool state);

void pole_set_stalled(bool state);

int32_t pole_read_position(void);

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
