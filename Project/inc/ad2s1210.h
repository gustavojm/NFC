#ifndef AD2S1210_H_
#define AD2S1210_H_

#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>

#define AD2S1210_DEF_CONTROL			0x7E

#define AD2S1210_MSB_IS_HIGH			1 << 7
#define AD2S1210_MSB_IS_LOW				~(1 << 7)
#define AD2S1210_PHASE_LOCK_RANGE_44	1 << 5
#define AD2S1210_ENABLE_HYSTERESIS		1 << 4
#define AD2S1210_SET_ENRES1				1 << 3
#define AD2S1210_SET_ENRES0				1 << 2
#define AD2S1210_SET_RES1				1 << 1
#define AD2S1210_SET_RES0				1 << 0

#define AD2S1210_SET_RESOLUTION		(AD2S1210_SET_RES1 | AD2S1210_SET_RES0)

#define AD2S1210_REG_POSITION			0x80
#define AD2S1210_REG_VELOCITY			0x82
#define AD2S1210_REG_LOS_THRD			0x88
#define AD2S1210_REG_DOS_OVR_THRD		0x89
#define AD2S1210_REG_DOS_MIS_THRD		0x8A
#define AD2S1210_REG_DOS_RST_MAX_THRD	0x8B
#define AD2S1210_REG_DOS_RST_MIN_THRD	0x8C
#define AD2S1210_REG_LOT_HIGH_THRD		0x8D
#define AD2S1210_REG_LOT_LOW_THRD		0x8E
#define AD2S1210_REG_EXCIT_FREQ			0x91
#define AD2S1210_REG_CONTROL			0x92
#define AD2S1210_REG_SOFT_RESET			0xF0
#define AD2S1210_REG_FAULT				0xFF

#define AD2S1210_MIN_CLKIN	6144000
#define AD2S1210_MAX_CLKIN	10240000
#define AD2S1210_MIN_EXCIT	2000
#define AD2S1210_MAX_EXCIT	20000
#define AD2S1210_MIN_FCW	0x4
#define AD2S1210_MAX_FCW	0x50

#define AD2S1210_DEF_EXCIT	10000

struct ad2s1210_gpios {
	void (*sample)(int);	//Pointer to functions to handle GPIO lines
	void (*wr_fsync)(int);
	void (*reset)(int);
};

struct ad2s1210_state {
	SemaphoreHandle_t lock;
	struct ad2s1210_gpios gpios;
	unsigned int fclkin;
	unsigned int fexcit;
	uint8_t hysteresis;
	uint8_t resolution;
	uint8_t rx[2];
	uint8_t tx[2];
};

/* write 1 bytes (address or data) to the chip */
int ad2s1210_config_write(struct ad2s1210_state *st, uint8_t data);

/* read value from one of the registers */
int ad2s1210_config_read(struct ad2s1210_state *st, uint8_t address);

int ad2s1210_update_frequency_control_word(struct ad2s1210_state *st);

int ad2s1210_soft_reset(struct ad2s1210_state *st);

void ad2s1210_hard_reset(struct ad2s1210_state *st);

unsigned int ad2s1210_get_fclkin(struct ad2s1210_state *st);

int ad2s1210_set_fclkin(struct ad2s1210_state *st, unsigned int fclkin);

unsigned int ad2s1210_get_fexcit(struct ad2s1210_state *st);

int ad2s1210_set_fexcit(struct ad2s1210_state *st, unsigned int fexcit);

int ad2s1210_get_control(struct ad2s1210_state *st);

int ad2s1210_set_control(struct ad2s1210_state *st, uint8_t udata);

uint8_t ad2s1210_get_resolution(struct ad2s1210_state *st);

int ad2s1210_set_resolution(struct ad2s1210_state *st, uint8_t udata);

/* read the fault register since last sample */
int ad2s1210_get_fault(struct ad2s1210_state *st);

int ad2s1210_clear_fault(struct ad2s1210_state *st);

int ad2s1210_get_reg(struct ad2s1210_state *st, uint8_t address);

int ad2s1210_set_reg(struct ad2s1210_state *st, uint8_t address, uint8_t data);

int ad2s1210_init(struct ad2s1210_state *st);

int ad2s1210_setup_gpios(struct ad2s1210_state *st);

int ad2s1210_read_position(struct ad2s1210_state *st);

#endif /* AD2S1210_H_ */
