#ifndef DIN_H_
#define DIN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void din_init();

bool din_zs1_lift_state();

bool din_zs2_lift_state();

#ifdef __cplusplus
}
#endif

#endif /* DIN_H_ */
