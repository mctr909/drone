#ifndef __LSM9DS1_H__
#define __LSM9DS1_H__

#include "lsm9ds1_defines.h"

#define LSM9DS1_AG_ADDR(sa0)	((sa0) == 0 ? 0x6A : 0x6B)
#define LSM9DS1_M_ADDR(sa1)		((sa1) == 0 ? 0x1C : 0x1E)

struct TwoWire;

class LSM9DS1 {
public:
	IMU_SETTINGS settings;
	float gx, gy, gz;
	int16_t ax, ay, az;
	int16_t mx, my, mz;
	int16_t temperature;
	float bias_a[3];
	float bias_g[3];
	float bias_m[3];

protected:
	TwoWire *_port;
	uint8_t _addr_ag, _addr_m;
	// _res_g, _res_a, and _res_m store the current resolution for each sensor. 
	// Units of these values would be DPS (or g's or Gs's) per ADC tick.
	// This value is calculated as (sensor scale) / (2^15).
	float _res_a, _res_g, _res_m;
	int16_t _bias_raw_m[3];

public:
	LSM9DS1();

	// Initialize the gyro, accelerometer, and magnetometer.
	// This will set up the scale and output rate of each sensor. The values set
	// in the IMU_SETTINGS struct will take effect after calling this function.
	// ## INPUTS
	// - addr_ag - Sets either the I2C address of the accel/gyro or SPI chip 
	//   select pin connected to the CS_XG pin.
	// - addr_m - Sets either the I2C address of the magnetometer or SPI chip 
	//   select pin connected to the CS_M pin.
	// - i2C port (Note, only on "begin()" funtion, for use with I2C com interface)
	//   defaults to Wire, but if hardware supports it, can use other TwoWire ports.
	uint16_t begin(uint8_t addr_ag = LSM9DS1_AG_ADDR(1), uint8_t addr_m = LSM9DS1_M_ADDR(1), TwoWire &port = Wire);

	// Polls the accelerometer status register to check
	// if new data is available.
	// ## Output
	// - 1 - New data available
	// - 0 - No new data available
	uint8_t available_a();
	// Polls the gyroscope status register to check
	// if new data is available.
	// ## Output
	// - 1 - New data available
	// - 0 - No new data available
	uint8_t available_g();
	// Polls the temperature status register to check
	// if new data is available.
	// ## Output
	// - 1 - New data available
	// - 0 - No new data available
	uint8_t available_t();
	// Polls the accelerometer status register to check
	// if new data is available.
	// ## Input
	//	- axis can be either X_AXIS, Y_AXIS, Z_AXIS, to check for new data
	//	  on one specific axis. Or ALL_AXIS (default) to check for new data
	//	  on all axes.
	// ## Output
	// - 1 - New data available
	// - 0 - No new data available
	uint8_t available_m(LSM9DS1_AXIS axis = ALL_AXIS);

	// Read the accelerometer output registers.
	// This function will read all six accelerometer output registers.
	// ### The readings are stored in the class'
	// - ax, ay, az 
	// Read those after calling this function.
	void read_a();
	// Read the gyroscope output registers.
	// This function will read all six gyroscope output registers.
	// ### The readings are stored in the class'
	// - gx, gy, gz
	// Read those after calling this function.
	void read_g();
	// Read the temperature output register.
	// This function will read two temperature output registers.
	// ### The combined readings are stored in the class'
	// - temperature
	// Read those after calling this function.
	void read_t();
	// Read the magnetometer output registers.
	// This function will read all six magnetometer output registers.
	// ### The readings are stored in the class'
	// - mx, my, mz
	// Read those after calling this function.
	void read_m();

	// Convert from RAW signed 16-bit value to gravity (g's).
	// This function reads in a signed 16-bit value and returns the scaled
	// g's. This function relies on aScale and _res_a being correct.
	// ## Input
	//	- accel = A signed 16-bit raw reading from the accelerometer.
	float calc_a(int16_t accel);
	// Convert from RAW signed 16-bit value to Gauss (Gs)
	// This function reads in a signed 16-bit value and returns the scaled
	// Gs. This function relies on mScale and _res_m being correct.
	// ## Input
	//	- mag = A signed 16-bit raw reading from the magnetometer.
	float calc_m(int16_t mag);

	// Set the full-scale range of the accelerometer.
	// This function can be called to set the scale of the accelerometer to
	// 2, 4, 6, 8, or 16 g's.
	// ## Input
	// 	- scale = The desired accelerometer scale. Must be one of five possible
	//		values from the ACCEL_SCALE.
	void set_scale_a(uint8_t scale);
	// Set the full-scale range of the gyroscope.
	// This function can be called to set the scale of the gyroscope to 
	// 245, 500, or 200 degrees per second.
	// ## Input
	// 	- scale = The desired gyroscope scale. Must be one of three possible
	//		values from the GYRO_SCALE.
	void set_scale_g(uint16_t scale);
	// Set the full-scale range of the magnetometer.
	// This function can be called to set the scale of the magnetometer to
	// 2, 4, 8, or 12 Gs.
	// ## Input
	// 	- scale = The desired magnetometer scale. Must be one of four possible
	//		values from the MAG_SCALE.
	void set_scale_m(uint8_t scale);

	// Set the output data rate of the accelerometer
	// ## Input
	//	- rate = The desired output rate of the accel.
	void set_odr_a(uint8_t rate); 	
	// Set the output data rate and bandwidth of the gyroscope
	// ## Input
	//	- rate = The desired output rate and cutoff frequency of the gyro.
	void set_odr_g(uint8_t rate);
	// Set the output data rate of the magnetometer
	// ## Input
	//	- rate = The desired output rate of the mag.
	void set_odr_m(uint8_t rate);

