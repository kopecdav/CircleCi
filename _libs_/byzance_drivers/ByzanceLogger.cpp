#include "ByzanceLogger.h"

Serial		*ByzanceLogger::_serial;

#if BYZANCE_SUPPORTS_USB
USBSerial	*ByzanceLogger::_usb;
#endif

bool ByzanceLogger::_initialised;

LoggerSource_t ByzanceLogger::_log_source;

DebugLevel_t ByzanceLogger::_log_level = DEBUG_LEVEL_NONE;

// prefix default: enable
bool ByzanceLogger::_log_prefix = true;

char ByzanceLogger::_log_buffer[LOGGER_BUFFER_SIZE];

LOGGER_MUTEX_DEFINITION_C;

void ByzanceLogger::init(Serial *serial){
	ByzanceLogger::_serial = serial;
	_log_source = LOGGER_SOURCE_SERIAL;
	_initialised = true;
}

#if BYZANCE_SUPPORTS_USB
void ByzanceLogger::init(USBSerial *serial){
	ByzanceLogger::_usb = serial;
	_log_source = LOGGER_SOURCE_USB;
	_initialised = true;
}
#endif

void ByzanceLogger::set_level(DebugLevel_t level){
	_log_level = level;
}

void ByzanceLogger::enable_prefix(bool prefix){
	_log_prefix = prefix;
}

void ByzanceLogger::log(const char* format, ...){

	LOGGER_INIT_CHECK;

	LOGGER_MUTEX_LOCK;

	/*
	 * serialize text
	 */
	va_list arg;
	va_start (arg, format);
	vsnprintf(_log_buffer, LOGGER_BUFFER_SIZE, format, arg);
	va_end (arg);

	switch(_log_source){
		case LOGGER_SOURCE_SERIAL:
			ByzanceLogger::_serial->printf(_log_buffer);

			// wait until written
			while(!_serial->writeable());

			break;
	#if BYZANCE_SUPPORTS_USB
		case LOGGER_SOURCE_USB:

			ByzanceLogger::_usb->printf(_log_buffer);

			// wait until written
			while(!_usb->writeable());

			break;
	#endif
	}

	LOGGER_MUTEX_UNLOCK;
}

void ByzanceLogger::new_log(DebugLevel_t lvl, const char* comp, const char* func, int line, const char* format, ...){

	LOGGER_INIT_CHECK;

	LOGGER_MUTEX_LOCK;

	// log level is lower than requested
	if(_log_level<lvl){
		LOGGER_MUTEX_UNLOCK;
		return;
	}

	// todo: log prefix

	/*
	 * solve component name, function, line
	 */
	switch(_log_source){
	case LOGGER_SOURCE_SERIAL:
		ByzanceLogger::_serial->printf("%s: %s, %d: ", comp, func, line);
		// wait until written
		while(!_serial->writeable());
		break;

	#if BYZANCE_SUPPORTS_USB
	case LOGGER_SOURCE_USB:
		ByzanceLogger::_usb->printf("%s: %s, %d: ", comp, func, line);
		// wait until written
		while(!_usb->writeable());
		break;
	#endif
	}

	/*
	 * serialize text
	 */
	va_list arg;
	va_start (arg, format);
	vsnprintf(_log_buffer, LOGGER_BUFFER_SIZE, format, arg);
	va_end (arg);

	switch(_log_source){
		case LOGGER_SOURCE_SERIAL:
			ByzanceLogger::_serial->printf(_log_buffer);

			// wait until written
			while(!_serial->writeable());

			break;
	#if BYZANCE_SUPPORTS_USB
		case LOGGER_SOURCE_USB:

			ByzanceLogger::_usb->printf(_log_buffer);

			// wait until written
			while(!_usb->writeable());

			break;
	#endif
	}

	LOGGER_MUTEX_UNLOCK;
}

void ByzanceLogger::prefix_str(DebugLevel_t lvl, char* prefix){
	if(_log_prefix){
		switch(lvl){
		case DEBUG_LEVEL_LOG:
			strcpy(prefix, "[LOG]");
			break;
		case DEBUG_LEVEL_ERROR:
			strcpy(prefix, "[ERR]");
			break;
		case DEBUG_LEVEL_WARNING:
			strcpy(prefix, "[WRN]");
			break;
		case DEBUG_LEVEL_INFO:
			strcpy(prefix, "[INF]");
			break;
		case DEBUG_LEVEL_DEBUG:
			strcpy(prefix, "[DBG]");
			break;
		case DEBUG_LEVEL_TRACE:
			strcpy(prefix, "[TRC]");
			break;
		default:
			strcpy(prefix, "[???]");
		}
	}

	strcpy(prefix, "");
}

