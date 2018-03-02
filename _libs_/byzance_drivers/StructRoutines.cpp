#include "StructRoutines.h"

bool struct_is_empty(void *data, uint32_t size){

	uint32_t i = 0;
	uint32_t x = 0;

	// how many bytes from struct are 0xFF
	for(i=0; i<size; i++){
		// increments x everytime byte of structure equals 0xFF (clean FLASH) or 0x00 (clean RAM)
		if((((uint8_t*)data)[i] == 0xFF) || (((uint8_t*)data)[i] == 0x00)){
			x++;
		}
	}

	if(x==size){
		return 1;
	}

	return 0;
}

// return how many bytes are different
uint32_t struct_compare(void *str1, void *str2, uint32_t size){

	uint32_t i = 0;
	uint32_t x = 0;

	for(i=0; i<size; i++){
		// increments x everytime struct has different byte
		if(((uint8_t*)str1)[i] != ((uint8_t*)str2)[i]){
			x++;
		}
	}

	return x;
}

void struct_clean(void *data, uint32_t size){
	for(uint32_t i=0; i<size; i++){
		((uint8_t*)data)[i] = 0x00;
	}
}


void defaults_struct_bin(struct_binary *bin){

	bin->size				= DEFAULTS_BIN_SIZE;
	bin->crc				= DEFAULTS_BIN_CRC;
	strcpy(bin->build_id, 	  DEFAULTS_BIN_BUILD_ID);
	strcpy(bin->version,      DEFAULTS_BIN_VERSION);
	bin->timestamp			= DEFAULTS_BIN_TIMESTAMP;
	strcpy(bin->usr_version,  DEFAULTS_BIN_USR_VERSION);
	strcpy(bin->usr_name, 	  DEFAULTS_BIN_USR_NAME);
	bin->state				= DEFAULTS_BIN_STATE;
}

void defaults_mqtt_connection(mqtt_connection *conn){
	strcpy(conn->hostname, DEFAULTS_MQTT_HOSTNAME);
	conn->port			 = DEFAULTS_MQTT_PORT;
}

void defaults_mqtt_credentials(mqtt_credentials *cred){
	strcpy(cred->username, DEFAULTS_MQTT_USERNAME);
	strcpy(cred->password, DEFAULTS_MQTT_PASSWORD);
}

void defaults_struct_config_ioda(struct_config_ioda *conf){
	conf->flashflag 			= DEFAULTS_CONF_FLASHFLAG;
	conf->autobackup			= DEFAULTS_CONF_AUTOBACKUP;
	conf->wdenable				= DEFAULTS_CONF_WATCHDOG_ENABLED;
	conf->wdtime				= DEFAULTS_CONF_WATCHDOG_TIME;
	conf->blreport				= DEFAULTS_CONF_BOOTLOADER_REPORT;
	conf->netsource				= DEFAULTS_CONF_NETSOURCE;
	conf->configured			= DEFAULTS_CONF_CONFIGURED;
	conf->launched				= DEFAULTS_CONF_LAUNCHED;
	strcpy(conf->alias, 		  DEFAULTS_CONF_ALIAS);
	conf->webview				= DEFAULTS_CONF_WEBVIEW;
	conf->webport				= DEFAULTS_CONF_WEBPORT;
	conf->timeoffset			= DEFAULTS_CONF_TIMEOFFSET;
	conf->timesync				= DEFAULTS_CONF_TIMESYNC;
	conf->lowpanbr				= DEFAULTS_CONF_LOWPANBR;
	conf->restartbl				= DEFAULTS_CONF_RESTARTBL;
	conf->autojump				= DEFAULTS_CONF_AUTOJUMP;
}

void defaults_struct_config_device(struct_config_device *conf){
	conf->flashflag 			= DEFAULTS_CONF_FLASHFLAG;
	conf->bootloader_report		= DEFAULTS_CONF_BOOTLOADER_REPORT;
	conf->configured			= DEFAULTS_CONF_CONFIGURED;
	conf->launched				= DEFAULTS_CONF_LAUNCHED;
	strcpy(conf->alias, 		  DEFAULTS_CONF_ALIAS);
}

/*
void defaults_struct_wifi_module(struct_wifi_module *wifi){
	wifi->firmware_name		= DEFAULTS_WIFI_VERSION_NAME;
	wifi->firmware_major 	= DEFAULTS_WIFI_VERSION_MAJOR;
	wifi->firmware_minor 	= DEFAULTS_WIFI_VERSION_MINOR;
	wifi->firmware_patch 	= DEFAULTS_WIFI_VERSION_PATCH;
	strcpy(wifi->mac, 		  DEFAULTS_WIFI_MAC);
	wifi->espid				= DEFAULTS_WIFI_ESPID;
	wifi->flashid			= DEFAULTS_WIFI_FLASHID;
	wifi->flashsize			= DEFAULTS_WIFI_FLASHSIZE;
	wifi->flashspeed		= DEFAULTS_WIFI_FLASHSPEED;
}
*/

/*
void defaults_struct_wifi_credentials(struct_wifi_credentials *wifi){
	strcpy(wifi->ssid, DEFAULTS_WIFI_CRED_SSID);
	strcpy(wifi->pass, DEFAULTS_WIFI_CRED_PASSWORD);
}
*/
