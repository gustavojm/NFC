#include "ad2s1210.h"
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "semphr.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "debug.h"

const uint32_t ad2s1210_resolution_value[] = { 10, 12, 14, 16 };

/* write 1 bytes (address or data) to the chip */
int32_t ad2s1210_config_write(struct ad2s1210_state *st, uint8_t data)
{
	int32_t ret = 0;

	uint8_t tx = data;
	ret = spi_write(&tx, 1);

	return ret;
}

/* read value from one of the registers */
static uint8_t ad2s1210_config_read(struct ad2s1210_state *st, uint8_t address)
{
	uint32_t xfers_count = 2;
	uint8_t rx[xfers_count];
	uint8_t tx[xfers_count];
	Chip_SSP_DATA_SETUP_T xfers[xfers_count];

	tx[0] = address | AD2S1210_ADDRESS_MASK;
	tx[1] = AD2S1210_REG_FAULT;	// Read some extra record to receive the data

	for (int i = 0; i < xfers_count; i++) {
		/* @formatter:off */
		Chip_SSP_DATA_SETUP_T xfer = {
			.length = 1,
			.rx_data = &rx[i],
			.tx_data = &tx[i],
			.rx_cnt = 0,
			.tx_cnt = 0
		};
	/* @formatter:on */

		xfers[i] = xfer;
	}
	int32_t ret = 0;
	ret = spi_sync_transfer(xfers, xfers_count, st->gpios.wr_fsync);
	if (ret < 0)
		return ret;

	return rx[1];
}

/* read value from two consecutive registers */
static uint16_t ad2s1210_config_read_two(struct ad2s1210_state *st, uint8_t address)
{
	uint32_t xfers_count = 3;
	uint8_t rx[xfers_count];
	uint8_t tx[xfers_count];
	Chip_SSP_DATA_SETUP_T xfers[xfers_count];

	tx[0] = address | AD2S1210_ADDRESS_MASK;
	tx[1] = address + 1 | AD2S1210_ADDRESS_MASK;
	tx[2] = AD2S1210_REG_FAULT;	// Read some extra record to receive the data

	for (int i = 0; i < xfers_count; i++) {
		/* @formatter:off */
		Chip_SSP_DATA_SETUP_T xfer = {
			.length = 1,
			.rx_data = &rx[i],
			.tx_data = &tx[i],
			.rx_cnt = 0,
			.tx_cnt = 0
		};
		/* @formatter:on */

		xfers[i] = xfer;
	}

	int32_t ret = 0;
	ret = spi_sync_transfer(xfers, xfers_count, st->gpios.wr_fsync);
	if (ret < 0)
		return ret;

	return rx[1] << 8 | rx[2];
}

static int32_t ad2s1210_update_frequency_control_word(struct ad2s1210_state *st)
{
	int32_t ret = 0;
	uint8_t fcw;

	fcw = (uint8_t) (st->fexcit * (1 << 15) / st->fclkin);
	if (fcw < AD2S1210_MIN_FCW || fcw > AD2S1210_MAX_FCW) {
		lDebug(Error, "ad2s1210: FCW out of range");
		return -ERANGE;
	}

	ret = ad2s1210_set_reg(st, AD2S1210_REG_EXCIT_FREQ, fcw);
}

int32_t ad2s1210_soft_reset(struct ad2s1210_state *st)
{
	int32_t ret;

	ret = ad2s1210_config_write(st, AD2S1210_REG_SOFT_RESET);
	return ret;
}

void ad2s1210_hard_reset(struct ad2s1210_state *st)
{
	st->gpios.reset(0);
	vTaskDelay(pdMS_TO_TICKS(1));
	st->gpios.reset(1);
}

uint32_t ad2s1210_get_fclkin(struct ad2s1210_state *st)
{
	return st->fclkin;
}

