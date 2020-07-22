/*
 * ad2s1210.c
 *
 *  Created on: 1 jul. 2020
 *      Author: gustavo
 */

#include "ad2s1210.h"
#include <stdio.h>
#include <errno.h>
#include "semphr.h"
#include "FreeRTOS.h"
#include "task.h"

struct spi_transfer {
	const void *tx_buf;
	void *rx_buf;
	unsigned len;
	unsigned cs_change :1;
};

const unsigned int ad2s1210_resolution_value[] = { 10, 12, 14, 16 };

int spi_write(uint8_t *data, __attribute__((unused)) int byte_count);
int spi_sync_transfer(struct spi_transfer *xfer,
		__attribute__((unused)) int byte_count);

int spi_write(uint8_t *data, __attribute__((unused)) int byte_count)
{
	printf("0x%x \n", (uint8_t) *data);
	return 1;
}

int spi_sync_transfer(struct spi_transfer *xfer,
		__attribute__((unused)) int byte_count)
{
	printf("0x%s \n", (char*) xfer);
	return 1;
}

/* write 1 bytes (address or data) to the chip */
int ad2s1210_config_write(struct ad2s1210_state *st, uint8_t data)
{
	int ret = 0;

	st->tx[0] = data;
	ret = spi_write(st->tx, 1);

	return ret;
}

/* read value from one of the registers */
int ad2s1210_config_read(struct ad2s1210_state *st, uint8_t address)
{
	struct spi_transfer xfers[] = { { .len = 1, .rx_buf = &st->rx[0], .tx_buf =
			&st->tx[0], .cs_change = 1, }, { .len = 1, .rx_buf = &st->rx[1],
			.tx_buf = &st->tx[1], }, };
	int ret = 0;

	st->tx[0] = address | AD2S1210_MSB_IS_HIGH;
	st->tx[1] = AD2S1210_REG_FAULT;
	ret = spi_sync_transfer(xfers, 2);
	if (ret < 0)
		return ret;

	return st->rx[1];
}

int ad2s1210_update_frequency_control_word(struct ad2s1210_state *st)
{
	int ret = 0;
	uint8_t fcw;

	fcw = (uint8_t) (st->fexcit * (1 << 15) / st->fclkin);
	if (fcw < AD2S1210_MIN_FCW || fcw > AD2S1210_MAX_FCW) {
		printf("ad2s1210: FCW out of range\n");
		return -ERANGE;
	}

	ret = ad2s1210_config_write(st, AD2S1210_REG_EXCIT_FREQ);
	if (ret < 0)
		return ret;

	return ad2s1210_config_write(st, fcw);
}

int ad2s1210_soft_reset(struct ad2s1210_state *st)
{
	int ret;

	ret = ad2s1210_config_write(st, AD2S1210_REG_SOFT_RESET);
	if (ret < 0)
		return ret;

	return ad2s1210_config_write(st, 0x0);
}

void ad2s1210_hard_reset(struct ad2s1210_state *st)
{
	st->gpios.reset(0);
	vTaskDelay(pdMS_TO_TICKS(0.02));
	st->gpios.reset(1);
}

unsigned int ad2s1210_get_fclkin(struct ad2s1210_state *st)
{

	return st->fclkin;
}

