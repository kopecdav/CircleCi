#include "Flash.h"

ExtMem* Flash::_extmem;
bool	Flash::_initialised = false;

REGISTER_DEBUG(flash);

void Flash::init(ExtMem* extmem){
	_extmem			= extmem;
	_initialised	= true;
}

int Flash::read(uint32_t offset, void *data, uint32_t size){

	if(!_initialised){
		__ERROR("read: extmem not initialised\n");
		return -1;
	}

	// begin of read check
	if(!addr_ok(offset)){
		__ERROR("read: begin of read outside of user area\n");
		return -2;
	}

	// end of read check
	if(!addr_ok(offset+size)){
		__ERROR("read: end of read outside of user area\n");
		return -3;
	}

	uint32_t rc = 0;

	rc = _extmem->read_data(EXTPART_USER, offset, data, size);
	if(rc){
		__ERROR("read: read %d bytes from %d failed (code=%d)\n", size, offset, rc);
		return -4;
	} else {
		__INFO("flash: read: read %d bytes from %d OK\n", size, offset, rc);
		return size;
	}

}

int Flash::write(uint32_t offset, void *data, uint32_t size){

	if(!_initialised){
		__ERROR("write: extmem not initialised\n");
		return -1;
	}

	// begin of write check
	if(!addr_ok(offset)){
		__ERROR("write: begin of write outside of user area\n");
		return -2;
	}

	// end of write check
	if(!addr_ok(offset+size)){
		__ERROR("write: end of write outside of user area\n");
		return -3;
	}

	uint32_t rc = 0;

	rc = _extmem->write_data(EXTPART_USER, offset, data, size);
	if(rc){
		__ERROR("write: write %d bytes to %d failed (code=%d)\n", size, offset, rc);
		return -4;
	} else {
		__INFO("flash: write: write %d bytes to %d OK\n", size, offset, rc);
		return size;
	}
}

int Flash::size(void){
	__INFO("flash: size: sizeof user extmem = %d\n", EXTMEM_SIZEOF_USER);
	return EXTMEM_SIZEOF_USER;
}

bool Flash::ready(void){
	if(_initialised){
		__INFO("flash: ready: true\n");
		return true;
	}

	__INFO("flash: ready: false\n");
	return false;
}

bool Flash::addr_ok(uint32_t offset){

	// probably not necessarry because uint32_t can't be lower than 0
	if(offset<0){
		__INFO("flash: addr_ok: underflow\n");
		return false;
	}

	if(offset>EXTMEM_SIZEOF_USER){
		__INFO("flash: addr_ok: overflow\n");
		return false;
	}

	__INFO("flash: addr_ok: ok\n");
	return true;
}
