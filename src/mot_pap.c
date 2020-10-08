#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "mot_pap.h"
#include "pid.h"
#include "ad2s1210.h"
#include "debug.h"
#include "pole_tmr.h"
#include "relay.h"
#include "dout.h"

extern bool stall_detection;

void mot_pap_init_limits(struct mot_pap *me)
{
	me->stalled = false; // If a new command was received, assume we are not stalled

	//obtener posición del RDC
	me->posAct = ad2s1210_read_position(me->rdc);
	me->cwLimitReached = false;
	me->ccwLimitReached = false;

	if (me->posAct >= me->cwLimit) {
		me->cwLimitReached = true;
	}

	if (me->posAct <= me->ccwLimit) {
		me->ccwLimitReached = true;
	}
}

void mot_pap_supervise(struct mot_pap *me)
{
	static uint16_t last_pos = 0;
	int32_t error;
	bool already_there;
	enum mot_pap_direction dir;

	me->posAct = ad2s1210_read_position(me->rdc);

	me->cwLimitReached = false;
	me->ccwLimitReached = false;

	if ((me->dir == MOT_PAP_DIRECTION_CW)
			&& (me->posAct >= (int32_t) me->cwLimit)) {
		me->cwLimitReached = true;
		pole_tmr_stop();
		lDebug(Warn, "%s: limit CW reached", me->name);
		goto cont;
	}

	if ((me->dir == MOT_PAP_DIRECTION_CCW)
			&& (me->posAct <= (int32_t) me->ccwLimit)) {
		me->ccwLimitReached = true;
		pole_tmr_stop();
		lDebug(Warn, "%s: limit CCW reached", me->name);
		goto cont;
	}

	if (stall_detection) {
		lDebug(Info, "STALL DETECTION posAct: %u, last_pos: %u", me->posAct,
				last_pos);
		if (abs((int) (me->posAct - last_pos)) < MOT_PAP_STALL_THRESHOLD) {
			me->stalled = true;
			pole_tmr_stop();
			relay_main_pwr(0);
			lDebug(Warn, "%s: stalled", me->name);
			goto cont;
		}
	}

	if (me->type == MOT_PAP_TYPE_CLOSED_LOOP) {
		error = me->posCmd - me->posAct;
		already_there = (abs((int) error) < MOT_PAP_POS_THRESHOLD);

		if (already_there) {
			me->type = MOT_PAP_TYPE_STOP;
			pole_tmr_stop();
			lDebug(Info, "%s: position reached", me->name);
		} else {
			dir = direction_calculate(error);
			if (me->dir != dir) {
				pole_tmr_stop();
				vTaskDelay(pdMS_TO_TICKS(MOT_PAP_DIRECTION_CHANGE_DELAY_MS));
				me->dir = dir;
				dout_pole_dir(me->dir);
				pole_tmr_start();
			}
			me->freq = mot_pap_freq_calculate(me->pid, me->posCmd, me->posAct);
			pole_tmr_set_freq(me->freq);
		}
	}
cont:
	last_pos = me->posAct;
}

void mot_pap_move_free_run(struct mot_pap *me,
		enum mot_pap_direction direction, uint32_t speed)
{
	bool allowed, speed_ok;

	allowed = mot_pap_movement_allowed(direction, me->cwLimitReached,
			me->ccwLimitReached);
	speed_ok = mot_pap_free_run_speed_ok(speed);

