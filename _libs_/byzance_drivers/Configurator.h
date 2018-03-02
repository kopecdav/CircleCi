#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include "mbed.h"

#include "ExtMem.h"
#include "struct_config_ioda.h"
#include "StructRoutines.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"

typedef enum : int {
	CONFIG_OK					= 0,
	CONFIG_ERROR_MEMORY 		= -1,
	CONFIG_ERROR_FAILED 		= -2,
	CONFIG_ERROR_BUSY			= -3,
	CONFIG_ERROR_UNINITIALISED  = -4,
} config_error_t;

#if defined(BYZANCE_BOOTLOADER)
extern void bootloader_print(const char* format, ...);
#endif

#define CONFIGURATOR_DEFAULT_TIMEOUT	300

#if BYZANCE_BOOTLOADER
	#define CONFIGURATOR_MUTEX_DEFINITION_H
	#define CONFIGURATOR_MUTEX_DEFINITION_C
	#define	CONFIGURATOR_MUTEX_LOCK
	#define	CONFIGURATOR_MUTEX_UNLOCK
	#define	CONFIGURATOR_UNUSED_MS	UNUSED(ms)
#else
	#define CONFIGURATOR_MUTEX_DEFINITION_H	static	Mutex _lock
	#define CONFIGURATOR_MUTEX_DEFINITION_C	Mutex	Configurator::_lock
	#define	CONFIGURATOR_MUTEX_LOCK \
		if(_lock.lock(ms)!=osOK){\
			__ERROR("config variable was not locked\n");\
			return CONFIG_ERROR_BUSY;\
		}
	#define	CONFIGURATOR_MUTEX_UNLOCK _lock.unlock()
	#define	CONFIGURATOR_UNUSED_MS

#endif

#define CONFIGURATOR_INIT_CHECK \
	if(!_initialised){\
		__ERROR("trying to get/set config but Configurator is not initiased yet\n");\
		return CONFIG_ERROR_UNINITIALISED;\
	}


class Configurator {

	public:

		static int init(ExtMem* extmem);

		/*
		 * return 0 - untouched
		 * return negative - error
		 * return positive - ok
		 */
		static int set_defaults(bool forced = 0);

		/*
		 * param getters
		 */
		static int get_flashflag(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_autobackup(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_wdenable(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_wdtime(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_blreport(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_netsource(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_configured(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_launched(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_alias(char* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_webview(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_webport(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_timeoffset(int* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_timesync(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_lowpanbr(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_restartbl(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int get_autojump(uint32_t* val, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);

		/*
		 * param setters
		 */
		static int set_flashflag(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_autobackup(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_wdenable(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_wdtime(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_blreport(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_netsource(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_configured(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_launched(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_alias(const char* val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_webview(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_webport(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_timeoffset(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_timesync(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_lowpanbr(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_restartbl(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);
		static int set_autojump(uint32_t val, bool save = true, uint32_t ms = CONFIGURATOR_DEFAULT_TIMEOUT);

	private:

		static	int					_save_config();

		static	ExtMem*				_extmem;

		static	struct_config_ioda	_config;

		static bool 				_initialised;

		CONFIGURATOR_MUTEX_DEFINITION_H;

		Configurator(){};
		~Configurator(){};
  
};

#endif /* CONFIGURATOR_H */
