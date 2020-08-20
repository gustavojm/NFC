#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "board.h"

void relay_init()
{
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 2, 4);		//DOUT0	P4_4	PIN9	GPIO2[4]  MAIN_PWR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 2, 5);		//DOUT1 P4_5	PIN10	GPIO2[5]  LIFT_PWR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 2, 6);		//DOUT2 P4_6 	PIN11	GPIO2[6]  LIFT_DIR
}

void relay_lift_dir(bool dir)
{
	if (dir) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 4);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 4);
	}
}

void relay_lift_pwr(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 5);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 5);
	}
}

void relay_main_pwr(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 2, 6);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 2, 6);
	}
}

