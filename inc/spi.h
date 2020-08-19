#ifndef SPI_H_
#define SPI_H_

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

struct spi_transfer {
	Chip_SSP_DATA_SETUP_T xf_setup;
	uint32_t cs_change :1;
};

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */
