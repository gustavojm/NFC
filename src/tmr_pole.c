#include "board.h"
#include <stdio.h>

#define TICKRATE_HZ 10000

/**
 * @brief	Handle interrupt from 32-bit timer
 * @return	Nothing
 */
void TIMER0_IRQHandler(void)
{
	static bool On = false;

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER0, 1);
		On = (bool) !On;
		//Board_LED_Set(0, On);
	}
}

/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int pole_tmr_init(void)
{
	uint32_t timerFreq;

	/* Enable timer 0 clock and reset it */
	Chip_TIMER_Init(LPC_TIMER0);
	Chip_RGU_TriggerReset(RGU_TIMER0_RST);
	while (Chip_RGU_InReset(RGU_TIMER0_RST)) {}

	/* Get timer 0 peripheral clock rate */
	timerFreq = Chip_Clock_GetRate(CLK_MX_TIMER0);

	/* Timer setup for match and interrupt at TICKRATE_HZ */
	Chip_TIMER_Reset(LPC_TIMER0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);
	Chip_TIMER_SetMatch(LPC_TIMER0, 1, (timerFreq / TICKRATE_HZ));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);
	Chip_TIMER_Enable(LPC_TIMER0);

	/* Enable timer interrupt */
	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);

	DEBUGOUT("Timer 0 clock     = %d Hz\r\n", timerFreq);
	DEBUGOUT("Timer 0 tick rate = %d Hz\r\n", TICKRATE_HZ);
}
// Funciones a implementar
// pole_timer_start(LPC_TIMER_T *pTMR) {
//
// pole_timer_is_active
// pole_timer_change_freq
// pole_timer_stop

