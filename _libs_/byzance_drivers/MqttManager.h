#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include "mbed.h"

#include "ExtMem.h"

#include "struct_mqtt.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "StructRoutines.h"

typedef enum{
	MQTT_NORMAL,
	MQTT_BACKUP
} mqtt_type_t;

typedef enum : int {
	MQTTMGR_OK					= 0,
	MQTTMGR_ERROR_MEMORY 		= -1,
	MQTTMGR_ERROR_FAILED 		= -2,
	MQTTMGR_ERROR_BUSY			= -3,
	MQTTMGR_ERROR_UNINITIALISED = -4,
} mqttmgr_error_t;

#define MQTTMANAGER_DEFAULT_TIMEOUT	100

#if BYZANCE_BOOTLOADER
	#define MQTTMANAGER_MUTEX_DEFINITION_H
	#define MQTTMANAGER_MUTEX_DEFINITION_C
	#define	MQTTMANAGER_MUTEX_LOCK
	#define	MQTTMANAGER_MUTEX_UNLOCK
	#define	MQTTMANAGER_UNUSED_MS	UNUSED(ms)
#else
	#define MQTTMANAGER_MUTEX_DEFINITION_H	static Mutex _lock
	#define MQTTMANAGER_MUTEX_DEFINITION_C	Mutex	MqttManager::_lock
	#define	MQTTMANAGER_MUTEX_LOCK \
		if(_lock.lock(ms)!=osOK){\
			__ERROR(" mutex lock error\n");\
			return MQTTMGR_ERROR_BUSY;\
		}
	#define	MQTTMANAGER_MUTEX_UNLOCK _lock.unlock()
	#define	MQTTMANAGER_UNUSED_MS

#endif

#define MQTTMANAGER_INIT_CHECK 	if(!_initialised){\
	__ERROR(" trying to get/set value but mqttmanager is not initiased yet\n");\
	return MQTTMGR_ERROR_UNINITIALISED;\
	}\

#ifndef MQTTMANAGER_DEBUGGING
#define MQTTMANAGER_DEBUGGING_LEVEL				DEBUG_LEVEL_NONE
#endif

#if defined(BYZANCE_BOOTLOADER)
extern void bootloader_print(const char* format, ...);
#endif

class MqttManager {

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
		static int get_hostname(mqtt_type_t type, char* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int get_port(mqtt_type_t type, uint32_t* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int get_connection(mqtt_type_t type, mqtt_connection* connection, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);

		static int get_username(char* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int get_password(char* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int get_credentials(mqtt_credentials* credentials, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);


		/*
		 * param setters
		 */
		static int set_hostname(mqtt_type_t type, const char* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int set_port(mqtt_type_t type, const uint32_t val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int set_connection(mqtt_type_t type, mqtt_connection* connection, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);

		static int set_username(const char* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int set_password(const char* val, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);
		static int set_credentials(mqtt_credentials* credentials, uint32_t ms = MQTTMANAGER_DEFAULT_TIMEOUT);


	private:

		static int					_fix_connection(mqtt_type_t type, bool forced);
		static int 					_fix_credentials(bool forced);

		static	mqtt_connection*	_connection_ptr(mqtt_type_t type);
		static int					_save_connection(mqtt_type_t type);
		static int 					_save_credentials();

		static	ExtMem*				_extmem;

		static	mqtt_connection		_connection_normal;
		static	mqtt_connection		_connection_backup;
		static	mqtt_credentials	_credentials_broker;

		static bool 			_initialised;

		MQTTMANAGER_MUTEX_DEFINITION_H;

		MqttManager(){};
		~MqttManager(){};
  
};

#endif /* MQTTMANAGER_H */
