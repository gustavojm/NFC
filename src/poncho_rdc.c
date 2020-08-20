#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "board.h"

void poncho_rdc_init()
{
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 3, 0);		//GPIO0	P6_1	PIN74	GPIO3[0]  RESET (BOTH)
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 5);		//GPIO1 P2_5 	PIN91	GPIO5[5]  SAMPLE (BOTH)
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 3, 8);		//GPIO2 P7_0 	PIN110	GPIO3[8]  WR/FSYNC (ARM)
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 5, 15);	//GPIO3 P6_7	PIN85	GPIO5[15] WR/FSYNC (POLE)
}

void poncho_rdc_reset(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 0);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 0);
	}
}

void poncho_rdc_sample(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 5);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 5);
	}
}


void poncho_rdc_arm_wr_fsync(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 3, 8);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 3, 8);
	}
}

void poncho_rdc_pole_wr_fsync(bool state)
{
	if (state) {
		Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 5, 15);
	} else {
		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 5, 15);
	}
}