int32_t ad2s1210_set_fclkin(struct ad2s1210_state *st, uint32_t fclkin)
{
	int32_t ret = 0;

	if (fclkin < AD2S1210_MIN_CLKIN || fclkin > AD2S1210_MAX_CLKIN) {
		lDebug(Error, "ad2s1210: fclkin out of range");
		return -EINVAL;
	}

	if (st->lock != NULL) {
		if ( xSemaphoreTake( st->lock, portMAX_DELAY ) == pdTRUE) {

			st->fclkin = fclkin;

			ret = ad2s1210_update_frequency_control_word(st);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_soft_reset(st);
error_ret:
			xSemaphoreGive(st->lock);
		}
	}
	return ret;
}

uint32_t ad2s1210_get_fexcit(struct ad2s1210_state *st)
{
	return st->fexcit;
}

int32_t ad2s1210_set_fexcit(struct ad2s1210_state *st, uint32_t fexcit)
{
	int32_t ret = 0;

	if (fexcit < AD2S1210_MIN_EXCIT || fexcit > AD2S1210_MAX_EXCIT) {
		lDebug(Error, "ad2s1210: excitation frequency out of range");
		return -EINVAL;
	}

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, portMAX_DELAY ) == pdTRUE) {
			st->fexcit = fexcit;
			ret = ad2s1210_update_frequency_control_word(st);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_soft_reset(st);
error_ret:
			xSemaphoreGive(st->lock);
		}
	}
	return ret;
}

static uint8_t ad2s1210_get_control(struct ad2s1210_state *st)
{
	uint8_t ret = 0;

	ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
	return ret;
}

static int32_t ad2s1210_set_control(struct ad2s1210_state *st, uint8_t udata)
{
	uint8_t data;
	int32_t ret = 0;

	data = udata & AD2S1210_DATA_MASK;
	ret = ad2s1210_set_reg(st, AD2S1210_REG_CONTROL, data);
	if (ret < 0)
		return ret;

	ret = ad2s1210_config_read(st, AD2S1210_REG_CONTROL);
	if (ret < 0)
		return ret;
	if (ret & AD2S1210_MSB_MASK) {
		ret = -EIO;
		lDebug(Error, "ad2s1210: write control register fail");
		return ret;
	}
	st->resolution = ad2s1210_resolution_value[data & AD2S1210_RESOLUTION_MASK];
	st->hysteresis = !!(data & AD2S1210_HYSTERESIS);
	return ret;
}

uint8_t ad2s1210_get_resolution(struct ad2s1210_state *st)
{
	return st->resolution;
}

int32_t ad2s1210_set_resolution(struct ad2s1210_state *st, uint8_t res)
{
	uint8_t data;
	int32_t ret = 0;

	if (res < 10 || res > 16) {
		lDebug(Error, "ad2s1210: resolution out of range");
		return -EINVAL;
	}

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, portMAX_DELAY ) == pdTRUE) {

			ret = ad2s1210_get_control(st);
			if (ret < 0)
				goto error_ret;
			data = ret;
			data &= ~AD2S1210_RESOLUTION_MASK;
			data |= (res - 10) >> 1;
			ret = ad2s1210_set_control(st, data);

error_ret:
			xSemaphoreGive(st->lock);
		}
	}
	return ret;
}

uint8_t ad2s1210_get_reg(struct ad2s1210_state *st, uint8_t address)
{
	uint8_t ret = 0;

	ret = ad2s1210_config_read(st, address | AD2S1210_ADDRESS_MASK);
	return ret;
}

int32_t ad2s1210_set_reg(struct ad2s1210_state *st, uint8_t address,
		uint8_t data)
{
	int32_t ret = 0;

	ret = ad2s1210_config_write(st, address | AD2S1210_ADDRESS_MASK);
	if (ret < 0)
		return ret;
	ret = ad2s1210_config_write(st, data & AD2S1210_DATA_MASK);
	return ret;
}

