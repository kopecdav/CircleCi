#ifndef INTMEM_H
#define INTMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intmem_config.h"
#include "memory_map.h"

#include "mbed.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

#include "stm32f4xx_hal.h"

typedef enum : unsigned char {
	INTMEM_OK				= 0x00, // ok
	INTMEM_LOCKED			= 0x01, // memory is locked
	INTMEM_ERASE_ERROR		= 0x02, // unsuccesfull erase
	INTMEM_WRITE_ERROR		= 0x03, // unsuccesfull write
	INTMEM_READ_ERROR		= 0x04, // unsuccesfull read
	INTMEM_BAD_ADDR			= 0x05,	// bad address to read or write
	INTMEM_DATA_BIG			= 0x06, // data are to big for write into given address
	INTMEM_ERROR			= 0xFF
} IntMem_t;

#define ADDR_FLASH_DEVICE_PAGE_SIZE  ((uint32_t)2048)
  
enum{
  FLASHIF_PROTECTION_NONE         = 0x0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED   = 0x2,
  FLASHIF_PROTECTION_RDPENABLED   = 0x4,
};

// Define bitmap representing user flash area that could be write protected (check restricted to pages 8-39)
#define FLASH_SECTOR_TO_BE_PROTECTED (OB_WRP_SECTOR_0 | OB_WRP_SECTOR_1 | OB_WRP_SECTOR_2 | OB_WRP_SECTOR_3 |\
                                      OB_WRP_SECTOR_4 | OB_WRP_SECTOR_5 | OB_WRP_SECTOR_6 | OB_WRP_SECTOR_7 )

class IntMem {

public:

	/** Initialize and unlock internal memory
	 *
	 * @param  none
	 * @return none
	 */
	static IntMem_t	init(void);

	/** Erase internal memory.
	 *
	 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 *    Be careful when using this function.
	 *    It doesnt't only erase given memory form start to end
	 *    but WHOLE PAGE where this address belongs.
	 *    It can cause unwanted data loss.
	 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 *
	 * @param	start 	begin address
	 * @param	end		end address
	 * @return	TODO: implement properly
	 */
	static IntMem_t erase(uint32_t StartAddress, uint32_t EndAddress);

	/** Read from internal memory
	 *
	 * @param	address	address to read
	 * @param	offset	offset from address to read
	 * @param	data	pointer to source data buffer
	 * @param	size	size of data to read
	 * @return	TODO:	implement properly
	 */
	static IntMem_t	read_data(uint32_t address, uint32_t offset, void *data, uint32_t size);
	
	/** Write to internal memory
	 *
	 * @param	address	address to write
	 * @param	offset	offset from address to write
	 * @param	data	pointer to result data buffer
	 * @param	size	size of data to write
	 * @param	erase	auto-erase memory before write
	 * 					leave blank or true if you are not sure
	 * 					or read erase warning
	 *
	 * @return	TODO:	implement properly
	 */
	static IntMem_t write_data(const uint32_t address, const uint32_t offset, const void *data, const uint32_t size, const bool erase = true);

protected:

	// todo: navratova hodnota
	static uint32_t get_sector(uint32_t address);

private:
    /*
     * Private constructor and destructor
     */
	IntMem() {};
	~IntMem() {};
};

#endif /* INTMEM_H */
