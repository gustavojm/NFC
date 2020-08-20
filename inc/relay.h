#ifndef RELAY_H_
#define RELAY_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void relay_init();

void relay_lift_dir(bool dir);

void relay_lift_pwr(bool state);

void relay_main_pwr(bool state);

#ifdef __cplusplus
}
#endif

#endif /* RELAY_H_ */
