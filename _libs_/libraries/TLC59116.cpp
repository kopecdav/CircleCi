/*
 * TLC59116.cpp
 *
 *  Created on: 18. 10. 2017
 *      Author: Viktor
 */

#include "TLC59116.h"

REGISTER_DEBUG(TLC59116);

TLC59116::TLC59116(void){
	_addr = TLC59116_DEFAULT_ADDRESS;
	_i2c = NULL;
	_initialized = false;

	__TRACE("_addr: 0x%02X (%d)\n", _addr, _addr);
}



uint8_t TLC59116::initialize(PinName sda, PinName scl) {
	int rc = 0;
	char reg[7];

	if(_initialized){
		return 1;
	}

	_i2c = new I2C(sda, scl);

	if(_i2c == NULL){
		__ERROR("I2C constructor failed:\n");
		return 2;
	}
	_i2c->frequency(400000);

	_initialized = true;

	Thread::wait(100);

	// Set driver to default state.
	if(reset_sw()){
		return 3;
	}
	Thread::wait(50);

	// Set basic control registers
	reg[0] = 0b10000000 + TLC59116_MODE1;	// Control Register + Starting data register
	reg[1] = 0b1111;						// MODE1 Register
	reg[2] = 0b10000000;					// MODE2 Register

	rc = _i2c->write(_addr, (const char*)reg, 3, false);
	if(rc){
		__ERROR("_write_bytes: write failed (%d)\n", rc);
		return 1;
	}

	// Config group dimming/blinking and drivers output
	reg[0] = 0b10000000 + TLC59116_GRPPWM;	// Control Register + Starting data register

	reg[1] = 0xFF;	// GRPPWM
	reg[2] = 0x10; 	// GRPFREQ
	reg[3] = 0b11111111;	// LED driver x individual brightness and group dimming/blinking
	reg[4] = 0b11111111;	// LED driver x individual brightness and group dimming/blinking
	reg[5] = 0b11111111;	// LED driver x individual brightness and group dimming/blinking
	reg[6] = 0b11111111;	// LED driver x individual brightness and group dimming/blinking

	rc = _i2c->write(_addr, (const char*)reg, 7, false);
	if(rc){
		__ERROR("_write_bytes: write failed (%d)\n", rc);
		return 1;
	}

	__TRACE("initialize ok\n");
	return 0;
}


uint8_t TLC59116::reset_sw(void){
	int rc = 0;
	char reg[2] = {0xA5, 0x5A};	// Two specific values wire on address TLC59116_SW_RESET_ADDRESS

	__TRACE("reset_sw entered\n");

	// Is driver already initialized?
	if(!_initialized){
		return 1;
	}


	// The Software Reset Call
	rc = _i2c->write(TLC59116_RESET, (const char*)reg, 2, false);
	if(rc){
		__ERROR("_write_bytes: write failed (%d)\n", rc);
		return 1;
	}

	__TRACE("TLC59116: reset_sw ok\n");
	return 0;
}


uint8_t TLC59116::set_register(uint8_t address, uint8_t value){
	int rc = 0;
	char reg[2];

	__TRACE("set_register entered (%d, %d)\n", address, value);

	// Is driver already initialized?
	if(!_initialized){
		return 1;
	}

	// Set PWM value registers
	reg[0] = 0b10000000 + address;	// Control Register + Starting data register
	reg[1] = value;					// Value


	rc = _i2c->write(_addr, (const char*)reg, 2, false);
	if(rc){
		__ERROR("set_register failed (%d)\n", rc);
		return 2;
	}

	__TRACE("set_register ok\n");
	return 0;
}




uint8_t TLC59116::set_channel(uint8_t channel, uint8_t pwm){
	__TRACE("set_channel entered (%d, %d)\n", channel, pwm);

	// Check channel validity
	if(channel > 16){
		return 1;
	}

	// Set PWM value registers
	if(set_register(TLC59116_PWM0 + channel, pwm)){
		return 2;
	}
	__TRACE("set_channel ok\n");
	return 0;
}


uint8_t TLC59116::set_all_channels(uint8_t * data){
	int rc = 0;
	char reg[17];

	__TRACE("set_all_channels entered\n");

	// Check data validity
	if(data == NULL){
		return 1;
	}

	// Is driver already initialized?
	if(!_initialized){
		return 1;
	}

	// Set PWM value registers
	reg[0] = 0b10000000 + TLC59116_PWM0;	// Control Register + Starting data register
	for(uint8_t i = 0; i < 16; i++){
		reg[i + 1] = data[i];				// Value for each channel
	}

	rc = _i2c->write(_addr, (const char*)reg, 17, false);
	if(rc){
		__ERROR("set_all_channels failed (%d)\n", rc);
		return 2;
	}

	__TRACE("set_all_channels ok\n");
	return 0;
}

uint8_t TLC59116::set_global_pwm(uint8_t pwm){
	__TRACE("set_global_pwm entered (%d)\n", pwm);

	// Set PWM value registers
	if(set_register(TLC59116_GRPPWM, pwm)){
		return 1;
	}
	__TRACE("set_global_pwm ok\n");
	return 0;
}