	if (allowed && speed_ok) {
		if ((me->dir != direction) && (me->type != MOT_PAP_TYPE_STOP)) {
			pole_tmr_stop();
			vTaskDelay(pdMS_TO_TICKS(MOT_PAP_DIRECTION_CHANGE_DELAY_MS));
		}
		me->type = MOT_PAP_TYPE_FREE_RUNNING;
		me->dir = direction;
		dout_pole_dir(me->dir);
		me->freq = speed * MOT_PAP_FREE_RUN_FREQ_MULTIPLIER;
		pole_tmr_set_freq(me->freq);
		pole_tmr_start();
		lDebug(Info, "%s: FREE RUN, speed: %u, direction: %s",me->name, me->freq,
				me->dir == MOT_PAP_DIRECTION_CW ? "CW" : "CCW");
	} else {
		if (!allowed)
			lDebug(Warn, "%s: movement out of bounds %s", me->name,
					direction == MOT_PAP_DIRECTION_CW ? "CW" : "CCW");
		if (!speed_ok)
			lDebug(Warn, "%s: chosen speed out of bounds %u", me->name ,speed);
	}
}

void mot_pap_move_closed_loop(struct mot_pap *me, uint16_t setpoint)
{
	int32_t error, threshold = 10;
	bool already_there;
	enum mot_pap_direction dir;

	if ((setpoint > me->cwLimit)
			| (setpoint < me->ccwLimit)) {
		lDebug(Warn, "%s: movement out of bounds", me->name);
	} else {
		me->posCmd = setpoint;
		lDebug(Info, "%s: CLOSED_LOOP posCmd: %u posAct: %u", me->name, me->posCmd,
				me->posAct);

		//calcular error de posición
		error = me->posCmd - me->posAct;
		already_there = (abs(error) < threshold);

		if (already_there) {
			pole_tmr_stop();
			lDebug(Info, "%s: already there", me->name);
		} else {
			dir = direction_calculate(error);
			if (mot_pap_movement_allowed(dir, me->cwLimitReached,
					me->ccwLimitReached)) {
				if ((me->dir != dir) && (me->type != MOT_PAP_TYPE_STOP)) {
					pole_tmr_stop();
					vTaskDelay(
							pdMS_TO_TICKS(MOT_PAP_DIRECTION_CHANGE_DELAY_MS));
				}
				me->type = MOT_PAP_TYPE_CLOSED_LOOP;
				me->dir = dir;
				dout_pole_dir(me->dir);
				me->freq = mot_pap_freq_calculate(me->pid, me->posCmd,
						me->posAct);
				pole_tmr_set_freq(me->freq);
				lDebug(Info, "%s: CLOSED LOOP, speed: %u, direction: %s", me->name,
						me->freq,
						me->dir == MOT_PAP_DIRECTION_CW ? "CW" : "CCW");
				if (!pole_tmr_started()) {
					pole_tmr_start();
				}
			} else {
				lDebug(Warn, "%s: movement out of bounds %s", me->name,
						dir == MOT_PAP_DIRECTION_CW ? "CW" : "CCW");
			}
		}
	}
}


void mot_pap_stop(struct mot_pap *me) {
	me->type = MOT_PAP_TYPE_STOP;
	pole_tmr_stop();
	lDebug(Info, "%s: STOP", me->name);
}

/**
 * @brief 	calculates the frequency to drive the stepper motors based on a PID algorithm.
 * @param 	pid		 : pointer to pid structure
 * @param 	setpoint : desired resolver value to reach
 * @param 	pos		 : current resolver value
 * @return	the calculated frequency or the limited value to MAX and MIN frequency.
 */
int32_t mot_pap_freq_calculate(struct pid *pid, uint32_t setpoint, uint32_t pos)
{
	int32_t cout;
	int32_t freq;

	cout = pid_controller_calculate(pid, (int32_t) setpoint, (int32_t) pos);
	lDebug(Info, "----COUT---- %i", cout);
	freq = (int32_t) abs((int) cout) * MOT_PAP_CLOSED_LOOP_FREQ_MULTIPLIER;
	lDebug(Info, "----FREQ---- %u", freq);
	if (freq > MOT_PAP_MAX_FREQ)
		return MOT_PAP_MAX_FREQ;

	if (freq < MOT_PAP_MIN_FREQ)
		return MOT_PAP_MIN_FREQ;

	return freq;
}
