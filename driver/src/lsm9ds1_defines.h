#ifndef __LSM9DS1_DEFINES_H__
#define __LSM9DS1_DEFINES_H__

#include <stdint.h>

// ACCEL_SCALE defines all possible FSR's of the accelerometer:
enum ACCEL_SCALE {
	A_SCALE_2G,  // 00:  2g
	A_SCALE_16G, // 01:  16g
	A_SCALE_4G,  // 10:  4g
	A_SCALE_8G   // 11:  8g
};

// GYRO_SCALE defines the possible full-scale ranges of the gyroscope:
enum GYRO_SCALE {
	G_SCALE_245DPS,  // 00:  245 degrees per second
	G_SCALE_500DPS,  // 01:  500 dps
	G_SCALE_2000DPS, // 11:  2000 dps
};

// MAG_SCALE defines all possible FSR's of the magnetometer:
enum MAG_SCALE {
	M_SCALE_4GS,  // 00:  4Gs
	M_SCALE_8GS,  // 01:  8Gs
	M_SCALE_12GS, // 10:  12Gs
	M_SCALE_16GS, // 11:  16Gs
};

// GYRO_ODR defines all possible data rate/bandwidth combos of the gyro:
enum GYRO_ODR {
	//! TODO
	G_ODR_PD,  // Power down (0)
	G_ODR_149, // 14.9 Hz (1)
	G_ODR_595, // 59.5 Hz (2)
	G_ODR_119, // 119 Hz (3)
	G_ODR_238, // 238 Hz (4)
	G_ODR_476, // 476 Hz (5)
	G_ODR_952  // 952 Hz (6)
};

// ACCEL_ODR defines all possible output data rates of the accelerometer:
enum ACCEL_ODR {
	XL_POWER_DOWN, // Power-down mode (0x0)
	XL_ODR_10,	 // 10 Hz (0x1)
	XL_ODR_50,	 // 50 Hz (0x02)
	XL_ODR_119,	// 119 Hz (0x3)
	XL_ODR_238,	// 238 Hz (0x4)
	XL_ODR_476,	// 476 Hz (0x5)
	XL_ODR_952	 // 952 Hz (0x6)
};

// MAG_ODR defines all possible output data rates of the magnetometer:
enum MAG_ODR {
	M_ODR_0625, // 0.625 Hz (0)
	M_ODR_125,  // 1.25 Hz (1)
	M_ODR_250,  // 2.5 Hz (2)
	M_ODR_5,	// 5 Hz (3)
	M_ODR_10,   // 10 Hz (4)
	M_ODR_20,   // 20 Hz (5)
	M_ODR_40,   // 40 Hz (6)
	M_ODR_80	// 80 Hz (7)
};

// ACCEL_ABW defines all possible anti-aliasing filter rates of the accelerometer:
enum ACCEL_ABW {
	A_ABW_408, // 408 Hz (0x0)
	A_ABW_211, // 211 Hz (0x1)
	A_ABW_105, // 105 Hz (0x2)
	A_ABW_50,  //  50 Hz (0x3)
};

enum FIFO_MODE {
	FIFO_OFF = 0,
	FIFO_THS = 1,
	FIFO_CONT_TRIGGER = 3,
	FIFO_OFF_TRIGGER = 4,
	FIFO_CONT = 6
};

enum LSM9DS1_AXIS {
	X_AXIS,
	Y_AXIS,
	Z_AXIS,
	ALL_AXIS
};

struct ACCEL_SETTINGS {
	uint8_t enabled;
	uint8_t scale;
	uint8_t sample_rate;
	uint8_t enable_x;
	uint8_t enable_y;
	uint8_t enable_z;
	int8_t bandwidth;
	uint8_t highres_enable;
	uint8_t highres_bandwidth;
};

struct GYRO_SETTINGS {
	uint8_t enabled;
	uint16_t scale;
	uint8_t sample_rate;
	uint8_t bandwidth;
	uint8_t lowpower_enable;
	uint8_t hpf_enable;
	uint8_t hpf_cutoff;
	uint8_t flip_x;
	uint8_t flip_y;
	uint8_t flip_z;
	uint8_t orientation;
	uint8_t enable_x;
	uint8_t enable_y;
	uint8_t enable_z;
	uint8_t latch_interrupt;
};

struct MAG_SETTINGS {
	uint8_t enabled;
	uint8_t scale;
	uint8_t sample_rate;
	uint8_t temp_compensation_enable;
	uint8_t xy_performance;
	uint8_t z_performance;
	uint8_t lowpower_enable;
	uint8_t operating_mode;
};

struct IMU_SETTINGS {
	GYRO_SETTINGS gyro;
	ACCEL_SETTINGS accel;
	MAG_SETTINGS mag;
	uint8_t temp_enabled;
};

#endif /* __LSM9DS1_DEFINES_H__ */
