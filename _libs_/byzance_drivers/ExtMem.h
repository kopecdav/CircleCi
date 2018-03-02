#ifndef EXTMEM_H
#define EXTMEM_H
 
#include "mbed.h"
#include "SpiFlash.h"
#include "extmem_config.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"

#if BYZANCE_BOOTLOADER
	#define EXTMEM_MUTEX_DEFINITION_H
	#define EXTMEM_MUTEX_DEFINITION_C
	#define	EXTMEM_MUTEX_LOCK
	#define	EXTMEM_MUTEX_UNLOCK
#else
	#define EXTMEM_MUTEX_DEFINITION_H	static Mutex _lock
	#define EXTMEM_MUTEX_DEFINITION_C	Mutex	ExtMem::_lock
	#define	EXTMEM_MUTEX_LOCK \
		if(_lock.lock(EXTMEM_MUTEX_TIMEOUT)!=osOK){\
			return EXTMEM_TIMEOUT;\
		}
	#define	EXTMEM_MUTEX_UNLOCK _lock.unlock()
#endif

/*
 * různé funkce vyžadují různý návratový kód
 * proto se dává do makra v argumentu
 */
#define EXTMEM_INIT_CHECK(RET) 	if(!_initialized){\
							__ERROR("extmem is not initialised\n");\
							return RET;\
							}\


typedef enum : unsigned char {
	EXTMEM_OK				= 0x00, // vše ok
	EXTMEM_ERASE_ERROR		= 0x02, // nepodarilo se mazani
	EXTMEM_WRITE_ERROR		= 0x03, // nepodaril se zapis
	EXTMEM_OVERLAP			= 0x04,	// zapisuju, kam nemam
	EXTMEM_MALLOC_ERROR		= 0x05, // malloc chcipnul
	EXTMEM_NOT_INITIALISED	= 0x06,
	EXTMEM_REINIT			= 0x07,
	EXTMEM_TIMEOUT			= 0x08,
	EXTMEM_ERROR			= 0xFF
} ExtMem_t;

struct struct_journal {
	uint32_t blocks;
	uint32_t dest_addr;
};

struct external_spiflash {
	uint32_t speed;
	uint32_t manID;
	uint32_t memType;
	uint32_t memCap;
	uint32_t size;
	uint32_t sectors;
	uint32_t sector_size;
	uint32_t subsectors;
	uint32_t subsector_size;
	uint32_t pages;
	uint32_t page_size;
	uint32_t otp_bytes;
	PinName spi_csn;
};

#define EXTMEM_MUTEX_TIMEOUT	1000

class ExtMem{

	public:

		/** Initialization of external memory
		 *
		 * @param settings
		 * @param byz_spi
		 * @return 0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t init(struct external_spiflash *settings, ByzanceSpi * byz_spi);

		/** This reads data from external memory
		 *
		 * @param	address
		 * @param	offset
		 * @param	data
		 * @param	size
		 * @return 0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t read_data(uint32_t address, uint32_t offset, void *data, uint32_t size);

		/** Write data to external memory
		 *
		 * @param address
		 * @param offset
		 * @param data
		 * @param size
		 * @param erase
		 * @return ==0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t write_data(uint32_t address, uint32_t offset, void *data, uint32_t size, bool erase = true);

		/** Read data from external memory
		 *
		 * @param  offset
		 * @param  data
		 * @param  size
		 * @return ==0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t secure_write(uint32_t address, uint32_t offset, void * data, uint32_t size);

		/** Erase the whole memory
		 *
		 * @param  none
		 * @return ==0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t erase(void);

		/** Erase subsectors from given address to given address
		 *
		 * @param  start_index
		 * @param  counter
		 * @return ==0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t erase_subsectors(uint32_t start_index, uint32_t counter);

		/** Erase sectors from given address to given address
		 *
		 * @param  start_index
		 * @param  counter
		 * @return ==0 - ok
		 * @return !=0 - error
		 */
		static ExtMem_t erase_sectors(uint32_t start_index, uint32_t counter);

		/** Calculate index of the flash subsector belonging to given address
		 *
		 * @param  address
		 * @return index or NULL if not initialized
		 */
		static uint32_t calculate_subsector_index(uint32_t address);

		/** Calculate index of the flash sector belonging to given address
		 *
		 * @param  address
		 * @return index or NULL if not initialized
		 */
		static uint32_t calculate_sector_index(uint32_t address);

		/** Calculate index of the flash page belonging to given address
		 *
		 * @param  address
		 * @return index or NULL if not initialized
		 */
		static uint32_t calculate_page_index(uint32_t address);

		/** Calculate begin address of the flash subsector belonging to given address
		 *
		 * @param  address
		 * @return subsector begin or NULL if not initialized
		 */
		static inline uint32_t calculate_subsector_begin(uint32_t address);

		/** Calculate begin address of the flash sector belonging to given address
		 *
		 * @param  address
		 * @return sector begin or NULL if not initialized
		 */
		static inline uint32_t calculate_sector_begin(uint32_t address);

		/** Calculate begin address of the flash page belonging to given address
		 *
		 * @param  address
		 * @return page begin or NULL if not initialized
		 */
		static inline uint32_t calculate_page_begin(uint32_t address);

		/** Determine subsector size of flash memory
		 *
		 * @param  none
		 * @return subsector size in bytes or NULL if not initialized
		 */
		static uint32_t get_subsector_size(void);

		/** Determine sector size of flash memory
		 *
		 * @param  none
		 * @return sector size in bytes or NULL if not initialized
		 */
		static uint32_t get_sector_size(void);

		/** Determine page size of flash memory
		 *
		 * @param  none
		 * @return page size in bytes or NULL if not initialized
		 */
		static uint32_t get_page_size(void);

		/** Check external memory
		 *
		 * @param  none
		 * @return ==0 - ok
		 * @return !=0 - error
		 */
		static bool check_ok(void);

	protected:

		static SpiFlash *_extflash;

		static external_spiflash *_spi_settings;

		static char *_subsector_buffer;

		// must be higher than sizeof(subsector)
		// so uint8_t is insufficient
		static uint16_t _secure_counter;
  
	private:

		static bool _initialized;

		EXTMEM_MUTEX_DEFINITION_H;

		/** This function handles pending operation in journal
         *
         * @param  journal	- journal struct loaded from extmem journal index
         * @return positive - journal handled successfully
         * @return zero		- nothing happened, no operation pending
         * @return negative	- error code
         */
		static int _handle_journal(struct_journal* journal);

		/** This function erases journal index struct
         *
         * @param  journal	- journal struct loaded from extmem journal index
         */
		static int _empty_journal(struct_journal* journal);
  
		// constructor and destructor is private, because this is static class
		ExtMem(){};
		~ExtMem(){};
};

#endif /* EXTMEM_H */
