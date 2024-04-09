#include <Wire.h>

#include "lsm9ds1.h"
#include "lsm9ds1_defines.h"

// Accel/Gyro Registers
#define ACT_THS				0x04
#define ACT_DUR				0x05
#define INT_GEN_CFG_XL		0x06
#define INT_GEN_THS_X_XL	0x07
#define INT_GEN_THS_Y_XL	0x08
#define INT_GEN_THS_Z_XL	0x09
#define INT_GEN_DUR_XL		0x0A
#define REFERENCE_G			0x0B
#define INT1_CTRL			0x0C
#define INT2_CTRL			0x0D
#define WHO_AM_I_XG			0x0F
#define CTRL_REG1_G			0x10
#define CTRL_REG2_G			0x11
#define CTRL_REG3_G			0x12
#define ORIENT_CFG_G		0x13
#define INT_GEN_SRC_G		0x14
#define OUT_TEMP_L			0x15
#define OUT_TEMP_H			0x16
#define STATUS_REG_0		0x17
#define OUT_X_L_G			0x18
#define OUT_X_H_G			0x19
#define OUT_Y_L_G			0x1A
#define OUT_Y_H_G			0x1B
#define OUT_Z_L_G			0x1C
#define OUT_Z_H_G			0x1D
#define CTRL_REG4			0x1E
#define CTRL_REG5_XL		0x1F
#define CTRL_REG6_XL		0x20
#define CTRL_REG7_XL		0x21
#define CTRL_REG8			0x22
#define CTRL_REG9			0x23
#define CTRL_REG10			0x24
#define INT_GEN_SRC_XL		0x26
#define STATUS_REG_1		0x27
#define OUT_X_L_XL			0x28
#define OUT_X_H_XL			0x29
#define OUT_Y_L_XL			0x2A
#define OUT_Y_H_XL			0x2B
#define OUT_Z_L_XL			0x2C
#define OUT_Z_H_XL			0x2D
#define FIFO_CTRL			0x2E
#define FIFO_SRC			0x2F
#define INT_GEN_CFG_G		0x30
#define INT_GEN_THS_XH_G	0x31
#define INT_GEN_THS_XL_G	0x32
#define INT_GEN_THS_YH_G	0x33
#define INT_GEN_THS_YL_G	0x34
#define INT_GEN_THS_ZH_G	0x35
#define INT_GEN_THS_ZL_G	0x36
#define INT_GEN_DUR_G		0x37

// Magneto Registers
#define OFFSET_X_REG_L_M	0x05
#define OFFSET_X_REG_H_M	0x06
#define OFFSET_Y_REG_L_M	0x07
#define OFFSET_Y_REG_H_M	0x08
#define OFFSET_Z_REG_L_M	0x09
#define OFFSET_Z_REG_H_M	0x0A
#define WHO_AM_I_M			0x0F
#define CTRL_REG1_M			0x20
#define CTRL_REG2_M			0x21
#define CTRL_REG3_M			0x22
#define CTRL_REG4_M			0x23
#define CTRL_REG5_M			0x24
#define STATUS_REG_M		0x27
#define OUT_X_L_M			0x28
#define OUT_X_H_M			0x29
#define OUT_Y_L_M			0x2A
#define OUT_Y_H_M			0x2B
#define OUT_Z_L_M			0x2C
#define OUT_Z_H_M			0x2D
#define INT_CFG_M			0x30
#define INT_SRC_M			0x31
#define INT_THS_L_M			0x32
#define INT_THS_H_M			0x33

// LSM9DS1 WHO_AM_I Responses
#define WHO_AM_I_AG_RSP		0x68
#define WHO_AM_I_M_RSP		0x3D

#define TO_RAD (3.14159265f / 180)

