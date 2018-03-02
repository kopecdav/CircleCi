#ifndef _BYZANCE_LOGGER_H
#define _BYZANCE_LOGGER_H

#include "mbed.h"
#include <string>
#if BYZANCE_SUPPORTS_USB
#include "USBSerial.h"
#endif

enum DebugLevel_t {
    DEBUG_LEVEL_LOG = -1,
    DEBUG_LEVEL_NONE = 0,
    DEBUG_LEVEL_ERROR = 1,
    DEBUG_LEVEL_WARNING = 2,
    DEBUG_LEVEL_INFO = 3,
    DEBUG_LEVEL_DEBUG = 4,
    DEBUG_LEVEL_TRACE = 5
};

typedef enum{
	LOGGER_SOURCE_SERIAL	= 0,
	LOGGER_SOURCE_USB		= 1,
} LoggerSource_t;

#define LOGGER_BUFFER_SIZE	256

#define LOGGER_MUTEX_TIMEOUT 100

#if BYZANCE_BOOTLOADER
	#define LOGGER_MUTEX_DEFINITION_H
	#define LOGGER_MUTEX_DEFINITION_C
	#define	LOGGER_MUTEX_LOCK
	#define	LOGGER_MUTEX_UNLOCK
#else
	#define LOGGER_MUTEX_DEFINITION_H	static Mutex _lock
	#define LOGGER_MUTEX_DEFINITION_C	Mutex	ByzanceLogger::_lock
	#define	LOGGER_MUTEX_LOCK \
	if(_lock.lock(LOGGER_MUTEX_TIMEOUT)!=osOK){\
		return;\
	}

	#define	LOGGER_MUTEX_UNLOCK _lock.unlock()

#endif

#define LOGGER_INIT_CHECK \
	if(!_initialised){\
		return;\
	}

class ByzanceLogger {

public:

	static void init(Serial *serial);

#if BYZANCE_SUPPORTS_USB
	static void init(USBSerial *serial);
#endif

	static void set_baud(int baud);

	static void set_level(DebugLevel_t level);
	static void enable_prefix(bool prefix = true);

	static void log(const char* format, ...);
	static void new_log(DebugLevel_t lvl, const char* comp, const char* func, int line, const char* format, ...);
	static void prefix_str(DebugLevel_t lvl, char* prefix);

protected:

	static Serial		*_serial;

#if BYZANCE_SUPPORTS_USB
	static USBSerial	*_usb;
#endif

	static bool			_initialised;

	static LoggerSource_t _log_source;

	static DebugLevel_t _log_level;
	static bool _log_prefix;

	static char _log_buffer[LOGGER_BUFFER_SIZE];

	LOGGER_MUTEX_DEFINITION_H;

	static void write_to_itf(const char* data);

private:

    ByzanceLogger() {};
    ~ByzanceLogger() {};
};

#endif /* _BYZANCE_LOGGER_H */
