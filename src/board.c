/*
 */

#include "board.h"
#include "string.h"


#if defined(DEBUG_ENABLE) && !defined(DEBUG_UART)
#error "Definir DEBUG_UART como LPC_USART{numero de UART}"
#endif


/* System configuration variables used by chip driver */
// CHIP module configuration. CIAA-NXP clock is based on a 12 Mhz crystal.
const uint32_t ExtRateIn = 0;
const uint32_t OscRateIn = 12000000;

typedef struct {
	uint8_t port;
	uint8_t pin;
} io_port_t;

static const io_port_t gpioLEDBits[] = {{3, 5}, {0, 14}, {3, 7}};

//Verificar los puertos y pines de gpio ya que estos pertenecen a la EDU
static const struct io_port_t GpioPorts[] = {
   {3, 0}, {3, 3}, {3, 4}, {5,15}, {5,16}, {3, 5}, {3, 6}, {3, 7}, {2, 8}
};

#define GPIO_PORTS_SIZE     (sizeof(GpioPorts) / sizeof(struct io_port_t))

static void Board_GPIO_Init()
{
   for (uint32_t i = 0; i < GPIO_PORTS_SIZE; ++i) {
      const struct gpio_t *io = &GpioPorts[i];
      Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, io->port, io->pin);
   }
}


static void Board_I2C_Init()
{
   Chip_I2C_Init(I2C0);
   Chip_SCU_I2C0PinConfig(BOARD_I2C_MODE);
   Chip_I2C_SetClockRate(I2C0, BOARD_I2C_SPEED);
}


static void Board_SPI_Init()
{
   Chip_SSP_Init(LPC_SSP1);
   Chip_SSP_Set_Mode(LPC_SSP1, BOARD_SPI_MODE);
   Chip_SSP_SetFormat(LPC_SSP1, BOARD_SPI_BITS, BOARD_SPI_FORMAT,
                      BOARD_SPI_POLARITY);
   Chip_SSP_SetBitRate(LPC_SSP1, BOARD_SPI_SPEED);
   Chip_SSP_Enable(LPC_SSP1);
}


static void Board_ADC_Init()
{
   ADC_CLOCK_SETUP_T cs;

   Chip_ADC_Init(LPC_ADC0, &cs);
   Chip_ADC_SetSampleRate(LPC_ADC0, &cs, BOARD_ADC_SAMPLE_RATE);
   Chip_ADC_SetResolution(LPC_ADC0, &cs, BOARD_ADC_RESOLUTION);
}


void Board_Debug_Init(void)
{
   Chip_UART_Init(DEBUG_UART);
   Chip_UART_SetBaudFDR(DEBUG_UART, DEBUG_UART_BAUD_RATE);
   Chip_UART_ConfigData(DEBUG_UART, DEBUG_UART_CONFIG);
   Chip_UART_TXEnable(DEBUG_UART);
}

void Board_UARTPutChar(char ch)
{
   while ( !(Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_THRE));
   Chip_UART_SendByte(DEBUG_UART, (uint8_t) ch);
}


int Board_UARTGetChar(void)
{
   if (Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_RDR) {
      return (int) Chip_UART_ReadByte(DEBUG_UART);
   }
   return EOF;
}

void Board_UARTPutSTR(const char *str)
{
   while (*str != '\0') {
      Board_UARTPutChar(*str++);
   }
}


static uint32_t lcd_cfg_val;

void Board_UART_Init(LPC_USART_T *pUART)
{
	Chip_SCU_PinMuxSet(0x6, 4, (SCU_MODE_INACT | SCU_MODE_FUNC2));					/* P6,4 : UART0_TXD */
	Chip_SCU_PinMuxSet(0x2, 1, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC1));/* P2.1 : UART0_RXD */
}

/* Initialize debug output via UART for board */
void Board_Debug_Init(void)
{
#if defined(DEBUG_UART)
	Board_UART_Init(DEBUG_UART);

	Chip_UART_Init(DEBUG_UART);
	Chip_UART_SetBaudFDR(DEBUG_UART, 115200);
	Chip_UART_ConfigData(DEBUG_UART, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);

	/* Enable UART Transmit */
	Chip_UART_TXEnable(DEBUG_UART);
#endif
}

/* Sends a character on the UART */
void Board_UARTPutChar(char ch)
{
#if defined(DEBUG_UART)
	/* Wait for space in FIFO */
	while ((Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_THRE) == 0) {}
	Chip_UART_SendByte(DEBUG_UART, (uint8_t) ch);
#endif
}

/* Gets a character from the UART, returns EOF if no character is ready */
int Board_UARTGetChar(void)
{
#if defined(DEBUG_UART)
	if (Chip_UART_ReadLineStatus(DEBUG_UART) & UART_LSR_RDR) {
		return (int) Chip_UART_ReadByte(DEBUG_UART);
	}
#endif
	return EOF;
}

/* Outputs a string on the debug UART */
void Board_UARTPutSTR(const char *str)
{
#if defined(DEBUG_UART)
	while (*str != '\0') {
		Board_UARTPutChar(*str++);
	}
#endif
}

/* Returns the MAC address assigned to this board */
void Board_ENET_GetMacADDR(uint8_t *mcaddr)
{
	uint8_t boardmac[] = {0x00, 0x60, 0x37, 0x12, 0x34, 0x56};

	memcpy(mcaddr, boardmac, 6);
}

/* Set up and initialize all required blocks and functions related to the
   board hardware */
void Board_Init(void)
{
   DEBUGINIT();

   Chip_GPIO_Init (LPC_GPIO_PORT);

   Board_GPIO_Init();
   //Board_ADC_Init();
   Board_SPI_Init();
   //Board_I2C_Init();
}


void Board_I2C_Init(I2C_ID_T id)
{
	if (id == I2C1) {
		/* Configure pin function for I2C1*/
		Chip_SCU_PinMuxSet(0x2, 3, (SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC1));		/* P2.3 : I2C1_SDA */
		Chip_SCU_PinMuxSet(0x2, 4, (SCU_MODE_ZIF_DIS | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC1));		/* P2.4 : I2C1_SCL */
	}
	else {
		Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
	}
}

static ADC_CHANNEL_T curADCChannel = 0xFF;

void __stdio_putchar(int c)
{
   Board_UARTPutChar(c);
}

int __stdio_getchar()
{
   return Board_UARTGetChar();;
}

void __stdio_init()
{
   Board_Debug_Init();
}

/**
 * @}
 */
