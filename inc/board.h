#ifndef BOARD_H_
#define BOARD_H_

#include <chip.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_CIAA_NXP_4337

#define BOARD_GPIO_0                0
#define BOARD_GPIO_1                1
#define BOARD_GPIO_2                2
#define BOARD_GPIO_3                3
#define BOARD_GPIO_4                4
#define BOARD_GPIO_5                5
#define BOARD_GPIO_6                6
#define BOARD_GPIO_7                7
#define BOARD_GPIO_8                8

#define BOARD_I2C_PORT              I2C0
#define BOARD_SPI_PORT              LPC_SSP1

#define DEBUG_UART                  LPC_USART2
#define DEBUG_UART_IRQ              USART2_IRQn
#define DEBUG_UART_IRQHANDLER       UART2_IRQHandler


// Debug UART defaults: 115200, 8N1.
#ifndef DEBUG_UART_BAUD_RATE
    #define DEBUG_UART_BAUD_RATE    115200
#endif

#ifndef DEBUG_UART_DATA_BITS
    #define DEBUG_UART_DATA_BITS    UART_LCR_WLEN8
#endif

#ifndef DEBUG_UART_PARITY
    #define DEBUG_UART_PARITY       UART_LCR_PARITY_DIS
#endif

#ifndef DEBUG_UART_STOP_BITS
    #define DEBUG_UART_STOP_BITS    UART_LCR_SBS_1BIT
#endif

#define DEBUG_UART_CONFIG           (DEBUG_UART_DATA_BITS | DEBUG_UART_PARITY \
                                    | DEBUG_UART_STOP_BITS)

// I2C defaults to Standard/Fast mode, 100 Khz
#ifndef BOARD_I2C_MODE
    #define BOARD_I2C_MODE          I2C0_STANDARD_FAST_MODE
#endif

#ifndef BOARD_I2C_SPEED
    #define BOARD_I2C_SPEED         1000000
#endif


// SPI default config: Master, 8 Bits, SPI format, CPHA0/CPOL0 polarity.
#ifndef BOARD_SPI_MODE
    #define BOARD_SPI_MODE          SSP_MODE_MASTER
#endif

#ifndef BOARD_SPI_BITS
    #define BOARD_SPI_BITS          SSP_BITS_8
#endif

#ifndef BOARD_SPI_FORMAT
    #define BOARD_SPI_FORMAT        SSP_FRAMEFORMAT_SPI
#endif

#ifndef BOARD_SPI_POLARITY
    #define BOARD_SPI_POLARITY      SSP_CLOCK_CPHA1_CPOL1
#endif

#ifndef BOARD_SPI_SPEED
    #define BOARD_SPI_SPEED         100000
#endif

// ADC maximum sampling rate: (4.5 Mhz / 11 bits) = ~400 Khz
#ifndef BOARD_ADC_SAMPLE_RATE
    #define BOARD_ADC_SAMPLE_RATE   400000
#endif

#ifndef BOARD_ADC_RESOLUTION
    #define BOARD_ADC_RESOLUTION    ADC_10BITS
#endif

/**
 * @brief	Returns the MAC address assigned to this board
 * @param	mcaddr : Pointer to 6-byte character array to populate with MAC address
 * @return	Nothing
 */
void Board_ENET_GetMacADDR(uint8_t *mcaddr);

void Board_SSP_Init(LPC_SSP_T *pSSP);

void Board_UART_Init(LPC_USART_T *pUART);

static void Board_I2C_Init();


#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
