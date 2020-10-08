#ifndef POLE_TMR_H_
#define POLE_TMR_H_

#include <stdint.h>
#include <stdbool.h>

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tmr {
	bool 		started;
	LPC_TIMER_T *lpc_timer;
	uint32_t	rgu_timer_rst;
	uint32_t	clk_mx_timer;
	uint32_t	timer_IRQn;
};


void tmr_init(struct tmr *me);

int32_t tmr_set_freq(struct tmr *me, uint32_t tick_rate_hz);

void tmr_start(struct tmr *me);

void tmr_stop(struct tmr *me);

uint32_t tmr_started(struct tmr *me);

bool tmr_match_pending(struct tmr *me);

#ifdef __cplusplus
}
#endif

#endif /* POLE_TMR_H_ */