// Sensor Sensitivity Constants
// Values set according to the typical specifications provided in
// table 3 of the LSM9DS1 datasheet. (pg 12)
#define SENSITIVITY_ACCELEROMETER_2  0.000061
#define SENSITIVITY_ACCELEROMETER_4  0.000122
#define SENSITIVITY_ACCELEROMETER_8  0.000244
#define SENSITIVITY_ACCELEROMETER_16 0.000732
#define SENSITIVITY_GYROSCOPE_245    0.00875
#define SENSITIVITY_GYROSCOPE_500    0.0175
#define SENSITIVITY_GYROSCOPE_2000   0.07
#define SENSITIVITY_MAGNETOMETER_4   0.00014
#define SENSITIVITY_MAGNETOMETER_8   0.00029
#define SENSITIVITY_MAGNETOMETER_12  0.00043
#define SENSITIVITY_MAGNETOMETER_16  0.00058

LSM9DS1::LSM9DS1() { }

uint16_t LSM9DS1::begin(uint8_t addr_ag, uint8_t addr_m, TwoWire &port) {
	_addr_ag = addr_ag;
	_addr_m = addr_m;
	_port = &port;

	init();

	constrain_scales();

	calc_res_g(); // Calculate DPS / ADC tick, stored in _res_g variable
	calc_res_m(); // Calculate Gs / ADC tick, stored in _res_m variable
	calc_res_a(); // Calculate g / ADC tick, stored in _res_a variable

	uint8_t test_m = read_m(WHO_AM_I_M);
	uint8_t test_ag = read_ag(WHO_AM_I_XG);
	uint16_t who_am_i = (test_ag << 8) | test_m;
	if (who_am_i != ((WHO_AM_I_AG_RSP << 8) | WHO_AM_I_M_RSP)) {
		return 0;
	}

	init_g();
	init_a();
	init_m();

	return who_am_i;
}

uint8_t LSM9DS1::available_a() {
	uint8_t status = read_ag(STATUS_REG_1);
	return (status & (1<<0));
}
uint8_t LSM9DS1::available_g() {
	uint8_t status = read_ag(STATUS_REG_1);
	return ((status & (1<<1)) >> 1);
}
uint8_t LSM9DS1::available_t() {
	uint8_t status = read_ag(STATUS_REG_1);
	return ((status & (1<<2)) >> 2);
}
uint8_t LSM9DS1::available_m(LSM9DS1_AXIS axis) {
	uint8_t status;
	status = read_m(STATUS_REG_M);
	return ((status & (1<<axis)) >> axis);
}

void LSM9DS1::read_a() {
	uint8_t temp[6];
	if (6 == i2c_read_bytes(_addr_ag, OUT_X_L_XL, temp, 6)) {
		ax = (temp[1] << 8) | temp[0];
		ay = (temp[3] << 8) | temp[2];
		az = (temp[5] << 8) | temp[4];
	}
}
void LSM9DS1::read_g() {
	uint8_t temp[6];
	if (6 == i2c_read_bytes(_addr_ag, OUT_X_L_G, temp, 6)) {
		gx = (temp[1] << 8) | temp[0];
		gy = (temp[3] << 8) | temp[2];
		gz = (temp[5] << 8) | temp[4];
		gx *= _res_g;
		gy *= _res_g;
		gz *= _res_g;
	}
}
void LSM9DS1::read_t() {
	uint8_t temp[2];
	if (2 == i2c_read_bytes(_addr_ag, OUT_TEMP_L, temp, 2)) {
		int16_t offset = 25;  // Per datasheet sensor outputs 0 typically @ 25 degrees centigrade
		temperature = offset + ((((int16_t)temp[1] << 8) | temp[0]) >> 8) ;
	}
}
void LSM9DS1::read_m() {
	uint8_t temp[6];
	if (6 == i2c_read_bytes(_addr_m, OUT_X_L_M, temp, 6)) {
		mx = (temp[1] << 8) | temp[0];
		my = (temp[3] << 8) | temp[2];
		mz = (temp[5] << 8) | temp[4];
	}
}

float LSM9DS1::calc_a(int16_t accel) {
	// Return the accel raw reading times our pre-calculated g's / (ADC tick):
	return _res_a * accel;
}
float LSM9DS1::calc_m(int16_t mag) {
	// Return the mag raw reading times our pre-calculated Gs / (ADC tick):
	return _res_m * mag;
}

