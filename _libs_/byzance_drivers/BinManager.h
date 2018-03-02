#ifndef BINMANAGER_H
#define BINMANAGER_H

#include "mbed.h"

#include "ExtMem.h"
#include "enum_binstate.h"
#include "struct_binary.h"

#include "IntMem.h"
#include "intmem_config.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "StructRoutines.h"
#include "enum_component.h"

#define BIN_UPLOAD_TIMEOUT	60 // seconds

/**
 * \struct bin_wrapper
 * \brief brief TO DO.
 * 
 *  
 * support structure which unifies RAM only 'progress' and 'busy' fields
 * and struct_binary which is in both RAM and FLASH
 */
struct bin_wrapper {
	struct_binary	bin;
	uint8_t			progress;
	bool			busy;
};

typedef enum : int {
	BINMGR_ERASED				= 4,
	BINMGR_FIXED				= 3,
	BINMGR_SKIPPED				= 2,
	BINMGR_BACKUP_STARTED		= 1,
	BINMGR_OK					= 0,
	BINMGR_ERROR_MEMORY 		= -1,
	BINMGR_ERROR_FAILED 		= -2,
	BINMGR_ERROR_BUSY			= -3,
	BINMGR_ERROR_UNINITIALIZED	= -4,
	BINMGR_ERROR_SIZE			= -5,
	BINMGR_ERROR_INVALID		= -6,
} binmgr_error_t;

typedef enum{
	MODE_FLASH		= 0x00, // flashuju z bufferu do MCU
	MODE_RESTORE	= 0x01, // flashuju z backupu do MCU
} FlashMode_t;

#ifndef BINMANAGER_DEBUGGING
#define BINMANAGER_DEBUGGING_LEVEL				DEBUG_LEVEL_NONE
#endif

#if defined(BYZANCE_BOOTLOADER)
extern void bootloader_print(const char* format, ...);
#endif

#define BINMANAGER_DEFAULT_TIMEOUT	100

#if BYZANCE_BOOTLOADER
	#define BINMANAGER_MUTEX_DEFINITION_H
	#define BINMANAGER_MUTEX_DEFINITION_C
	#define	BINMANAGER_MUTEX_LOCK
	#define	BINMANAGER_MUTEX_UNLOCK
	#define	BINMANAGER_UNUSED_MS	UNUSED(ms)
#else
	#define BINMANAGER_MUTEX_DEFINITION_H	static Mutex _lock
	#define BINMANAGER_MUTEX_DEFINITION_C	Mutex  BinManager::_lock
	#define	BINMANAGER_MUTEX_LOCK \
		if(_lock.lock(ms)!=osOK){\
			__ERROR("mutex lock error\n");\
			return BINMGR_ERROR_BUSY;\
		}
	#define	BINMANAGER_MUTEX_UNLOCK _lock.unlock()
	#define	BINMANAGER_UNUSED_MS
#endif

#define BINMANAGER_INIT_CHECK if(!_initialised){\
		__ERROR("trying to get/set config but BinManager is not initiased yet\n");\
		return BINMGR_ERROR_UNINITIALIZED;\
	}\

/** 
* \class BinManager 
* \brief brief TO DO 
*
* Long description TO DO 
*/
class BinManager {

	public:

		/**
		*
		* \param
		* \return
		*/ 
		static int init(ExtMem* extmem, IntMem* intmem);

		/**
		 * \param
		 * \return 0 - untouched
		 * \return negative - error
		 * \return positive - ok
		 */
		static int _fix_component(bin_component_t comp, bool forced = 0);

