#include "SpiFlash.h"

// TODO: add bootloader - interface bootloader_print
REGISTER_DEBUG(SpiFlash);

SpiFlash::SpiFlash(ByzanceSpi * byz_spi, PinName cs, uint32_t frequency) : _flash_cs(cs) {
	_flash_spi = byz_spi;
	/*
	 Mode	Pol		Phase
	 0		0		0
	 1		0		1
	 2		1		0
	 3		1		1
	 */
	_flash_spi->format(8, 0);

	_flash_cs = 1;
	_flash_spi->frequency(frequency);

	_transaction_busy = 0;
	_spi_done_callback = callback(this, &SpiFlash::_done);

	/*
	DigitalOut* _flash_wp = new DigitalOut(PF_10);

	*_flash_wp=1;

	// in all cases - unlock flash
	set_write_enable(false);
	*/

}

void SpiFlash::get_jedec_id(char *manID, char *memType, char *memCap) {

	SPI_LOCK;

	char b1, b2, b3;

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_RDID);
	b1 = _flash_spi->write(0x00);
	b2 = _flash_spi->write(0x00);
	b3 = _flash_spi->write(0x00);
	_flash_cs = 1;

	*manID = b1;
	*memType = b2;
	*memCap = b3;

	__TRACE("manid=0x%02X\n", b1);
	__TRACE("memtype=0x%02X\n", b2);
	__TRACE("memcap=0x%02X\n", b3);

	SPI_UNLOCK;
}

void SpiFlash::read(int address, char *buffer, int length) {

	__TRACE("read: start\n");

	SPI_LOCK;

	__TRACE("locked\n");

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_READ);
	_flash_spi->write(address >> 16);
	_flash_spi->write(address >> 8);
	_flash_spi->write(address);

	__TRACE("read: cmd sent\n");

#if SPIFLASH_ASYNC_READ
	_transaction_busy = 1;
	_flash_spi->transfer((char*)NULL, (int)0, (char*)buffer, (int)length, _spi_done_callback, SPI_EVENT_COMPLETE);

	while(_transaction_busy){
		RTOS_WAIT;
	}
#else

	// todo: stálo by za to přepsat pomocí asynchronního API MBED
	// připravena je na to funkce SpiFlash::_done a callback _spi_done_callback
	// ale asi je v mbedu bug a potřebujeme update na MBED OS 5.4

	for (int i = 0; i < length; i++) {
		buffer[i] = _flash_spi->write(0x00);
	}

#endif

	_flash_cs = 1;

	SPI_UNLOCK;

	__TRACE("read: done\n");
}

void SpiFlash::write(int address, char *buffer, int length, bool block) {

	SPI_LOCK;

#if WRITE_ENABLE_PROTECTION
	set_write_enable(true);
#endif

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_PP);
	_flash_spi->write(address >> 16);
	_flash_spi->write(address >> 8);
	_flash_spi->write(address);

#if SPIFLASH_ASYNC_WRITE
	// new asynchronous API
	_transaction_busy = 1;
	_flash_spi->transfer((char*)buffer, (int)length, (char*)NULL, (int)0, _spi_done_callback);

	while(_transaction_busy){
		RTOS_WAIT;
	}
#else

	// todo: stálo by za to přepsat pomocí asynchronního API MBED
	for (int i = 0; i < length; i++) {
		_flash_spi->write(buffer[i]);
	}

#endif


	_flash_cs = 1;

	// wait to finish chip write
	if (block) {
		while (chip_is_busy()){
			RTOS_WAIT;
		}
	}

#if WRITE_ENABLE_PROTECTION
	set_write_enable(false);
#endif

	SPI_UNLOCK;

}

void SpiFlash::erase_subsector(int address, bool block) {

	SPI_LOCK;

#if WRITE_ENABLE_PROTECTION
	set_write_enable(true);
#endif

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_SSE);
	_flash_spi->write(address >> 16);
	_flash_spi->write(address >> 8);
	_flash_spi->write(address);

	_flash_cs = 1;

	if (block) {
		while (chip_is_busy()){
			RTOS_WAIT;
		}
	}

#if WRITE_ENABLE_PROTECTION
	set_write_enable(false);
#endif

	SPI_UNLOCK;

}

void SpiFlash::erase_sector(int address, bool block) {

	SPI_LOCK;

#if WRITE_ENABLE_PROTECTION
	set_write_enable(true);
#endif

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_SE);
	_flash_spi->write(address >> 16);
	_flash_spi->write(address >> 8);
	_flash_spi->write(address);

	_flash_cs = 1;

	if (block){
		while (chip_is_busy()){
			RTOS_WAIT;
		}
	}

#if WRITE_ENABLE_PROTECTION
	set_write_enable(false);
#endif

	SPI_UNLOCK;

}

void SpiFlash::erase_chip(bool block) {

	SPI_LOCK;

#if WRITE_ENABLE_PROTECTION
	set_write_enable(true);
#endif

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_BE);
	_flash_cs = 1;

	// wait until chip finishes write
	if (block) {
		while (chip_is_busy()){
			RTOS_WAIT;
		}
	}

#if WRITE_ENABLE_PROTECTION
	set_write_enable(false);
#endif

	SPI_UNLOCK;

}

void SpiFlash::set_write_enable(bool enable) {

	SPI_LOCK;

	_flash_cs = 0;
	if (enable) {
		_flash_spi->write(SPIFLASH_WREN);
	} else {
		_flash_spi->write(SPIFLASH_WRDI);
	}
	_flash_cs = 1;

	SPI_UNLOCK;
}

bool SpiFlash::chip_is_busy(void) {
	char retval = _get_status();
	retval = retval & 0x01;
	return retval;
}


char SpiFlash::_get_status(void) {

	SPI_LOCK;

	_flash_cs = 0;
	_flash_spi->write(SPIFLASH_RDSR);

	char retval = _flash_spi->write(0);
	_flash_cs = 1;

	SPI_UNLOCK;
	return retval;
}

void SpiFlash::_done(int event){
	UNUSED(event);
	_transaction_busy = 0;
}