void LSM9DS1::set_scale_a(uint8_t scale) {
	// We need to preserve the other bytes in CTRL_REG6_XL. So, first read it:
	uint8_t temp_reg = read_ag(CTRL_REG6_XL);
	// Mask out accel scale bits:
	temp_reg &= 0xE7;
	switch (scale) {
	case 4:
		temp_reg |= (0x2 << 3);
		settings.accel.scale = 4;
		break;
	case 8:
		temp_reg |= (0x3 << 3);
		settings.accel.scale = 8;
		break;
	case 16:
		temp_reg |= (0x1 << 3);
		settings.accel.scale = 16;
		break;
	default: // Otherwise it'll be set to 2g (0x0 << 3)
		settings.accel.scale = 2;
		break;
	}
	write_ag(CTRL_REG6_XL, temp_reg);
	// Then calculate a new _res_a, which relies on aScale being set correctly:
	calc_res_a();
}
void LSM9DS1::set_scale_g(uint16_t scale) {
	// Read current value of CTRL_REG1_G:
	uint8_t temp_reg = read_ag(CTRL_REG1_G);
	// Mask out scale bits (3 & 4):
	temp_reg &= 0xE7;
	switch (scale) {
	case 500:
		temp_reg |= (0x1 << 3);
		settings.gyro.scale = 500;
		break;
	case 2000:
		temp_reg |= (0x3 << 3);
		settings.gyro.scale = 2000;
		break;
	default: // Otherwise we'll set it to 245 dps (0x0 << 4)
		settings.gyro.scale = 245;
		break;
	}
	write_ag(CTRL_REG1_G, temp_reg);
	calc_res_g();	
}
void LSM9DS1::set_scale_m(uint8_t scale) {
	// We need to preserve the other bytes in CTRL_REG6_XM. So, first read it:
	uint8_t temp_reg = read_m(CTRL_REG2_M);
	// Then mask out the mag scale bits:
	temp_reg &= 0xFF^(0x3 << 5);
	switch (scale) {
	case 8:
		temp_reg |= (0x1 << 5);
		settings.mag.scale = 8;
		break;
	case 12:
		temp_reg |= (0x2 << 5);
		settings.mag.scale = 12;
		break;
	case 16:
		temp_reg |= (0x3 << 5);
		settings.mag.scale = 16;
		break;
	default: // Otherwise we'll default to 4 gauss (00)
		settings.mag.scale = 4;
		break;
	}
	// And write the new register value back into CTRL_REG6_XM:
	write_m(CTRL_REG2_M, temp_reg);
	// We've updated the sensor, but we also need to update our class variables
	// First update mScale:
	//mScale = scale;
	// Then calculate a new _res_m, which relies on mScale being set correctly:
	calc_res_m();
}

void LSM9DS1::set_odr_a(uint8_t rate) {
	// Only do this if rate is not 0 (which would disable the accel)
	if ((rate & 0x07) != 0) {
		// We need to preserve the other bytes in CTRL_REG1_XM. So, first read it:
		uint8_t temp = read_ag(CTRL_REG6_XL);
		// Then mask out the accel ODR bits:
		temp &= 0x1F;
		// Then shift in our new ODR bits:
		temp |= ((rate & 0x07) << 5);
		settings.accel.sample_rate = rate & 0x07;
		// And write the new register value back into CTRL_REG1_XM:
		write_ag(CTRL_REG6_XL, temp);
	}
}
void LSM9DS1::set_odr_g(uint8_t rate) {
	// Only do this if rate is not 0 (which would disable the gyro)
	if ((rate & 0x07) != 0) {
		// We need to preserve the other bytes in CTRL_REG1_G. So, first read it:
		uint8_t temp = read_ag(CTRL_REG1_G);
		// Then mask out the gyro ODR bits:
		temp &= 0xFF^(0x7 << 5);
		temp |= (rate & 0x07) << 5;
		// Update our settings struct
		settings.gyro.sample_rate = rate & 0x07;
		// And write the new register value back into CTRL_REG1_G:
		write_ag(CTRL_REG1_G, temp);
	}
}
void LSM9DS1::set_odr_m(uint8_t rate) {
	// We need to preserve the other bytes in CTRL_REG5_XM. So, first read it:
	uint8_t temp = read_m(CTRL_REG1_M);
	// Then mask out the mag ODR bits:
	temp &= 0xFF^(0x7 << 2);
	// Then shift in our new ODR bits:
	temp |= ((rate & 0x07) << 2);
	settings.mag.sample_rate = rate & 0x07;
	// And write the new register value back into CTRL_REG5_XM:
	write_m(CTRL_REG1_M, temp);
}

