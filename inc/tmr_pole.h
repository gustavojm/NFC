#ifndef TMR_POLE_H_
#define TMR_POLE_H_

#ifdef __cplusplus
extern "C" {
#endif

void pole_tmr_init(void);

void pole_tmr_set_freq(int32_t tick_rate_hz);

void pole_tmr_start(void);

void pole_tmr_stop(void);

uint32_t pole_tmr_started(void);

#ifdef __cplusplus
}
#endif

#endif /* TMR_POLE_H_ */
