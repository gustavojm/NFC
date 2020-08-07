#ifndef __SPI_H__
#define __SPI_H__

#include "board.h"

struct spi_transfer {
	Chip_SSP_DATA_SETUP_T xf_setup;
	uint32_t cs_change :1;
};

#endif /* _SPI_H_ */