void LSM9DS1::calibrate_ag() {
	uint8_t samples = 0;
	int ii;
	int32_t temp_bias_a[3] = {0, 0, 0};
	float temp_bias_g[3] = {0, 0, 0};
	// Turn on FIFO and set threshold to 32 samples
	enable_fifo(true);
	set_fifo(FIFO_THS, 0x1F);
	while (samples < 0x1F) {
		samples = (read_ag(FIFO_SRC) & 0x3F); // Read number of stored samples
	}
	for(ii = 0; ii < samples; ii++) {
		// Read the gyro data stored in the FIFO
		read_g();
		temp_bias_g[0] += gx;
		temp_bias_g[1] += gy;
		temp_bias_g[2] += gz;
		read_a();
		temp_bias_a[0] += ax;
		temp_bias_a[1] += ay;
		temp_bias_a[2] += az - (int16_t)(1./_res_a); // Assumes sensor facing up!
	}
	for (ii = 0; ii < 3; ii++) {
		bias_a[ii] = calc_a(temp_bias_a[ii] / samples);
		bias_g[ii] = temp_bias_g[ii] / samples;
	}
	enable_fifo(false);
	set_fifo(FIFO_OFF, 0x00);
}
void LSM9DS1::calibrate_m(bool loadIn) {
	int i, j;
	int16_t mag_min[3] = {0, 0, 0};
	int16_t mag_max[3] = {0, 0, 0}; // The road warrior
	for (i=0; i<128; i++) {
		while (!available_m())
		;
		read_m();
		int16_t temp_mag[3] = {0, 0, 0};
		temp_mag[0] = mx;		
		temp_mag[1] = my;
		temp_mag[2] = mz;
		for (j = 0; j < 3; j++) {
			if (temp_mag[j] > mag_max[j]) mag_max[j] = temp_mag[j];
			if (temp_mag[j] < mag_min[j]) mag_min[j] = temp_mag[j];
		}
	}
	for (j = 0; j < 3; j++) {
		_bias_raw_m[j] = (mag_max[j] + mag_min[j]) / 2;
		bias_m[j] = calc_m(_bias_raw_m[j]);
		if (loadIn)
			offset_m(j, _bias_raw_m[j]);
	}
}
void LSM9DS1::offset_m(uint8_t axis, int16_t offset) {
	if (axis > 2)
		return;
	uint8_t msb, lsb;
	msb = (offset & 0xFF00) >> 8;
	lsb = offset & 0x00FF;
	write_m(OFFSET_X_REG_L_M + (2 * axis), lsb);
	write_m(OFFSET_X_REG_H_M + (2 * axis), msb);
}


