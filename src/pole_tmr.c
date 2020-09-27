#include <stdio.h>
#include <errno.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "board.h"
#include "core_cm4_v5.h"
#include "dout.h"
#include "mot_pap.h"
#include "debug.h"
#include "pole.h"

extern bool stall_detection;
extern SemaphoreHandle_t pole_supervisor_semaphore;

/**
 * @brief	handle interrupt from 32-bit timer to generate pulses for the stepper motor drivers
 * @return	nothing
 * @note 	calls the supervisor task every x number of generated steps
 */
void TIMER0_IRQHandler(void)
{
	uint32_t static steps = 0;
	static bool On = false;
	BaseType_t xHigherPriorityTaskWoken;

	static enum mot_pap_direction last_dir = MOT_PAP_DIRECTION_CW;

	if (Chip_TIMER_MatchPending(LPC_TIMER0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER0, 1);

		if (pole_get_status().dir != last_dir) {
			steps = 0;
			last_dir = pole_get_status().dir;
		}

		On = (bool) !On;
		// Generate waveform
		dout_pole_pulse(On);


		if (++steps == MOT_PAP_SUPERVISOR_RATE) {
			steps = 0;
			xHigherPriorityTaskWoken = pdFALSE;
			xSemaphoreGiveFromISR(pole_supervisor_semaphore,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

/**
 * @brief	enables timer 0 clock and resets it
 * @return	nothing
 */
void pole_tmr_init(void)
{
	Chip_TIMER_Init(LPC_TIMER0);
	Chip_RGU_TriggerReset(RGU_TIMER0_RST);

	while (Chip_RGU_InReset(RGU_TIMER0_RST)) {
	}

	Chip_TIMER_Reset(LPC_TIMER0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER0, 1);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 1);
}


/**
 * @brief	sets TIMER0 frequency
 * @param 	tick_rate_hz 	: desired frequency
 * @return	0 on success
 * @return	-EINVAL if tick_rate_hz > 300000
 */
int32_t pole_tmr_set_freq(int32_t tick_rate_hz)
{
	uint32_t timerFreq;

	if ((tick_rate_hz < 0) || (tick_rate_hz > MOT_PAP_COMPUMOTOR_MAX_FREQ)) {
		lDebug(Error, "pole: invalid freq");
		return -EINVAL;
	}

	/* Get timer 0 peripheral clock rate */
	timerFreq = Chip_Clock_GetRate(CLK_MX_TIMER0);

	/* Timer setup for match at tick_rate_hz */
	Chip_TIMER_SetMatch(LPC_TIMER0, 1, (timerFreq / tick_rate_hz));
	return 0;
}


/**
 * @brief 	enables timer interrupt and starts it
 * @return 	nothing
 */
void pole_tmr_start(void)
{
	Chip_TIMER_Enable(LPC_TIMER0);
	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
}

/**
 * @brief 	disables timer interrupt and stops it
 * @return 	nothing
 */
void pole_tmr_stop(void)
{
	Chip_TIMER_Disable(LPC_TIMER0);
	NVIC_DisableIRQ(TIMER0_IRQn);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
}

/**
 * @brief	returns if timer is started by querying if the interrupt is enabled
 * @return  0 timer is not started.
 * @return  1 timer is started.
 */
uint32_t pole_tmr_started(void)
{
	return NVIC_GetEnableIRQ(TIMER0_IRQn);
}

