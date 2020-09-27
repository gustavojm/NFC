#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "board.h"

#if defined(DEBUG_ENABLE) && !defined(DEBUG_UART)
#error "Definir DEBUG_UART como LPC_USART{numero de UART}"
#endif

/* System configuration variables used by chip driver */
const uint32_t ExtRateIn = 0;
const uint32_t OscRateIn = 12000000;	///< CHIP module configuration. CIAA-NXP clock is based on a 12 Mhz crystal.

/**
 * @struct io_port_t
 * @brief  GPIO port/pin pairs
 */
typedef struct {
	uint8_t port;
	uint8_t pin;
} io_port_t;

/* @formatter:off */
static const io_port_t GpioPorts[] = {
		{ 3, 0 }, 	// GPIO0	PIN74	P6_1	FUNCTION0	GPIO3[0]
		{ 5, 5 }, 	// GPIO1	PIN91	P2_5	FUNCTION4	GPIO5[5]
		{ 3, 8 }, 	// GPIO2	PIN102	P7_0	FUNCTION0	GPIO3[8]
		{ 5, 15 },	// GPIO3	PIN85	P6_7	FUNCTION4	GPIO5[15]
					// GPIO4	I2S0_RX_MCLK
					// GPIO5	I2S0_RX_SCK
					// GPIO6	I2S0_TX_SCK
		{ 3, 2 }, 	// GPIO7	PIN79	P6_3	FUNCTION0	GPIO3[2]
		{ 0, 5 } 	// GPIO8	PIN83	P6_6	FUNCTION0	GPIO0[5]
					// GPIO9	AUX_CLK
					// GPIO10	RTC_ALARM
					// GPIO11	WAKEUP0
					// GPIO12	RESET
};
/* @formatter:on */

#define GPIO_PORTS_SIZE     (sizeof(GpioPorts) / sizeof(io_port_t))

static void Board_GPIO_Init()
{
	for (uint32_t i = 0; i < GPIO_PORTS_SIZE; ++i) {
		const io_port_t *io = &GpioPorts[i];
		Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, io->port, io->pin);
	}
}

static void Board_I2C_Init()
{
	Chip_I2C_Init(I2C0);
	Chip_SCU_I2C0PinConfig(BOARD_I2C_MODE);
	Chip_I2C_SetClockRate(I2C0, BOARD_I2C_SPEED);
}

void Board_SSP_Init(LPC_SSP_T *pSSP)
{
	if (pSSP == LPC_SSP1) {
		Chip_SCU_PinMuxSet(0xF, 4, (SCU_PINIO_FAST | SCU_MODE_FUNC0)); /* PF.4 => SCK1 */

		Chip_SCU_PinMuxSet(0x1, 4,
				(SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS
						| SCU_MODE_FUNC5)); /* P1.4 => MOSI1 */
		Chip_SCU_PinMuxSet(0x1, 3,
				(SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS
						| SCU_MODE_FUNC5)); /* P1.3 => MISO1 */
	} else {
		return;
	}
}

static void Board_ADC_Init()
{
	ADC_CLOCK_SETUP_T cs;

	Chip_ADC_Init(LPC_ADC0, &cs);
	Chip_ADC_SetSampleRate(LPC_ADC0, &cs, BOARD_ADC_SAMPLE_RATE);
	Chip_ADC_SetResolution(LPC_ADC0, &cs, BOARD_ADC_RESOLUTION);
}

/**
 * @brief 	initializes debug output via UART for board
 */
void Board_Debug_Init(void)
{
#if defined(DEBUG_UART)
	Board_UART_Init(DEBUG_UART);

	Chip_UART_Init(DEBUG_UART);
	Chip_UART_SetBaudFDR(DEBUG_UART, 115200);
	Chip_UART_ConfigData(DEBUG_UART,
	UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);

	/* Enable UART Transmit */
	Chip_UART_TXEnable(DEBUG_UART);
#endif
}

/**
 * @brief sends a character on the UART
 */
void Board_UARTPutChar(char ch)
{
#if defined(DEBUG_UART)
	/* Wait for space in FIFO */
	while ((Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_THRE) == 0) {
	}
	Chip_UART_SendByte(DEBUG_UART, (uint8_t) ch);
#endif
}

/**
 * @brief	gets a character from the UART.
 * @return	EOF if no character is ready
 */
int32_t Board_UARTGetChar(void)
{
#if defined(DEBUG_UART)
	if (Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_RDR) {
		return (int) Chip_UART_ReadByte(DEBUG_UART);
	}
#endif
	return EOF;
}

void Board_UART_Init(LPC_USART_T *pUART)
{
	Chip_SCU_PinMuxSet(0x6, 4, (SCU_MODE_INACT | SCU_MODE_FUNC2)); /* P6,4 : UART0_TXD */
	Chip_SCU_PinMuxSet(0x2, 1,
			(SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS
					| SCU_MODE_FUNC1));/* P2.1 : UART0_RXD */
}

/**
 * @brief	outputs a string on the debug UART
 * @param 	str	: pointer to the string to output
 */
void Board_UARTPutSTR(const char *str)
{
#if defined(DEBUG_UART)
	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}
#endif
}

/**
 * @brief	returns the MAC address assigned to this board
 * @param	mcaddr : Pointer to 6-byte character array to populate with MAC address
 * @return	nothing
 */
void Board_ENET_GetMacADDR(uint8_t *mcaddr)
{
	uint8_t boardmac[] = { 0x00, 0x60, 0x37, 0x12, 0x34, 0x56 };

	memcpy(mcaddr, boardmac, 6);
}

/**
 * @brief 	sets up and initialize all required blocks and functions related to the
 * 			board hardware
 * @return	nothing
 */
void Board_Init(void)
{
//   DEBUGINIT();
//   Chip_GPIO_Init (LPC_GPIO_PORT);

	Board_GPIO_Init();
	//Board_ADC_Init();
	//Board_I2C_Init();
}

static ADC_CHANNEL_T curADCChannel = 0xFF;

int32_t __stdio_getchar()
{
	return Board_UARTGetChar();;
}

void __stdio_init()
{
	Board_Debug_Init();
}

