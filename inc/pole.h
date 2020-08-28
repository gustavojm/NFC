#ifndef POLE_H_
#define POLE_H_

#include <stdint.h>
#include <stdbool.h>
#include <mot_pap.h>

#ifdef __cplusplus
extern "C" {
#endif

void pole_init();

struct mot_pap_status pole_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
