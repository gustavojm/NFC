#ifndef POLE_H_
#define POLE_H_

#include <stdint.h>
#include <stdbool.h>
#include <mot_pap.h>

#ifdef __cplusplus
extern "C" {
#endif

void pole_init();

enum mot_pap_direction pole_get_direction(void);

struct mot_pap_status pole_get_status(void);

void pole_set_cw_limit_reached(bool state);

void pole_set_ccw_limit_reached(bool state);

void pole_set_stalled(bool state);

int32_t pole_read_position(void);

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
