#ifndef POLE_H_
#define POLE_H_

#include <stdint.h>
#include <stdbool.h>

#include "mot_pap.h"

#ifdef __cplusplus
extern "C" {
#endif

void pole_init();

struct mot_pap pole_get_status(void);

uint16_t pole_get_RDC_position();

void pole_set_cwLimit(uint16_t pos);

void pole_set_ccwLimit(uint16_t pos);

#ifdef __cplusplus
}
#endif

#endif /* POLE_H_ */
;
