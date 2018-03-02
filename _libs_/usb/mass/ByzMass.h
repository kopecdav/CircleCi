#ifndef MASS_H
#define MASS_H
 
#include "mbed.h"
#include "USBMSD.h"
 
class ByzMass : public USBMSD {
public:
 
    ByzMass(uint16_t vendor_id = 0x0703, uint16_t product_id = 0x0104, uint16_t product_release = 0x0001);

	virtual int disk_read(uint8_t* data, uint64_t block, uint8_t count);
	virtual int disk_write(const uint8_t* data, uint64_t block, uint8_t count);
    virtual int disk_initialize();
    virtual uint64_t disk_sectors();
    virtual uint64_t disk_size();
    virtual int disk_status();
 
protected:
 
    int _status;
};
 
#endif
