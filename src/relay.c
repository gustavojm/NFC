#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "board.h"

void relay_init()
{
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 15);	//DOUT4 P4_8  MAIN_PWR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 33);	//DOUT5 P4_9  LIFT_PWR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 35);	//DOUT6 P4_10 LIFT_DIR
}

void relay_lift_dir(bool dir)
{
	if (dir) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 35);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 35);
	}
}

void relay_lift_pwr(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 33);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 33);
	}
}

void relay_main_pwr(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 15);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 15);
	}
}