void LSM9DS1::init() {
	settings.gyro.enabled = true;
	settings.gyro.enable_x = true;
	settings.gyro.enable_y = true;
	settings.gyro.enable_z = true;
	// gyro scale can be 245, 500, or 2000
	settings.gyro.scale = 245;
	// gyro sample rate: value between 1-6
	// 1 = 14.9    4 = 238
	// 2 = 59.5    5 = 476
	// 3 = 119     6 = 952
	settings.gyro.sample_rate = 6;
	// gyro cutoff frequency: value between 0-3
	// Actual value of cutoff frequency depends
	// on sample rate.
	settings.gyro.bandwidth = 0;
	settings.gyro.lowpower_enable = false;
	settings.gyro.hpf_enable = false;
	// Gyro HPF cutoff frequency: value between 0-9
	// Actual value depends on sample rate. Only applies
	// if gyroHPFEnable is true.
	settings.gyro.hpf_cutoff = 0;
	settings.gyro.flip_x = false;
	settings.gyro.flip_y = false;
	settings.gyro.flip_z = false;
	settings.gyro.orientation = 0;
	settings.gyro.latch_interrupt = true;

	settings.accel.enabled = true;
	settings.accel.enable_x = true;
	settings.accel.enable_y = true;
	settings.accel.enable_z = true;
	// accel scale can be 2, 4, 8, or 16
	settings.accel.scale = 2;
	// accel sample rate can be 1-6
	// 1 = 10 Hz    4 = 238 Hz
	// 2 = 50 Hz    5 = 476 Hz
	// 3 = 119 Hz   6 = 952 Hz
	settings.accel.sample_rate = 6;
	// Accel cutoff freqeuncy can be any value between -1 - 3. 
	// -1 = bandwidth determined by sample rate
	// 0 = 408 Hz   2 = 105 Hz
	// 1 = 211 Hz   3 = 50 Hz
	settings.accel.bandwidth = -1;
	settings.accel.highres_enable = false;
	// accelHighResBandwidth can be any value between 0-3
	// LP cutoff is set to a factor of sample rate
	// 0 = ODR/50    2 = ODR/9
	// 1 = ODR/100   3 = ODR/400
	settings.accel.highres_bandwidth = 0;

	settings.mag.enabled = true;
	// mag scale can be 4, 8, 12, or 16
	settings.mag.scale = 4;
	// mag data rate can be 0-7
	// 0 = 0.625 Hz  4 = 10 Hz
	// 1 = 1.25 Hz   5 = 20 Hz
	// 2 = 2.5 Hz    6 = 40 Hz
	// 3 = 5 Hz      7 = 80 Hz
	settings.mag.sample_rate = 7;
	settings.mag.temp_compensation_enable = false;
	// magPerformance can be any value between 0-3
	// 0 = Low power mode      2 = high performance
	// 1 = medium performance  3 = ultra-high performance
	settings.mag.xy_performance = 3;
	settings.mag.z_performance = 3;
	settings.mag.lowpower_enable = false;
	// magOperatingMode can be 0-2
	// 0 = continuous conversion
	// 1 = single-conversion
	// 2 = power down
	settings.mag.operating_mode = 0;

	settings.temp_enabled = true;
	for (int i=0; i<3; i++) {
		bias_g[i] = 0;
		bias_a[i] = 0;
		bias_m[i] = 0;
		_bias_raw_m[i] = 0;
	}
}
void LSM9DS1::init_a() {
	uint8_t temp_reg = 0;

	//	CTRL_REG5_XL (0x1F) (Default value: 0x38)
	//	[DEC_1][DEC_0][Zen_XL][Yen_XL][Zen_XL][0][0][0]
	//	DEC[0:1] - Decimation of accel data on OUT REG and FIFO.
	//		00: None, 01: 2 samples, 10: 4 samples 11: 8 samples
	//	Zen_XL - Z-axis output enabled
	//	Yen_XL - Y-axis output enabled
	//	Xen_XL - X-axis output enabled
	if (settings.accel.enable_z) temp_reg |= (1<<5);
	if (settings.accel.enable_y) temp_reg |= (1<<4);
	if (settings.accel.enable_x) temp_reg |= (1<<3);

	write_ag(CTRL_REG5_XL, temp_reg);

	// CTRL_REG6_XL (0x20) (Default value: 0x00)
	// [ODR_XL2][ODR_XL1][ODR_XL0][FS1_XL][FS0_XL][BW_SCAL_ODR][BW_XL1][BW_XL0]
	// ODR_XL[2:0] - Output data rate & power mode selection
	// FS_XL[1:0] - Full-scale selection
	// BW_SCAL_ODR - Bandwidth selection
	// BW_XL[1:0] - Anti-aliasing filter bandwidth selection
	temp_reg = 0;
	// To disable the accel, set the sampleRate bits to 0.
	if (settings.accel.enabled) {
		temp_reg |= (settings.accel.sample_rate & 0x07) << 5;
	}
	switch (settings.accel.scale) {
	case 4:
		temp_reg |= (0x2 << 3);
		break;
	case 8:
		temp_reg |= (0x3 << 3);
		break;
	case 16:
		temp_reg |= (0x1 << 3);
		break;
	// Otherwise it'll be set to 2g (0x0 << 3)
	}
	if (settings.accel.bandwidth >= 0) {
		temp_reg |= (1<<2); // Set BW_SCAL_ODR
		temp_reg |= (settings.accel.bandwidth & 0x03);
	}
	write_ag(CTRL_REG6_XL, temp_reg);

	// CTRL_REG7_XL (0x21) (Default value: 0x00)
	// [HR][DCF1][DCF0][0][0][FDS][0][HPIS1]
	// HR - High resolution mode (0: disable, 1: enable)
	// DCF[1:0] - Digital filter cutoff frequency
	// FDS - Filtered data selection
	// HPIS1 - HPF enabled for interrupt function
	temp_reg = 0;
	if (settings.accel.highres_enable) {
		temp_reg |= (1<<7); // Set HR bit
		temp_reg |= (settings.accel.highres_bandwidth & 0x3) << 5;
	}
	write_ag(CTRL_REG7_XL, temp_reg);
}
void LSM9DS1::init_g() {
	uint8_t temp_reg = 0;

	// CTRL_REG1_G (Default value: 0x00)
	// [ODR_G2][ODR_G1][ODR_G0][FS_G1][FS_G0][0][BW_G1][BW_G0]
	// ODR_G[2:0] - Output data rate selection
	// FS_G[1:0] - Gyroscope full-scale selection
	// BW_G[1:0] - Gyroscope bandwidth selection

	// To disable gyro, set sample rate bits to 0. We'll only set sample
	// rate if the gyro is enabled.
	if (settings.gyro.enabled) {
		temp_reg = (settings.gyro.sample_rate & 0x07) << 5;
	}
	switch (settings.gyro.scale) {
	case 500:
		temp_reg |= (0x1 << 3);
		break;
	case 2000:
		temp_reg |= (0x3 << 3);
		break;
	// Otherwise we'll set it to 245 dps (0x0 << 4)
	}
	temp_reg |= (settings.gyro.bandwidth & 0x3);
	write_ag(CTRL_REG1_G, temp_reg);

	// CTRL_REG2_G (Default value: 0x00)
	// [0][0][0][0][INT_SEL1][INT_SEL0][OUT_SEL1][OUT_SEL0]
	// INT_SEL[1:0] - INT selection configuration
	// OUT_SEL[1:0] - Out selection configuration
	write_ag(CTRL_REG2_G, 0x00);	

	// CTRL_REG3_G (Default value: 0x00)
	// [LP_mode][HP_EN][0][0][HPCF3_G][HPCF2_G][HPCF1_G][HPCF0_G]
	// LP_mode - Low-power mode enable (0: disabled, 1: enabled)
	// HP_EN - HPF enable (0:disabled, 1: enabled)
	// HPCF_G[3:0] - HPF cutoff frequency
	temp_reg = settings.gyro.lowpower_enable ? (1<<7) : 0;
	if (settings.gyro.hpf_enable) {
		temp_reg |= (1<<6) | (settings.gyro.hpf_cutoff & 0x0F);
	}
	write_ag(CTRL_REG3_G, temp_reg);

	// CTRL_REG4 (Default value: 0x38)
	// [0][0][Zen_G][Yen_G][Xen_G][0][LIR_XL1][4D_XL1]
	// Zen_G - Z-axis output enable (0:disable, 1:enable)
	// Yen_G - Y-axis output enable (0:disable, 1:enable)
	// Xen_G - X-axis output enable (0:disable, 1:enable)
	// LIR_XL1 - Latched interrupt (0:not latched, 1:latched)
	// 4D_XL1 - 4D option on interrupt (0:6D used, 1:4D used)
	temp_reg = 0;
	if (settings.gyro.enable_z) temp_reg |= (1<<5);
	if (settings.gyro.enable_y) temp_reg |= (1<<4);
	if (settings.gyro.enable_x) temp_reg |= (1<<3);
	if (settings.gyro.latch_interrupt) temp_reg |= (1<<1);
	write_ag(CTRL_REG4, temp_reg);

	// ORIENT_CFG_G (Default value: 0x00)
	// [0][0][SignX_G][SignY_G][SignZ_G][Orient_2][Orient_1][Orient_0]
	// SignX_G - Pitch axis (X) angular rate sign (0: positive, 1: negative)
	// Orient [2:0] - Directional user orientation selection
	temp_reg = 0;
	if (settings.gyro.flip_x) temp_reg |= (1<<5);
	if (settings.gyro.flip_y) temp_reg |= (1<<4);
	if (settings.gyro.flip_z) temp_reg |= (1<<3);
	write_ag(ORIENT_CFG_G, temp_reg);
}
void LSM9DS1::init_m() {
	uint8_t temp_reg = 0;

	// CTRL_REG1_M (Default value: 0x10)
	// [TEMP_COMP][OM1][OM0][DO2][DO1][DO0][0][ST]
	// TEMP_COMP - Temperature compensation
	// OM[1:0] - X & Y axes op mode selection
	//	00:low-power, 01:medium performance
	//	10: high performance, 11:ultra-high performance
	// DO[2:0] - Output data rate selection
	// ST - Self-test enable
	if (settings.mag.temp_compensation_enable) temp_reg |= (1<<7);
	temp_reg |= (settings.mag.xy_performance & 0x3) << 5;
	temp_reg |= (settings.mag.sample_rate & 0x7) << 2;
	write_m(CTRL_REG1_M, temp_reg);

	// CTRL_REG2_M (Default value 0x00)
	// [0][FS1][FS0][0][REBOOT][SOFT_RST][0][0]
	// FS[1:0] - Full-scale configuration
	// REBOOT - Reboot memory content (0:normal, 1:reboot)
	// SOFT_RST - Reset config and user registers (0:default, 1:reset)
	temp_reg = 0;
	switch (settings.mag.scale) {
	case 8:
		temp_reg |= (0x1 << 5);
		break;
	case 12:
		temp_reg |= (0x2 << 5);
		break;
	case 16:
		temp_reg |= (0x3 << 5);
		break;
	// Otherwise we'll default to 4 gauss (00)
	}
	write_m(CTRL_REG2_M, temp_reg); // +/-4Gauss

	// CTRL_REG3_M (Default value: 0x03)
	// [I2C_DISABLE][0][LP][0][0][SIM][MD1][MD0]
	// I2C_DISABLE - Disable I2C interace (0:enable, 1:disable)
	// LP - Low-power mode cofiguration (1:enable)
	// SIM - SPI mode selection (0:write-only, 1:read/write enable)
	// MD[1:0] - Operating mode
	//	00:continuous conversion, 01:single-conversion,
	//  10,11: Power-down
	temp_reg = 0;
	if (settings.mag.lowpower_enable) temp_reg |= (1<<5);
	temp_reg |= (settings.mag.operating_mode & 0x3);
	write_m(CTRL_REG3_M, temp_reg); // Continuous conversion mode

	// CTRL_REG4_M (Default value: 0x00)
	// [0][0][0][0][OMZ1][OMZ0][BLE][0]
	// OMZ[1:0] - Z-axis operative mode selection
	//	00:low-power mode, 01:medium performance
	//	10:high performance, 10:ultra-high performance
	// BLE - Big/little endian data
	temp_reg = 0;
	temp_reg = (settings.mag.z_performance & 0x3) << 2;
	write_m(CTRL_REG4_M, temp_reg);

	// CTRL_REG5_M (Default value: 0x00)
	// [0][BDU][0][0][0][0][0][0]
	// BDU - Block data update for magnetic data
	//	0:continuous, 1:not updated until MSB/LSB are read
	temp_reg = 0;
	write_m(CTRL_REG5_M, temp_reg);
}

