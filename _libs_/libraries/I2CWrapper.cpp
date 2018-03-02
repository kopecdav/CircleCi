/*
 * I2CWrapper.cpp
 *
 *  Created on: 14. 12. 2017
 *      Author: martin
 */
#include "I2CWrapper.h"


REGISTER_DEBUG(I2CWrapper);

I2CWrapper::I2CWrapper(PinName sda, PinName scl,uint8_t address):I2C(sda,scl){
	_address = address;
}

int I2CWrapper::write_reg(uint8_t reg, uint8_t val){
	uint8_t cmd[2];
	cmd[0]=reg;
	cmd[1]=val;
	int rc = I2C::write(_address,(char*)cmd,2);
	if(rc){
		__ERROR("instance %p: I2C write error %d\n",this,rc);
	}
	return rc;
};

int I2CWrapper::write_reg(uint8_t reg, uint8_t *val, uint16_t size){
	int rc = I2C::write(_address,(char*)&reg,1,1);	//write first byte - addr
	if(rc){
		__ERROR("instance %p: I2C write error %d\n",this,rc);
		return rc;
	}
	rc = I2C::write(_address,(char*)val,size);
	if(rc){
		__ERROR("instance %p: I2C write error %d\n",this,rc);
	}
	return rc;
};

int I2CWrapper::read_reg(uint8_t reg, uint8_t *val){
	int rc = I2C::write(_address,(char*)&reg,1,1);
	if(rc){
		__ERROR("instance %p: I2C write error %d\n",this,rc);
		return rc;
	}
	rc = I2C::read(_address,(char*)val,1);
	if(rc){
		__ERROR("instance %p: I2C read error %d\n",this,rc);
	}
	return rc;
}

int I2CWrapper::read_reg(uint8_t reg, uint8_t *val, uint16_t size){
	int rc = I2C::write(_address,(char*)&reg,1,1);
	if(rc){
		__ERROR("instance %p: I2C write error %d\n",this,rc);
		return rc;
	}
	rc = I2C::read(_address,(char*)val,size);
	if(rc){
		__ERROR("instance %p: I2C read error %d\n",this,rc);
	}
	return rc;
}

int I2CWrapper::write_bit(uint8_t reg, uint8_t bit, bool val){
	uint8_t temp;
	int rc=read_reg(reg,&temp);
	if(rc==0){
		return rc;
	}
	if(val){	//set bit
		rc = write_reg(reg,temp|(1<<bit));
	}else{
		rc = write_reg(reg,temp&(~(1<<bit)));
	}
	return rc;
}

int I2CWrapper::read_bit(uint8_t reg, uint8_t bit, bool *val){
	uint8_t temp;
	int rc = read_reg(reg,&temp);
	if(temp&(1<<bit)){
		*val=true;
	}else{
		*val=false;
	}
	return rc;
}

int I2CWrapper::search(){
	uint8_t temp=_address,sink;
	for(uint8_t i=0; i<128; i++){
		_address = i<<1;
		int rc = read_reg(0,&sink);
		if(rc==0){
			__LOG("found device address %x with reg 0 value %x\n",_address,sink);
		}else{
			__LOG("device with addr %x is no present\n",_address);
		}
	}
	_address=temp;
	return 0;
}
