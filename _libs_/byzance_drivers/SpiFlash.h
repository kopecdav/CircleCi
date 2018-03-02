#ifndef SPIFLASH_H
#define SPIFLASH_H
 
#include "mbed.h"
#include "ByzanceSpi.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"

/*
FLASH SIZE
*/
//#if SPIFLASH_NAME == N25Q64A
#define SPIFLASH_SIZE             (uint32_t)8388608
#define SPIFLASH_SECTORS          (uint32_t)128
#define SPIFLASH_SECTOR_SIZE      (uint32_t)65536
#define SPIFLASH_SUBSECTORS       (uint32_t)2048
#define SPIFLASH_SUBSECTOR_SIZE   (uint32_t)4096
#define SPIFLASH_PAGES            (uint32_t)32768
#define SPIFLASH_PAGE_SIZE        (uint32_t)256
#define SPIFLASH_OTP_BYTES        (uint32_t)64
//#endif

/*
COMPATIBLE COMMANDS
*/

// datasheet page 55/154, table 13
// http://docs-europe.electrocomponents.com/webdocs/0f7f/0900766b80f7f8aa.pdf

#define SPIFLASH_WRSR			0x01 // Write Status Register
#define SPIFLASH_PP				0x02 // Page Program
#define SPIFLASH_READ			0x03 // Read Data Bytes
#define SPIFLASH_WRDI			0x04 // Write Disable
#define SPIFLASH_RDSR			0x05 // Read Status Register
#define SPIFLASH_WREN			0x06 // Write Enable

#define SPIFLASH_FAST_READ		0x0b // Read Data Bytes at Higher Speed

#define SPIFLASH_QIEFP			0x12 // Quad Input Extended Fast Program

#define SPIFLASH_SSE			0x20 // Sub Sector Erase
#define SPIFLASH_BE				0xC7 // Bulk Erase
#define SPIFLASH_SE				0xD8 // Sector Erase
#define SPIFLASH_RDID			0x9F // Read Identification

// async transfer takes about 3kB of flash memory
// its disabled in bootloader
#if BYZANCE_BOOTLOADER
	#define SPIFLASH_ASYNC_READ		0
	#define SPIFLASH_ASYNC_WRITE	0

	#define SPI_LOCK
	#define SPI_UNLOCK
	#define RTOS_WAIT

#else
	#define SPIFLASH_ASYNC_READ		0
	#define SPIFLASH_ASYNC_WRITE	1

	#define SPI_LOCK	_flash_spi->_mutex.lock(osWaitForever)
	#define SPI_UNLOCK	_flash_spi->_mutex.unlock()
	#define RTOS_WAIT Thread::wait(1)

#endif


#define WRITE_ENABLE_PROTECTION		1
 
/**
* Class to write/read from SPIFLASH serial flash memory
*
* No write restrictions are implemented, the memory also should not
* have write restrictions enabled, this is factory default.
*/
class SpiFlash {

	public:

		/** Constructor
		*
		* @param mosi - Mosi pin to be used
		* @param miso - Miso pin to be used
		* @param sclk - Sclk pin to be used
		* @param cs - CS pin to be used
		*/
		SpiFlash(ByzanceSpi * byz_spi, PinName cs, uint32_t frequency);

		/** Checks if communication with the device functions
		*
		* @return none
		*/
		void get_jedec_id(char *manID, char *memType, char *memCap);

		/** Read a number of memory locations
		*
		* Take into account reading will stay within the page (256B) the address is in
		*
		* @param address - start address to read from
		* @param buffer - char array to read the data into
		* @param length - number of samples to read
		*/
		void read(int address, char *buffer, int length);

		/** Write a number of memory locations
		*
		* Take into account writing will stay within the page (256B) the address is in
		*
		* @param address - start address to write to
		* @param buffer - char array to read the data from
		* @param length - number of samples to write
		* @param block - true (default) to block until writing is finished
		*/
		void write(int address, char *buffer, int length, bool block = true);

		/** Erase a subsector (4KB)
		*
		* @param address - address in the sector that needs to be erased
		* @param block - true (default) to block until erasing is finished
		*/
		void erase_subsector(int address, bool block = true);

		/** Erase a sector (64KB)
		*
		* @param address - address in the block that needs to be erased
		* @param block - true (default) to block until erasing is finished
		*/
		void erase_sector(int address, bool block = true);

		/** Erase the entire chip
		*
		* @param block - true (default) to block until erasing is finished
		*/
		void erase_chip(bool block = true);

		/** Check if writing/erasing is being performed
		*
		* @return - true if busy, false if idle
		*/
		bool chip_is_busy( void );

		/** Enable/disable write
		*
		* @return - none
		*/
		void set_write_enable(bool enable);

	private:

		/** Determine status
		*
		* @return - none
		*/
		char _get_status(void);

		void _done(int event);

		volatile bool	_transaction_busy;

		ByzanceSpi* _flash_spi;
		DigitalOut	_flash_cs;
		DigitalOut*	_flash_wp;

		event_callback_t _spi_done_callback;
  
};

#endif /* SPIFLASH_H */
