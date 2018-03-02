#include "ByzMass.h"
 
ByzMass::ByzMass(uint16_t vendor_id, uint16_t product_id, uint16_t product_release): USBMSD(vendor_id, product_id, product_release) {

    //no init
    _status = 0x01;

	connect();
}
 
int ByzMass::disk_read(uint8_t* data, uint64_t block, uint8_t count){

	return 0;
}

int ByzMass::disk_write(const uint8_t* data, uint64_t block, uint8_t count){

	return 0;
}

int ByzMass::disk_initialize(){

    // OK
    _status = 0x00;

	return 0;
}

uint64_t ByzMass::disk_sectors(){
	return 32768;
}

uint64_t ByzMass::disk_size(){
	return 32768;
}

int ByzMass::disk_status(){
	return _status;
}
