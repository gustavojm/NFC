#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "board.h"
#include "dout.h"
#include "mot_pap.h"
#include "debug.h"
#include "pole.h"

#define TMR_INTERRUPT_PRIORITY 		( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 )

/**
 * @brief	enables timer 0 clock and resets it
 * @return	nothing
 */
void tmr_init(struct tmr *me)
{
	Chip_TIMER_Init(me->lpc_timer);
	Chip_RGU_TriggerReset(me->rgu_timer_rst);

	while (Chip_RGU_InReset(me->rgu_timer_rst)) {
	}

	Chip_TIMER_Reset(me->lpc_timer);
	Chip_TIMER_MatchEnableInt(me->lpc_timer, 1);
	Chip_TIMER_ResetOnMatchEnable(me->lpc_timer, 1);
}

/**
 * @brief	sets TIMER0 frequency
 * @param 	me				: pointer to tmr structure
 * @param 	tick_rate_hz 	: desired frequency
 * @return	0 on success
 * @return	-1 if tick_rate_hz > 300000
 */
int32_t tmr_set_freq(struct tmr *me, uint32_t tick_rate_hz)
{
	uint32_t timerFreq;

	if ((tick_rate_hz < 0) || (tick_rate_hz > MOT_PAP_COMPUMOTOR_MAX_FREQ)) {
		lDebug(Error, "pole: invalid freq");
		return -1;
	}

	/* Get timer 0 peripheral clock rate */
	timerFreq = Chip_Clock_GetRate(me->clk_mx_timer);

	/* Timer setup for match at tick_rate_hz */
	Chip_TIMER_SetMatch(me->lpc_timer, 1, (timerFreq / tick_rate_hz));
	return 0;
}

/**
 * @brief 	enables timer interrupt and starts it
 * @param 	me				: pointer to tmr structure
 * @return 	nothing
 */
void tmr_start(struct tmr *me)
{
	Chip_TIMER_Enable(LPC_TIMER0);
	NVIC_SetPriority(me->timer_IRQn, TMR_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(me->timer_IRQn);
	NVIC_ClearPendingIRQ(me->timer_IRQn);
	me->started = true;
}

/**
 * @brief 	disables timer interrupt and stops it
 * @param 	me				: pointer to tmr structure
 * @return 	nothing
 */
void tmr_stop(struct tmr *me)
{
	Chip_TIMER_Disable(me->lpc_timer);
	NVIC_DisableIRQ(me->timer_IRQn);
	NVIC_ClearPendingIRQ(me->timer_IRQn);
	me->started = false;
}

/**
 * @brief	returns if timer is started by querying if the interrupt is enabled
 * @param 	me				: pointer to tmr structure
 * @return  0 timer is not started.
 * @return  1 timer is started.
 */
uint32_t tmr_started(struct tmr *me)
{
	return me->started;
}

/**
 * @brief	Determine if a match interrupt is pending
 * @param 	me				: pointer to tmr structure
 * @return  0 timer is not started.
 * @return  false if the interrupt is not pending, otherwise true
 * @note	Determine if the match interrupt for the passed timer and match
 * 			counter is pending. If the interrupt is pending clears the match counter
 */
bool tmr_match_pending(struct tmr *me)
{
	bool ret = Chip_TIMER_MatchPending(LPC_TIMER0, 1);
	if (ret) {
		Chip_TIMER_ClearMatch(LPC_TIMER0, 1);
	}
	return ret;
}
