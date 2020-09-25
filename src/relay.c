#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "board.h"
#include "relay.h"
#include "lift.h"

/**
 * @brief 	initializes RELAYs
 * @return	nothing
 */
void relay_init()
{
	///@todo ver si falta ejecutar PinMuxSet... casi seguro
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 2, 4);		//DOUT0	P4_4	PIN9	GPIO2[4]  MAIN_PWR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 2, 5);		//DOUT1 P4_5	PIN10	GPIO2[5]  LIFT_PWR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 2, 6);		//DOUT2 P4_6 	PIN11	GPIO2[6]  LIFT_DIR
}

/**
 * @brief	sets GPIO corresponding to DOUT2 where LIFT_DIR relay is connected
 * @param 	dir		: direction of movement. Should be:
 * 					  LIFT_DIRECTION_UP
 * 					  LIFT_DIRECTION_DOWN
 * @return	nothing
 */
void relay_lift_dir(enum lift_direction dir)
{
	if (dir == LIFT_DIRECTION_UP) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 4);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 4);
	}
}

/**
 * @brief	sets GPIO corresponding to DOUT1 where LIFT_PWR relay is connected
 * @param 	state	: boolean value for the output
 * @return	nothing
 */
void relay_lift_pwr(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 5);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 5);
	}
}

/**
 * @brief	sets GPIO corresponding to DOUT1 where MAIN_PWR relay is connected
 * @param 	state	: boolean value for the output
 * @return	nothing
 */
void relay_main_pwr(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 6);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 6);
	}
}

