/*

This is a library for the BH1750FVI Digital Light Sensor breakout board.

The BH1750 board uses I2C for communication. Two pins are required to
interface to the device. Configuring the I2C bus is expected to be done
in user code. The BH1750 library doesn't do this automatically.

Written by Christopher Laws, March, 2013.

*/

#include "BH1750.h"


// Use _delay_ms from utils for AVR-based platforms
#include <util/delay.h>


/**
* Constructor
* @params addr Sensor address (0x76 or 0x72, see data sheet)
*
* On most sensor boards, it was 0x76
*/
BH1750::BH1750(uint8_t addr) {

	BH1750_I2CADDR = addr;
	// Allows user to change TwoWire instance
	I2C = &TinyI2C;
}

/**
* Configure sensor
* @param mode Measurement mode
* @param addr Address of the sensor
* @param i2c TwoWire instance connected to I2C bus
*/
bool BH1750::begin(Mode mode, uint8_t addr, TinyI2CMaster* i2c) {

	// I2C is expected to be initialized outside this library
	// But, allows a different address and TwoWire instance to be used
	if (i2c) {
		I2C = i2c;
	}
	if (addr) {
		BH1750_I2CADDR = addr;
	}

	// Configure sensor in specified mode and set default MTreg
	return (configure(mode) && setMTreg(BH1750_DEFAULT_MTREG));
}

/**
* Configure BH1750 with specified mode
* @param mode Measurement mode
*/
bool BH1750::configure(Mode mode) {

	// Check measurement mode is valid
	switch (mode) {

		case BH1750::CONTINUOUS_HIGH_RES_MODE:
		case BH1750::CONTINUOUS_HIGH_RES_MODE_2:
		case BH1750::CONTINUOUS_LOW_RES_MODE:
		case BH1750::ONE_TIME_HIGH_RES_MODE:
		case BH1750::ONE_TIME_HIGH_RES_MODE_2:
		case BH1750::ONE_TIME_LOW_RES_MODE:

		// Send mode to sensor
		I2C->start(BH1750_I2CADDR, 0);
		I2C->write((uint8_t)mode);
		I2C->stop();
		BH1750_MODE = mode;
	}
	return true;

}

/**
* Configure BH1750 MTreg value
* MT reg = Measurement Time register
* @param MTreg a value between 31 and 254. Default: 69
* @return bool true if MTReg successful set
* 		false if MTreg not changed or parameter out of range
*/
bool BH1750::setMTreg(uint8_t MTreg) {
	if (MTreg < BH1750_MTREG_MIN || MTreg > BH1750_MTREG_MAX) {
		// Serial.println(F("[BH1750] ERROR: MTreg out of range"));
		return false;
	}
	uint8_t ack = 5;
	// Send MTreg and the current mode to the sensor
	//   High bit: 01000_MT[7,6,5]
	//    Low bit: 011_MT[4,3,2,1,0]
	I2C->start(BH1750_I2CADDR, 0);
	I2C->write((0b01000 << 3) | (MTreg >> 5));
	I2C->write((0b011 << 5) | (MTreg & 0b11111));
	I2C->write(BH1750_MODE);
	I2C->stop();

	// Wait a few moments to wake up
	_delay_ms(10);

	return true;
}

void readRegisterRegion(uint8_t address, uint8_t* outputPointer, uint8_t offset,
uint8_t length) {

	// request uint8_ts from slave device
	TinyI2C.start(address, length);
	uint8_t i = 0;
	char c = 0;
	while (i < length) // slave may send less than requested
	{
		c = TinyI2C.read(); // receive a uint8_t as character
		*outputPointer = c;
		outputPointer++;
		i++;
	}
	TinyI2C.stop();
}

/**
* Read light level from sensor
* The return value range differs if the MTreg value is changed. The global
* maximum value is noted in the square brackets.
* @return Light level in lux (0.0 ~ 54612,5 [117758,203])
* 	   -1 : no valid return value
* 	   -2 : sensor not configured
*/
float BH1750::readLightLevel() {

	if (BH1750_MODE == UNCONFIGURED) {
		// Serial.println(F("[BH1750] Device is not configured!"));
		return -2.0;
	}

	// Measurement result will be stored here
	float level = -1.0;

	// Read two uint8_ts from the sensor, which are low and high parts of the sensor
	// value
	uint8_t data[2];
	readRegisterRegion(BH1750_I2CADDR, data, 0, 2);
	uint16_t tmp = data[0];
	tmp <<= 8;
	tmp |= data[1];

	level = tmp;
	//lastReadTimestamp = millis();

	if (level != -1.0) {
		if (BH1750_MTreg != BH1750_DEFAULT_MTREG) {
			level *= (float)((uint8_t)BH1750_DEFAULT_MTREG / (float)BH1750_MTreg);
		}
		if (BH1750_MODE == BH1750::ONE_TIME_HIGH_RES_MODE_2 ||
		BH1750_MODE == BH1750::CONTINUOUS_HIGH_RES_MODE_2) {
			level /= 2;
		}
		// Convert raw value to lux
		level /= BH1750_CONV_FACTOR;
	}

	return level;
}
