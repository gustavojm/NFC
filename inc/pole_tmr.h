#ifndef POLE_TMR_H_
#define POLE_TMR_H_

#include <stdint.h>
#include <board.h>

#ifdef __cplusplus
extern "C" {
#endif

void pole_tmr_init(void);

int32_t pole_tmr_set_freq(int32_t tick_rate_hz);

void pole_tmr_start(void);

void pole_tmr_stop(void);

uint32_t pole_tmr_started(void);

#ifdef __cplusplus
}
#endif

#endif /* POLE_TMR_H_ */