int ad2s1210_set_fclkin(struct ad2s1210_state *st, unsigned int fclkin)
{

	int ret = 0;

	if (fclkin < AD2S1210_MIN_CLKIN || fclkin > AD2S1210_MAX_CLKIN) {
		printf("ad2s1210: fclkin out of range\n");
		return -EINVAL;
	}

	if (st->lock != NULL) {
		if ( xSemaphoreTake( st->lock, ( TickType_t ) 10 ) == pdTRUE) {

			st->fclkin = fclkin;

			ret = ad2s1210_update_frequency_control_word(st);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_soft_reset(st);
error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

unsigned int ad2s1210_get_fexcit(struct ad2s1210_state *st)
{

	return st->fexcit;
}

int ad2s1210_set_fexcit(struct ad2s1210_state *st, unsigned int fexcit)
{
	int ret = 0;

	if (fexcit < AD2S1210_MIN_EXCIT || fexcit > AD2S1210_MAX_EXCIT) {
		printf("ad2s1210: excitation frequency out of range\n");
		return -EINVAL;
	}
	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			st->fexcit = fexcit;
			ret = ad2s1210_update_frequency_control_word(st);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_soft_reset(st);
error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_get_control(struct ad2s1210_state *st)
{
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_set_control(struct ad2s1210_state *st, uint8_t udata)
{

	uint8_t data;
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {

			ret = ad2s1210_config_write(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;
			data = udata & AD2S1210_MSB_IS_LOW;
			ret = ad2s1210_config_write(st, data);
			if (ret < 0)
				goto error_ret;

			ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;
			if (ret & AD2S1210_MSB_IS_HIGH) {
				ret = -EIO;
				printf("ad2s1210: write control register fail\n");
				goto error_ret;
			}
			st->resolution = ad2s1210_resolution_value[data
					& AD2S1210_SET_RESOLUTION];
			st->hysteresis = !!(data & AD2S1210_ENABLE_HYSTERESIS);

error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

uint8_t ad2s1210_get_resolution(struct ad2s1210_state *st)
{

	return st->resolution;
}

int ad2s1210_set_resolution(struct ad2s1210_state *st, uint8_t udata)
{
	uint8_t data;
	int ret = 0;

	if (udata < 10 || udata > 16) {
		printf("ad2s1210: resolution out of range\n");
		return -EINVAL;
	}

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;
			data = ret;
			data &= ~AD2S1210_SET_RESOLUTION;
			data |= (udata - 10) >> 1;
			ret = ad2s1210_config_write(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_config_write(st, data & AD2S1210_MSB_IS_LOW);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;
			data = ret;
			if (data & AD2S1210_MSB_IS_HIGH) {
				ret = -EIO;
				printf("ad2s1210: setting resolution fail\n");
				goto error_ret;
			}
			st->resolution = ad2s1210_resolution_value[data
					& AD2S1210_SET_RESOLUTION];
error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

/* read the fault register since last sample */
int ad2s1210_get_fault(struct ad2s1210_state *st)
{
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {

			ret = ad2s1210_config_read(st, AD2S1210_REG_FAULT);
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_clear_fault(struct ad2s1210_state *st)
{
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			st->gpios.sample(0);
			/* delay (2 * tck + 20) nano seconds */
			vTaskDelay(pdMS_TO_TICKS(0.02));
			st->gpios.sample(1);
			ret = ad2s1210_config_read(st, AD2S1210_REG_FAULT);
			if (ret < 0)
				goto error_ret;

			st->gpios.sample(0);
			st->gpios.sample(1);

error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_get_reg(struct ad2s1210_state *st, uint8_t address)
{
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			ret = ad2s1210_config_read(st, address);
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_set_reg(struct ad2s1210_state *st, uint8_t address, uint8_t data)
{

	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			ret = ad2s1210_config_write(st, address);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_config_write(st, data & AD2S1210_MSB_IS_LOW);
error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_init(struct ad2s1210_state *st)
{
	uint8_t data;
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {

			ret = ad2s1210_config_write(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;
			data = AD2S1210_DEF_CONTROL & ~(AD2S1210_SET_RESOLUTION);
			data |= (st->resolution - 10) >> 1;
			ret = ad2s1210_config_write(st, data);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
			if (ret < 0)
				goto error_ret;

			if (ret & AD2S1210_MSB_IS_HIGH) {
				ret = -EIO;
				goto error_ret;
			}

			ret = ad2s1210_update_frequency_control_word(st);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_soft_reset(st);
error_ret:
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

int ad2s1210_read_position(struct ad2s1210_state *st)
{
	int ret = 0;

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, ( TickType_t ) 10 ) == pdTRUE) {
			ret = ad2s1210_config_read(st, AD2S1210_REG_POSITION);
			xSemaphoreGive(st->lock);
		} else {
			printf("unable to take mutex\n");
			ret = -EBUSY;
		}
	}
	return ret;
}

//Truncar a la resolución configurada
//if (st->hysteresis)
//	pos >>= 16 - st->resolution;
//*val = pos;
//
