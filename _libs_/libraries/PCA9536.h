/*
 * PCA9536.h
 *
 *  Created on: 18. 10. 2017
 *      Author: martin
 */

#ifndef PCA9536_H_
#define PCA9536_H_

#include "mbed.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

#define PCA9536_ADDR 				0x82

#define PCA9536_PINS 				4

#define PCA9536_REG_INPUT 			0x00
#define PCA9536_REG_OUTPUT 			0x01
#define PCA9536_REG_POLARITY 		0x02
#define PCA9536_REG_CONFIGURATION 	0x03

class PCA9536{
	public:
		PCA9536(PinName sda, PinName scl);

		/*
		 * all funtions has the same return codes
		 *
		 *  0 		OK
		 * -2 		invalid pin
		 * other 	inherted from I2C library
		 */

		//set port as input
		int set_all_inputs();

		//set port as output
		int set_all_outputs();

		//set pin as output
		int set_output(uint8_t pin);

		//set pin as input
		int set_input(uint8_t pin);

		//set port value
		int set_value(uint8_t value);

		//set pin value
		int set_value(uint8_t pin, uint8_t value);

		//get port value
		int get_value(uint8_t *value);

		//get pin value
		int get_value(uint8_t pin, uint8_t *value);
	protected:
	    I2C     _i2c;

	    //write val to reg
	    int _write_reg(uint8_t reg, uint8_t val);

	    //read val from reg
	    int _read_reg(uint8_t reg, uint8_t *val);
};


#endif /* PCA9536_H_ */