void LSM9DS1::constrain_scales() {
	if ((settings.gyro.scale != 245) &&
		(settings.gyro.scale != 500) && 
		(settings.gyro.scale != 2000)
	) {
		settings.gyro.scale = 245;
	}
	if ((settings.accel.scale != 2) &&
		(settings.accel.scale != 4) &&
		(settings.accel.scale != 8) &&
		(settings.accel.scale != 16)
	) {
		settings.accel.scale = 2;
	}
	if ((settings.mag.scale != 4) &&
		(settings.mag.scale != 8) &&
		(settings.mag.scale != 12) &&
		(settings.mag.scale != 16)
	) {
		settings.mag.scale = 4;
	}
}
void LSM9DS1::calc_res_a() {
	switch (settings.accel.scale) {
	case 2:
		_res_a = SENSITIVITY_ACCELEROMETER_2;
		break;
	case 4:
		_res_a = SENSITIVITY_ACCELEROMETER_4;
		break;
	case 8:
		_res_a = SENSITIVITY_ACCELEROMETER_8;
		break;
	case 16:
		_res_a = SENSITIVITY_ACCELEROMETER_16;
		break;
	default:
		break;
	}
}
void LSM9DS1::calc_res_g() {
	switch (settings.gyro.scale) {
	case 245:
		_res_g = SENSITIVITY_GYROSCOPE_245 * TO_RAD;
		break;
	case 500:
		_res_g = SENSITIVITY_GYROSCOPE_500 * TO_RAD;
		break;
	case 2000:
		_res_g = SENSITIVITY_GYROSCOPE_2000 * TO_RAD;
		break;
	default:
		break;
	}
}
void LSM9DS1::calc_res_m() {
	switch (settings.mag.scale) {
	case 4:
		_res_m = SENSITIVITY_MAGNETOMETER_4;
		break;
	case 8:
		_res_m = SENSITIVITY_MAGNETOMETER_8;
		break;
	case 12:
		_res_m = SENSITIVITY_MAGNETOMETER_12;
		break;
	case 16:
		_res_m = SENSITIVITY_MAGNETOMETER_16;
		break;
	}	
}

