#ifndef DOUT_H_
#define DOUT_H_

#include <stdint.h>
#include "mot_pap.h"

#ifdef __cplusplus
extern "C" {
#endif

void dout_init();

void dout_arm_dir(bool dir);

void dout_arm_pulse(bool state);

void dout_pole_dir(enum mot_pap_direction dir);

void dout_pole_pulse(bool state);

#ifdef __cplusplus
}
#endif

#endif /* DOUT_H_ */
