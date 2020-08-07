#include "board.h"
#include "stdio.h"
#include "spi.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define LPC_SSP           LPC_SSP1
#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER SSP1_IRQHandler


#define BUFFER_SIZE                         (0x100)
#define SSP_DATA_BITS                       (SSP_BITS_8)

/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;
static volatile uint8_t  isXferCompleted = 0;

//xf_setup.length = BUFFER_SIZE;
//xf_setup.tx_data = Tx_Buf;
//xf_setup.rx_data = Rx_Buf;
//
//xf_setup.rx_cnt = xf_setup.tx_cnt = 0;



spi_sync_transfer(struct spi_transfer *xfers, uint32_t num_xfers)
{
	uint32_t i;

	for (i = 0; i < num_xfers; ++i) {
		Chip_SSP_RWFrames_Blocking(LPC_SSP, &(xfers[i].xf_setup));

		if (xfers[i]->cs_change) {
			if (i != num_xfers) {
//				spi_set_cs(false);
//				//algo de delay aqui?;
//				spi_set_cs(true);
			}
		}
	}
}




int main(void)
{
	/* SSP initialization */
	Board_SSP_Init(LPC_SSP);

	Chip_SSP_Init(LPC_SSP);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE3;
        Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_Enable(LPC_SSP);

	Chip_SSP_SetMaster(LPC_SSP, 1);

}
