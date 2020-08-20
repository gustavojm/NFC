#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "board.h"

void dout_init()
{
	//Chip_SCU_PinMuxSet( 4, 8, SCU_MODE_FUNC7 );
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 12);//DOUT4 P4_8 	PIN15  	GPIO5[12]   ARM_DIR

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 13);//DOUT5 P4_9  	PIN33	GPIO5[13] 	ARM_PULSE
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 14);//DOUT6 P4_10 	PIN35	GPIO5[14] 	POLE_DIR
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 1, 8);	//DOUT7 P1_5 	PIN48 	GPIO1[8]   	POLE_PULSE
}

void dout_arm_dir(bool dir)
{
	if (dir) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 12);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 12);
	}
}

void dout_arm_pulse(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 13);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 13);
	}
}

void dout_pole_dir(bool dir)
{
	if (dir) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 14);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 14);
	}
}

void dout_pole_pulse(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 0, 8);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 0, 8);
	}
}
