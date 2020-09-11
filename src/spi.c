#include "board.h"
#include "stdio.h"
#include "spi.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "task.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define LPC_SSP           LPC_SSP1

#define SSP_DATA_BITS                       (SSP_BITS_8)

static SSP_ConfigFormat ssp_format;
static volatile uint8_t isXferCompleted = 0;

int32_t spi_sync_transfer(struct spi_transfer *xfers, uint32_t num_xfers,
		void (*gpio_wr_fsync)(bool))
{
	uint32_t i;

	for (i = 0; i < num_xfers; ++i) {
		Chip_SSP_RWFrames_Blocking(LPC_SSP, &(xfers[i].xf_setup));

		if (xfers[i].cs_change) {
			if (i != num_xfers) {
				if (gpio_wr_fsync != NULL) {
					gpio_wr_fsync(false);
					vTaskDelay(100);
					gpio_wr_fsync(true);
				}
			}
		}
	}
	return 0;
}

void spi_init(void)
{
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
