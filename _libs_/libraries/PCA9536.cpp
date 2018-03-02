/*
 * PCA9536.cpp
 *
 *  Created on: 18. 10. 2017
 *      Author: martin
 */
#include "PCA9536.h"

REGISTER_DEBUG(PCA9536);

PCA9536::PCA9536(PinName sda, PinName scl)  : _i2c(sda, scl) {

};

int PCA9536::set_all_outputs(){

	int rc = 0;

	rc = _write_reg(PCA9536_REG_CONFIGURATION, 0);
	if(rc){
		__ERROR("set all outputs failed\n");
	}

	return rc;
}

int PCA9536::set_all_inputs(){

	int rc = 0;

	rc = _write_reg(PCA9536_REG_CONFIGURATION, 0xff);
	if(rc){
		__ERROR("set_all_inputs failed\n");
	}

	return rc;
}

int PCA9536::set_value(uint8_t value){

	int rc = 0;

	rc = _write_reg(PCA9536_REG_OUTPUT, value&0xFF);
	if(rc){
		__ERROR("set value failed\n");
	}

	return rc;
}

int PCA9536::set_output(uint8_t pin){

	int		rc = 0;
	uint8_t	temp = 0;

	if(pin >= PCA9536_PINS){	//check if pin is valid
		return -2;
	}

	rc = _read_reg(PCA9536_REG_CONFIGURATION, &temp);	//read current value
	if(rc != 0){
		__ERROR("set output failed\n");
		return rc;
	}

	uint8_t mask=~(1<<pin);
	rc = _write_reg(PCA9536_REG_CONFIGURATION, temp&mask);	//set pin direction to 0
	if(rc){
		__ERROR("set output failed\n");
	}

	return rc;
}

int PCA9536::set_input(uint8_t pin){

	int		rc = 0;
	uint8_t	temp = 0;

	if(pin >= PCA9536_PINS){	//check if pin is valid
		return -2;
	}

	rc = _read_reg(PCA9536_REG_CONFIGURATION,&temp);	//read current value
	if(rc != 0){
		__ERROR("set input failed\n");
		return rc;
	}

	uint8_t mask=1<<pin;
	rc = _write_reg(PCA9536_REG_CONFIGURATION, temp|mask);	//set pin direction to 1
	if(rc){
		__ERROR("set input failed\n");
	}

	return rc;
}

int PCA9536::set_value(uint8_t pin, uint8_t value){

	int		rc = 0;
	uint8_t	temp = 0;

	if(pin >= PCA9536_PINS){	//check if pin is valid
		return -2;
	}

	rc = _read_reg(PCA9536_REG_OUTPUT, &temp);	//read current value
	if(rc){
		__ERROR("set value failed\n");
	}

	if(value){
		uint8_t mask=1<<pin;
		_write_reg(PCA9536_REG_OUTPUT, temp|mask);	//set pin to 1
	}else{
		uint8_t mask=~(1<<pin);
		_write_reg(PCA9536_REG_OUTPUT, temp&mask);	//set pin to 0
	}
	return rc;
}

int PCA9536::get_value(uint8_t *value){

	int rc = 0;

	rc = _read_reg(PCA9536_REG_INPUT, value);
	if(rc){
		__ERROR("get value failed\n");
	}

	return rc;
}

int PCA9536::get_value(uint8_t pin, uint8_t *value){

	int		rc = 0;
	uint8_t	val = 0;

	rc = _read_reg(PCA9536_REG_INPUT, &val);
	if(rc){
		__ERROR("read failed\n");
	}

	uint8_t mask=1<<pin;
	if(mask&val){
		*value=1;
	}else{
		*value=0;
	}

	return rc;
}

int PCA9536::_write_reg(uint8_t reg, uint8_t val){

	int rc = 0;

	uint8_t cmd[2];
	cmd[0]=reg;
	cmd[1]=val;

	rc = _i2c.write(PCA9536_ADDR, (char*)cmd, 2);
	if(rc){
		__ERROR("write reg failed\n");
	}

	return rc;
};

int PCA9536::_read_reg(uint8_t reg, uint8_t *val){

	int rc = 0;

	rc = _i2c.write(PCA9536_ADDR,(char*)&reg, 1, 1);
	if(rc!=0){
		_i2c.stop();
		__ERROR("rc1 failed\n");
	}

	rc = _i2c.read(PCA9536_ADDR, (char*)val, 1);
	if(rc!=0){
		__ERROR("rc2 failed\n");
	}

	return rc;
}