	// This is a function that uses the FIFO to accumulate sample of accelerometer and gyro data, average
	// them, scales them to  gs and deg/s, respectively, and then passes the biases to the main sketch
	// for subtraction from all subsequent data. There are no gyro and accelerometer bias registers to store
	// the data as there are in the ADXL345, a precursor to the LSM9DS0, or the MPU-9150, so we have to
	// subtract the biases ourselves. This results in a more accurate measurement in general and can
	// remove errors due to imprecise or varying initial placement. Calibration of sensor data in this manner
	// is good practice.
	void calibrate_ag();
	void calibrate_m(bool loadin = true);
	void offset_m(uint8_t axis, int16_t offset);

protected:
	// Sets up gyro, accel, and mag settings to default.
	// to set com interface and/or addresses see begin() and beginSPI().
	void init();
	// Sets up the accelerometer to begin reading.
	// This function steps through all accelerometer related control registers.
	// Upon exit these registers will be set as:
	//	- CTRL_REG0_XM = 0x00: FIFO disabled. HPF bypassed. Normal mode.
	//	- CTRL_REG1_XM = 0x57: 100 Hz data rate. Continuous update.
	//		all axes enabled.
	//	- CTRL_REG2_XM = 0x00:  2g scale. 773 Hz anti-alias filter BW.
	//	- CTRL_REG3_XM = 0x04: Accel data ready signal on INT1_XM pin.
	void init_a();
	// Sets up the gyroscope to begin reading.
	// This function steps through all five gyroscope control registers.
	// Upon exit, the following parameters will be set:
	//	- CTRL_REG1_G = 0x0F: Normal operation mode, all axes enabled. 
	//		95 Hz ODR, 12.5 Hz cutoff frequency.
	//	- CTRL_REG2_G = 0x00: HPF set to normal mode, cutoff frequency
	//		set to 7.2 Hz (depends on ODR).
	//	- CTRL_REG3_G = 0x88: Interrupt enabled on INT_G (set to push-pull and
	//		active high). Data-ready output enabled on DRDY_G.
	//	- CTRL_REG4_G = 0x00: Continuous update mode. Data LSB stored in lower
	//		address. Scale set to 245 DPS. SPI mode set to 4-wire.
	//	- CTRL_REG5_G = 0x00: FIFO disabled. HPF disabled.
	void init_g();
	// Sets up the magnetometer to begin reading.
	// This function steps through all magnetometer-related control registers.
	// Upon exit these registers will be set as:
	//	- CTRL_REG4_XM = 0x04: Mag data ready signal on INT2_XM pin.
	//	- CTRL_REG5_XM = 0x14: 100 Hz update rate. Low resolution. Interrupt
	//		requests don't latch. Temperature sensor disabled.
	//	- CTRL_REG6_XM = 0x00:  2 Gs scale.
	//	- CTRL_REG7_XM = 0x00: Continuous conversion mode. Normal HPF mode.
	//	- INT_CTRL_REG_M = 0x09: Interrupt active-high. Enable interrupts.
	void init_m();

	void constrain_scales();
	// Calculate the resolution of the accelerometer.
	// This function will set the value of the _res_a variable. aScale must
	// be set prior to calling this function.
	void calc_res_a();
	// Calculate the resolution of the gyroscope.
	// This function will set the value of the _res_g variable. gScale must
	// be set prior to calling this function.
	void calc_res_g();
	// Calculate the resolution of the magnetometer.
	// This function will set the value of the _res_m variable. mScale must
	// be set prior to calling this function.
	void calc_res_m();

	// Read a byte from a register in the accel/mag sensor
	// ## Input
	//	- addr_sub = Register to be read from.
	// ## Output
	//	- An 8-bit value read from the requested register.
	uint8_t read_ag(uint8_t addr_sub);
	// Reads a byte from a specified gyroscope register.
	// ## Input
	// 	- addr_sub = Register to be read from.
	// ## Output
	// 	- An 8-bit value read from the requested address.
	uint8_t read_m(uint8_t addr_sub);
	// Write a byte to a register in the accel/mag sensor.
	// ## Input
	//	- addr_sub = Register to be written to.
	//	- data = data to be written to the register.
	void write_ag(uint8_t addr_sub, uint8_t data);
	// Write a byte to a register in the gyroscope.
	// ## Input
	//	- addr_sub = Register to be written to.
	//	- data = data to be written to the register.
	void write_m(uint8_t addr_sub, uint8_t data);
	// Read a series of bytes, starting at a register
	// ## Input
	//	- address = The 7-bit I2C address of the slave _device.
	//	- addr_sub = The register to begin reading.
	//	- *dest = Pointer to an array where we'll store the readings.
	//	- count = Number of registers to be read.
	// ## Output
	//	 No value is returned by the function, but the registers read are
	//	 all stored in the *dest array given.
	uint8_t i2c_read_bytes(uint8_t address, uint8_t addr_sub, uint8_t * dest, uint8_t count);

	// Enable or disable the FIFO
	// ## Input
	//	- enable: true = enable, false = disable.
	void enable_fifo(bool enable = true);
	// Configure FIFO mode and Threshold
	// ## Input
	//	- fifo_mode: Set FIFO mode to off, FIFO (stop when full), continuous, bypass
	//	  Possible inputs: FIFO_OFF, FIFO_THS, FIFO_CONT_TRIGGER, FIFO_OFF_TRIGGER, FIFO_CONT
	//	- fifo_threshold: FIFO threshold level setting
	//	  Any value from 0-0x1F is acceptable.
	void set_fifo(FIFO_MODE fifo_mode, uint8_t fifo_threshold);
	// Get number of FIFO samples
	uint8_t get_fifo_samples();
};

#endif // __LSM9DS1_H__ //
