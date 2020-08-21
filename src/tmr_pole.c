#include "board.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include "core_cm4_v5.h"
#include "dout.h"

extern bool stall_detection;
extern SemaphoreHandle_t pole_supervisor_semaphore;

/**
 * @brief	Handle interrupt from 32-bit timer
 * @return	Nothing
 */

void TIMER0_IRQHandler(void)
{
	static bool On = false;
	BaseType_t xHigherPriorityTaskWoken;

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER0, 1);
		On = (bool) !On;
		// Generate waveform
		dout_pole_pulse(On);

		if (stall_detection) {
			xHigherPriorityTaskWoken = pdFALSE;
			xSemaphoreGiveFromISR(pole_supervisor_semaphore,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

void pole_tmr_init(void)
{
	/* Enable timer 0 clock and reset it */
	Chip_TIMER_Init(LPC_TIMER0);
	Chip_RGU_TriggerReset(RGU_TIMER0_RST);

	while (Chip_RGU_InReset(RGU_TIMER0_RST)) {
	}

	Chip_TIMER_Reset(LPC_TIMER0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);
}

void pole_tmr_set_freq(int32_t tick_rate_hz)
{
	uint32_t timerFreq;
	/* Get timer 0 peripheral clock rate */
	timerFreq = Chip_Clock_GetRate(CLK_MX_TIMER0);

	/* Timer setup for match at tick_rate_hz */
	Chip_TIMER_SetMatch(LPC_TIMER0, 1, (timerFreq / tick_rate_hz));
}

void pole_tmr_start(void)
{
	/* Enable timer interrupt */
	Chip_TIMER_Enable(LPC_TIMER0);
	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
}

void pole_tmr_stop(void)
{
	/* Disable timer interrupt */
	NVIC_DisableIRQ(TIMER0_IRQn);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
}

uint32_t pole_tmr_started(void)
{
	return NVIC_GetEnableIRQ(TIMER0_IRQn);
}

