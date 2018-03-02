/*
 * I2CWrapper.h
 *
 *  Created on: 14. 12. 2017
 *      Author: martin
 */

#ifndef I2CWRAPPER_H_
#define I2CWRAPPER_H_
#include "mbed.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

class I2CWrapper:public I2C{
public:
	I2CWrapper(PinName sda, PinName scl,uint8_t address);
	int write_reg(uint8_t reg, uint8_t val);
	int write_reg(uint8_t reg, uint8_t *val, uint16_t size);
	int read_reg(uint8_t reg, uint8_t *val);
	int read_reg(uint8_t reg, uint8_t *val, uint16_t size);
	int write_bit(uint8_t reg, uint8_t bit, bool val);
	int read_bit(uint8_t reg, uint8_t bit, bool *val);
	int search();
protected:
	uint8_t _address;
};



#endif /* I2CWRAPPER_H_ */