uint8_t LSM9DS1::read_ag(uint8_t addr_sub) {
	uint8_t data;
	_port->beginTransmission(_addr_ag);
	_port->write(addr_sub);
	_port->endTransmission(false);
	_port->requestFrom(_addr_ag, (uint8_t) 1);
	data = _port->read();
	return data;
}
uint8_t LSM9DS1::read_m(uint8_t addr_sub) {
	uint8_t data;
	_port->beginTransmission(_addr_m);
	_port->write(addr_sub);
	_port->endTransmission(false);
	_port->requestFrom(_addr_m, (uint8_t) 1);
	data = _port->read();
	return data;
}
void LSM9DS1::write_ag(uint8_t addr_sub, uint8_t data) {
	_port->beginTransmission(_addr_ag);
	_port->write(addr_sub);
	_port->write(data);
	_port->endTransmission();
}
void LSM9DS1::write_m(uint8_t addr_sub, uint8_t data) {
	_port->beginTransmission(_addr_m);
	_port->write(addr_sub);
	_port->write(data);
	_port->endTransmission();
}
uint8_t LSM9DS1::i2c_read_bytes(uint8_t address, uint8_t addr_sub, uint8_t * dest, uint8_t count) {
	uint8_t ret;
	_port->beginTransmission(address);
	_port->write(addr_sub | 0x80);
	ret = _port->endTransmission(false);
	if (ret != 0) {
		return 0;
	}
	ret = _port->requestFrom(address, count);
	if (ret != count) {
		return 0;
	}
	for (int i=0; i<count;) {
		dest[i++] = _port->read();
	}
	return count;
}

void LSM9DS1::enable_fifo(bool enable) {
	uint8_t temp = read_ag(CTRL_REG9);
	if (enable) temp |= (1<<1);
	else temp &= ~(1<<1);
	write_ag(CTRL_REG9, temp);
}
void LSM9DS1::set_fifo(FIFO_MODE fifo_mode, uint8_t fifo_threshold) {
	// Limit threshold - 0x1F (31) is the maximum. If more than that was asked
	// limit it to the maximum.
	uint8_t threshold = fifo_threshold <= 0x1F ? fifo_threshold : 0x1F;
	write_ag(FIFO_CTRL, ((fifo_mode & 0x7) << 5) | (threshold & 0x1F));
}
uint8_t LSM9DS1::get_fifo_samples() {
	return (read_ag(FIFO_SRC) & 0x3F);
}
