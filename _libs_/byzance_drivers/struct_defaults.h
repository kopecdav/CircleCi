#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "enum_netsource.h"

// obecna neinicializovana polozka
#define DEFAULTS_UNKNOWN_VALUE				0xFFFFFFFF

// MQTT defaults
#define DEFAULTS_MQTT_HOSTNAME				(const char*)"192.168.65.179"  	// user configurable
#define DEFAULTS_MQTT_PORT					(uint32_t)1881                  // user configurable
#define DEFAULTS_MQTT_USERNAME				(const char*)"user"            	// user configurable
#define DEFAULTS_MQTT_PASSWORD				(const char*)"pass"            	// user configurable

// configuration
#define DEFAULTS_CONF_FLASHFLAG				0                              	// managed by byzance
#define DEFAULTS_CONF_AUTOBACKUP			0                              	// user configurable
#define DEFAULTS_CONF_BOOTLOADER_REPORT		0                          		// user configurable
#define DEFAULTS_CONF_WATCHDOG_ENABLED		1                          		// user configurable
#define DEFAULTS_CONF_WATCHDOG_TIME			30                           	// user configurable
#define DEFAULTS_CONF_NETSOURCE				NETSOURCE_ETHERNET             	// user configurable
#define DEFAULTS_CONF_CONFIGURED			0                              	// user configurable
#define DEFAULTS_CONF_LAUNCHED				0                              	// managed by byzance
#define DEFAULTS_CONF_ALIAS					""                       		// user configurable
#define DEFAULTS_CONF_BACKUPTIME			60                             	// user configurable
#define DEFAULTS_CONF_WEBVIEW				1                               // user configurable
#define DEFAULTS_CONF_WEBPORT				80                              // user configurable
#define DEFAULTS_CONF_TIMEOFFSET			0                             	// user configurable
#define DEFAULTS_CONF_TIMESYNC				1                              	// user configurable
#define DEFAULTS_CONF_LOWPANBR				0                              	// user configurable
#define DEFAULTS_CONF_RESTARTBL				0                              	// user configurable
#define DEFAULTS_CONF_AUTOJUMP				300                            	// user configurable

// binary defaults
#define DEFAULTS_BIN_SIZE					0                               // managed by byzance
#define DEFAULTS_BIN_CRC					0                               // managed by byzance
#define DEFAULTS_BIN_BUILD_ID				""                       		// managed by byzance
#define DEFAULTS_BIN_VERSION				""                       		// managed by byzance
#define DEFAULTS_BIN_TIMESTAMP				0								// managed by byzance
#define DEFAULTS_BIN_USR_VERSION			""                       		// managed by byzance
#define DEFAULTS_BIN_USR_NAME				""                       		// managed by byzance
#define DEFAULTS_BIN_STATE					BINSTRUCT_STATE_INVALID         // managed by byzance
#endif /* DEFAULTS_H */