		/**
		 * \return 0 - untouched
		 * \return negative - error
		 * \return positive - ok
		 */
		static int set_defaults(bool forced = 0);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int get_size(bin_component_t comp, uint32_t* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int get_crc(bin_component_t comp, uint32_t* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		*
		* \param
		* \return
		*/ 
		static int get_build_id(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int get_version(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int get_timestamp(bin_component_t comp, uint32_t* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		*
		* \param
		* \return
		*/
		static int get_usr_version(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		*
		* \param
		* \return
		*/
		static int get_usr_name(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int get_state(bin_component_t comp, bin_state_t* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int set_size(bin_component_t comp, uint32_t val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int set_crc(bin_component_t comp, uint32_t val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		 *
		 * \param
		 * \return
		 */
		static int set_build_id(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/** param setters
		 *
		 * \param
		 * \return
		 */
		static int set_version(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		*
		* \param
		* \return
		*/ 
		static int set_timestamp(bin_component_t comp, uint32_t val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		 *
		 * \param
		 * \return
		 */
		static int set_usr_version(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		*
		* \param
		* \return
		*/ 
		static int set_usr_name(bin_component_t comp, char* val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);
		
		/**
		*
		* \param
		* \return
		*/ 
		static int set_state(bin_component_t comp, bin_state_t val, uint32_t ms = BINMANAGER_DEFAULT_TIMEOUT);

		/**
		*
		* \param
		* \return
		*/ 
		static bool is_busy(bin_component_t comp);

		/**
		*
		* \param
		* \return
		*/ 
		static bool is_busy();

		/**
		*
		* \param
		* \return
		*/ 
		static bool set_busy(bin_component_t comp, bool busy);

		/**
		* percentage of progress 
		* \param
		* \return
		*/ 
		static bool get_progress(bin_component_t comp, uint8_t* val);

		 /**
		* percentage of progress of specifed component
		* \param
		* \return
		*/ 
		static bool	set_progress(bin_component_t comp, uint8_t val);

		
		/**
		* get the whole binary structure info 
		* \param
		* \return
		*/ 
		static bool	get_bin_info(bin_component_t comp, struct_binary* bin);

		/**
		* 
		* \param
		* \return
		*/
		static void	show_binary(bin_component_t comp);

		/**
		* 
		* \param
		* \return
		*/
		static void	show_binary(struct_binary* bin);

		static int upload_start(void);
		static int upload_stop(bool forced = 1);
		static int upload_reset(void);

		static bool upload_running(void);

		/**
		* 
		* \param
		* \return
		*/
		static int	yield();

		/**
		* 
		* \param
		* \return
		*/
		static int	restore_processes();

		/**
		* 
		* \param
		* \return
		*/
		static int	fix_build_id(const char* build_id);

		/**
		* 
		* \param
		* \return
		*/
		static int	fix_bootloader(const char* version);

		 
		/**
		* 
		* \param forced - ignore build_id check; take into account only validity of backup
		* \return
		*/
		static int	backup_start(bool check_build_id = false);

		/**
		* 
		* \param
		* \return
		*/
		static int	backup_stop();

		/**
		* 
		* \param
		* \return
		*/
		static int	buffer_check(struct_binary* bin);

		
		/**
		* erase function included inside
		* \param
		* \return
		*/
		static int	buffer_prepare(struct_binary* bin);

		/**
		* 
		* \param
		* \return
		*/
		static void buffer_erase(uint32_t size);

		/**
		*
		* \param
		* \return
		*/
		static int write_bin_part(char* data, uint32_t size);

		/**
		* 
		* \param bootloader, backup
		* \return
		*/
		static int	update_component(bin_component_t comp);

		/**
		* 
		* \param
		* \return
		*/
		static int	flash_firmware(FlashMode_t mode);

		/**
		*
		* \param
		* \return
		*/
	    static void name_component(bin_component_t comp, char* name);

		/**
		*
		* \param
		* \return
		*/
	    static void	name_state(bin_state_t state, char* name);

		/** TODO add brief
	     *
	     * TODO add long description
	     *
	     * @param bool
	     *
	     */
		static void attach_state_changed(void (*function)(bin_component_t comp, bin_state_t state));

		/** TODO add brief
	     *
	     * TODO add long description
	     *
	     * @param bool
	     *
	     */
	    template<typename T>
		static void attach_state_changed(T *object, void (T::*member)(bin_component_t comp, bin_state_t state)) {
			_state_changed.attach(object, member);
		}

		/** TODO add brief
	     *
	     * TODO add long description
	     *
	     * @param bool
	     *
	     */
		static void attach_led_blink(void (*function)(void));

		/** TODO add brief
	     *
	     * TODO add long description
	     *
	     * @param bool
	     *
	     */
	    template<typename T>
		static void attach_led_blink(T *object, void (T::*member)(void)) {
			_led_blink.attach(object, member);
		}

	private:

		static	ExtMem*			_extmem;
		static	IntMem*			_intmem;

		static	bin_wrapper		_bin_buffer;
		static	bin_wrapper		_bin_backup;
		static	bin_wrapper		_bin_firmware;
		static	bin_wrapper		_bin_bootloader;

		//static	bin_state		_state;

	    // used for backup procedure
	    static	uint32_t		_backup_index;  	// current part
	    static	uint32_t		_backup_counter;	// number of all parts

		// variables used for long-term erase
		static uint32_t			_erase_index;
		static uint32_t			_erase_counter;

		static bool 			_initialised;

		static uint32_t			_bin_size_written;

		// state changed callback
		static Callback<void(bin_component_t comp, bin_state_t state)>	_state_changed;

		// led blink callback
		static Callback<void(void)>	_led_blink;

		// upload end ticker
		static Ticker _ticker;

		static bool	_upload_running;
		static int	_upload_time;

		BINMANAGER_MUTEX_DEFINITION_H;

		/**
		* 
		* \param
		* \return
		*/
		static int	 			_backup_prepare();

		/**
		* 
		* \param
		* \return
		*/
	    static int	 			_backup_function();

		/**
		* 
		* \param
		* \return
		*/
	    static int	 			_erase_function();

	    /**
		 * @param
	     * @return 0 - ok
	     * @return number - error
	     */
	    static int 				_save_binary(bin_component_t comp);

		/**
		* 
		* \param
		* \return
		*/
	    static void				_copy_binary(bin_component_t dest, bin_component_t src, bool copy_state = 1);

		/**
		* 
		* \param
		* \return
		*/
	    static void 			_copy_binary(bin_component_t dest, struct_binary* src, bool copy_state = 1);


	    static void 			_tick_callback(void);

		/**
		* 
		* \param
		* \return
		*/
	    static bin_wrapper*		_component_ptr(bin_component_t comp);

		BinManager(){};
		~BinManager(){};
  
};

#endif /* BINMANAGER_H */
