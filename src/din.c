#include "stdio.h"
#include "stdbool.h"
#include "board.h"

//DIN0 P4_0 	PIN1  	GPIO2[0]	ZS1_LIFT
#define ZS1_LIFT_GPIO_PORT	2
#define ZS1_LIFT_GPIO_BIT	0
#define ZS1_LIFT_PIN_PORT	4
#define ZS1_LIFT_PIN_NUM	0
#define ZS1_LIFT_INT_CH		0

//DIN1 P4_1 	PIN1  	GPIO2[1]	ZS2_LIFT
#define ZS2_LIFT_GPIO_PORT	2
#define ZS2_LIFT_GPIO_BIT	1
#define ZS2_LIFT_PIN_PORT	4
#define ZS2_LIFT_PIN_NUM	1
#define ZS2_LIFT_INT_CH		1

void din_init()
{
	// Configure ZS1_LIFT to generate interrupts on falling edge
	/* Set pin back to GPIO (on some boards may have been changed to something else by Board_Init()) */
	Chip_SCU_PinMuxSet(ZS1_LIFT_PIN_PORT, ZS1_LIFT_PIN_NUM,
			(SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC0));

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, ZS1_LIFT_GPIO_PORT,
	ZS1_LIFT_GPIO_BIT);

	/* Configure interrupt channel 0 for the GPIO pin in SysCon block */
	Chip_SCU_GPIOIntPinSel(ZS1_LIFT_INT_CH, ZS1_LIFT_GPIO_PORT,
	ZS1_LIFT_GPIO_BIT);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(ZS1_LIFT_INT_CH));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(ZS1_LIFT_INT_CH));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(ZS1_LIFT_INT_CH));

	/* Enable interrupt in the NVIC */
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT0_IRQn);

	// Configure ZS1_LIFT to generate interrupts on falling edge
	/* Set pin back to GPIO (on some boards may have been changed to something else by Board_Init()) */
	Chip_SCU_PinMuxSet(ZS2_LIFT_PIN_PORT, ZS2_LIFT_PIN_NUM,
			(SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC0));

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, ZS2_LIFT_GPIO_PORT,
	ZS2_LIFT_GPIO_BIT);

	/* Configure interrupt channel 0 for the GPIO pin in SysCon block */
	Chip_SCU_GPIOIntPinSel(ZS1_LIFT_INT_CH, ZS2_LIFT_GPIO_PORT,
	ZS2_LIFT_GPIO_BIT);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(ZS2_LIFT_INT_CH));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(ZS2_LIFT_INT_CH));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(ZS2_LIFT_INT_CH));

	/* Enable interrupt in the NVIC */
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);

}

bool din_zs1_lift_state()
{
	return Chip_GPIO_GetPinState(LPC_GPIO_PORT, ZS1_LIFT_GPIO_PORT,
			ZS1_LIFT_GPIO_BIT);
}

bool din_zs2_lift_state()
{
	return Chip_GPIO_GetPinState(LPC_GPIO_PORT, ZS2_LIFT_GPIO_PORT,
			ZS2_LIFT_GPIO_BIT);
}