int32_t ad2s1210_init(struct ad2s1210_state *st)
{
	uint8_t data;
	int32_t ret = 0;

	if (st->resolution < 10 || st->resolution > 16) {
		lDebug(Error, "ad2s1210: resolution out of range");
		return -EINVAL;
	}

	if (st->fclkin < AD2S1210_MIN_CLKIN || st->fclkin > AD2S1210_MAX_CLKIN) {
		lDebug(Error, "ad2s1210: fclkin out of range");
		return -EINVAL;
	}

	if (st->fexcit < AD2S1210_MIN_EXCIT || st->fexcit > AD2S1210_MAX_EXCIT) {
		lDebug(Error, "ad2s1210: excitation frequency out of range");
		return -EINVAL;
	}

	spi_init();

	if (st->lock != NULL) {
		if ( xSemaphoreTake(st->lock, portMAX_DELAY ) == pdTRUE) {
			data = AD2S1210_DEF_CONTROL & ~(AD2S1210_RESOLUTION_MASK);
			data |= (st->resolution - 10) >> 1;
			ret = ad2s1210_set_control(st, data);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_update_frequency_control_word(st);
			if (ret < 0)
				goto error_ret;
			ret = ad2s1210_soft_reset(st);
error_ret:
			xSemaphoreGive(st->lock);
		}
	}
	return ret;
}

int16_t ad2s1210_read_position(struct ad2s1210_state *st)
{
	int16_t pos = 0;

	/* The position and velocity registers are updated with a high-to-low
	 transition of the SAMPLE signal. This pin must be held low for at least t16 ns
	 to guarantee correct latching of the data. */
	st->gpios.sample(0);

	//No creo que haga falta delay ya que hay varias instrucciones previas
	//a leer el registro
	pos = ad2s1210_config_read_two(st, AD2S1210_REG_POSITION);
	if (st->hysteresis)
		pos >>= 16 - st->resolution;

	st->gpios.sample(1);
	return pos;
}

int16_t ad2s1210_read_velocity(struct ad2s1210_state *st)
{
	int16_t vel = 0;

	/* The position and velocity registers are updated with a high-to-low
	 transition of the SAMPLE signal. This pin must be held low for at least t16 ns
	 to guarantee correct latching of the data. */
	st->gpios.sample(0);

	//No creo que haga falta delay ya que hay varias instrucciones previas
	//a leer el registro
	vel = ad2s1210_config_read_two(st, AD2S1210_REG_VELOCITY);
	vel >>= 16 - st->resolution;
//	if (vel & 0x8000) {
//		int16_t negative = (0xffff >> st->resolution) << st->resolution;
//		vel |= negative;
//	}

	st->gpios.sample(1);
	return vel;
}

/* read the fault register since last sample */
uint8_t ad2s1210_get_fault_register(struct ad2s1210_state *st)
{
	uint8_t ret = 0;

	ret = ad2s1210_config_read(st, AD2S1210_REG_FAULT);
	return ret;
}

void ad2s1210_print_fault_register(uint8_t fr)
{
	if (fr & (1 << 0))
		lDebug(Info, "Configuration parity error");
	if (fr & (1 << 1))
		lDebug(Info, "Phase error exceeds phase lock range");
	if (fr & (1 << 2))
		lDebug(Info, "Velocity exceeds maximum tracking rate");
	if (fr & (1 << 3))
		lDebug(Info, "Tracking error exceeds LOT threshold");
	if (fr & (1 << 4))
		lDebug(Info, "Sine/cosine inputs exceed DOS mismatch threshold");
	if (fr & (1 << 5))
		lDebug(Info, "Sine/cosine inputs exceed DOS overrange threshold");
	if (fr & (1 << 6))
		lDebug(Info, "Sine/cosine inputs below LOS threshold");
	if (fr & (1 << 7))
		lDebug(Info, "Sine/cosine inputs clipped");
}

int32_t ad2s1210_clear_fault_register(struct ad2s1210_state *st)
{
	int32_t ret = 0;

	st->gpios.sample(0);
	/* delay (2 * tck + 20) nano seconds */
	udelay(1);
	st->gpios.sample(1);

	ret = ad2s1210_config_read(st, AD2S1210_REG_FAULT);
	if (ret < 0)
		return ret;

	st->gpios.sample(0);
	udelay(1);
	st->gpios.sample(1);

	return ret;
}

