#include "board.h"
#include "stdio.h"
#include "spi.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define LPC_SSP           LPC_SSP1

#define SSP_DATA_BITS                       (SSP_BITS_8)

static SSP_ConfigFormat ssp_format;
SemaphoreHandle_t spi_mutex;


int32_t spi_sync_transfer(Chip_SSP_DATA_SETUP_T *xfers, uint32_t num_xfers,
		void (*gpio_wr_fsync)(bool))
{
	uint32_t i;
	if (spi_mutex != NULL) {
		if (xSemaphoreTake(spi_mutex, portMAX_DELAY) == pdTRUE) {
			for (i = 0; i < num_xfers; ++i) {
				if (gpio_wr_fsync != NULL) {
					gpio_wr_fsync(0);
				}
				Chip_SSP_RWFrames_Blocking(LPC_SSP, &(xfers[i]));

				if (gpio_wr_fsync != NULL) {
					gpio_wr_fsync(1);
				}

				if (i != num_xfers) {
					/* Delay WR/FSYNC falling edge to SCLK rising edge 3 ns min
					 Delay WR/FSYNC falling edge to SDO release from high-Z
					 VDRIVE = 4.5 V to 5.25 V 16 ns min */
					__NOP();	// 83ns delay at 12MHz
				}
			}
			xSemaphoreGive(spi_mutex);
		}
	}
	return 0;
}

void spi_init(void)
{
	spi_mutex = xSemaphoreCreateMutex();
	/* SSP initialization */
	Board_SSP_Init(LPC_SSP);

	Chip_SSP_Init(LPC_SSP);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE3;
	Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat,
			ssp_format.clockMode);
	Chip_SSP_Enable(LPC_SSP);

	Chip_SSP_SetMaster(LPC_SSP, 1);

}
