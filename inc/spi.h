#ifndef SPI_H_
#define SPI_H_

#include "board.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void spi_init(void);

struct spi_transfer {
	Chip_SSP_DATA_SETUP_T xf_setup;
	uint32_t cs_change :1;
};

int32_t spi_sync_transfer(struct spi_transfer *xfers, uint32_t num_xfers,
		void (*gpio_wr_fsync)(bool));

/**
 * spi_write - SPI synchronous write
 * @spi: device to which data will be written
 * @buf: data buffer
 * @len: data buffer size
 *
 * This function writes the buffer @buf.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int spi_write(void *buf, size_t len)
{

	/* @formatter:off */
	struct spi_transfer t = {
			.xf_setup = { .tx_data = buf, .length = len },
			.cs_change = 0
	};
	/* @formatter:on */

	return spi_sync_transfer(&t, 1, NULL);
}

/**
 * spi_read - SPI synchronous read
 * @spi: device from which data will be read
 * @buf: data buffer
 * @len: data buffer size
 *
 * This function reads from SPI to the buffer @buf.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int spi_read(void *buf, size_t len)
{
	/* @formatter:off */
	struct spi_transfer t = {
			.xf_setup = { .rx_data = buf, .length = len },
			.cs_change = 0
	};
	/* @formatter:on */

	return spi_sync_transfer(&t, 1, NULL);
}

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */
