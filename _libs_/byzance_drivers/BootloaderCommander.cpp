#if BYZANCE_BOOTLOADER

#include "BootloaderCommander.h"

size_t				BootloaderCommander::_cmdcount;
ExtMem*				BootloaderCommander::_extmem;
IntMem*				BootloaderCommander::_intmem;

Callback<void(void)>	BootloaderCommander::_led_blink;

REGISTER_DEBUG(Commander);

BootloaderCommander::BootloaderCommander(){

	// https://stackoverflow.com/questions/1898657/result-of-sizeof-on-array-of-structs-in-c
	_cmdcount = sizeof commands / sizeof commands[0];
}

void BootloaderCommander::init(ExtMem *extmem, IntMem *intmem){
	_extmem = extmem;
	_intmem = intmem;
}

size_t BootloaderCommander::size(void){
	return _cmdcount;
};

int	BootloaderCommander::parse(char* cmd, char* resp){

	char		response[1024];
	cmd_type_t	req_type;
	const char*	arg;
	int 		rc;

	char* pch;
	char* poscmd;

	// find equal sign in command
	// if there is equal sign, cut arg from command into separate variables
	//poseq = cmd.find("=");
	pch = strchr(cmd, '=');

	// command is followed by equal sing (probably PARAM type)
	if(pch!=NULL){
		arg = pch + 1;

		// trim command after '=' character
		pch[0] = 0x00;

		req_type=COMMAND_TYPE_PARAM;

	// not followed by equal sign (probably WITHOUT param type)
	} else {
		req_type=COMMAND_TYPE_WITHOUT;
	}

	// convert command to lowercase (not the argument)
    for (uint32_t i=0; i<strlen(cmd); i++){
         cmd[i]=tolower(cmd[i]);
    }

	// search all available commands
	for(uint32_t i = 0; i<_cmdcount; i++){

		// find requested command in command set
		poscmd = strstr(cmd, commands[i].name);

		// command not found in supported commands
		// or begin of requested command is not matched with begin of supported command
		// e.g. set_defaults != defaults
		if((poscmd==NULL)||(poscmd!=cmd)){
			continue;
		}

		// restart can match restartbl, so check whole lengths
		if(strlen(poscmd)!=strlen(commands[i].name)){
			continue;
		}

		/*
		 * CHECK IF COMMAND SUPPORTS REQUESTED METHOD AND CALL IT
		 */
		// requesting command without parameter
		if(req_type==COMMAND_TYPE_WITHOUT){

			// command supports variant without parameter
			if((commands[i].type==COMMAND_TYPE_WITHOUT)||(commands[i].type==COMMAND_TYPE_BOTH)){

				rc = commands[i].fp(COMMAND_TYPE_WITHOUT, NULL, response);

			// not supported, probably because command needs parameter
			} else {
				sprintf(resp, "%s=error", commands[i].name);
				return COMMAND_RESPONSE_ERROR;
			}

		// requesting command WITH parameter
		} else if (req_type==COMMAND_TYPE_PARAM){

			// command supports variant with parameter
			if((commands[i].type==COMMAND_TYPE_PARAM)||(commands[i].type==COMMAND_TYPE_BOTH)){

				rc = commands[i].fp(COMMAND_TYPE_PARAM, arg, response);

			// not supported, because command is only possible without argument
			} else {
				sprintf(resp, "%s=error", commands[i].name);
				return COMMAND_RESPONSE_ERROR;
			}

		}

		// show response
		switch(rc){
		case COMMAND_RESPONSE_OK:

			sprintf(resp, "%s=%s\n", commands[i].name, response);
			return 1;

		case COMMAND_RESPONSE_MISSING:

			// don't show response
			// help and overview
			return 0;

		case COMMAND_RESPONSE_ERROR:

			sprintf(resp, "%s=error\n", commands[i].name);
			return -1;

		default:
			break;

		}

	}

	sprintf(resp, "%s=command invalid", cmd);

	return -1;

}

int BootloaderCommander::cmd_ping(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	strcpy(resp, "ok");

	return COMMAND_RESPONSE_OK;

}

int BootloaderCommander::cmd_help(cmd_type_t type, const char* arg, char* resp){

	UNUSED(arg);
	UNUSED(resp);

	for(uint32_t i = 0; i<_cmdcount; i++){
		__INFO("%s (%s)\n", commands[i].name, type_to_str(commands[i].type));
	}

	return COMMAND_RESPONSE_MISSING;

}

int BootloaderCommander::cmd_overview(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);
	UNUSED(resp);

	char response[256];

	cmd_target(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_target), response);
	cmd_fullid(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_fullid), response);

#if BYZANCE_SUPPORTS_DEVLIST
	// devlist
	cmd_devlist_clear(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_devlist_clear), response);
	cmd_devlist_counter(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_devlist_counter), response);
	cmd_devlist_list(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_devlist_list), response);
#endif

	/**********************************************
	 *
	 * WITH AND WITHOUT PARAMETER
	 *
	 **********************************************/

	cmd_normal_mqtt_hostname(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_normal_mqtt_hostname), response);
	cmd_normal_mqtt_port(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_normal_mqtt_port), response);
	cmd_backup_mqtt_hostname(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_backup_mqtt_hostname), response);
	cmd_backup_mqtt_port(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_backup_mqtt_port), response);

	// username and password is hidden
	cmd_mqtt_username(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=*****\n", _get_name(cmd_mqtt_username));
	cmd_mqtt_password(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=*****\n", _get_name(cmd_mqtt_password));

	cmd_alias(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_alias), response);
	cmd_mac(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_mac), response);
	cmd_blreport(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_blreport), response);
	cmd_wdenable(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_wdenable), response);
	cmd_wdtime(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_wdtime), response);
	cmd_autobackup(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_autobackup), response);
	cmd_netsource(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_netsource), response);
	cmd_configured(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_configured), response);
	cmd_timestamp(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_timestamp), response);
	cmd_webview(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_webview), response);
	cmd_webport(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_webport), response);
	cmd_timeoffset(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_timeoffset), response);
	cmd_timesync(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_timesync), response);
	cmd_lowpanbr(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_lowpanbr), response);
	cmd_restartbl(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_restartbl), response);
	cmd_revision(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_revision), response);
	cmd_autojump(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s=%s\n", _get_name(cmd_autojump), response);
  
#if BYZANCE_SUPPORTS_WIFI
	cmd_wifi_ssid(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s\n", _get_name(cmd_wifi_ssid), response);
	cmd_wifi_pass(COMMAND_TYPE_WITHOUT, "", response);
	__INFO("%s\n", _get_name(cmd_wifi_pass), response);
#endif

	/**********************************************
	 *
	 * PARAMETER ONLY
	 *
	 **********************************************/

	// bootloader, firmware, buffer, backup
	cmd_info(COMMAND_TYPE_PARAM, "bootloader", response);
	__INFO("%s=%s\n", _get_name(cmd_info), response);
	cmd_info(COMMAND_TYPE_PARAM, "firmware", response);
	__INFO("%s=%s\n", _get_name(cmd_info), response);
	cmd_info(COMMAND_TYPE_PARAM, "buffer", response);
	__INFO("%s=%s\n", _get_name(cmd_info), response);
	cmd_info(COMMAND_TYPE_PARAM, "backup", response);
	__INFO("%s=%s\n", _get_name(cmd_info), response);

	// bootloader, firmware, buffer, backup
	cmd_memsize(COMMAND_TYPE_PARAM, "bootloader", response);
	__INFO("%s=%s\n", _get_name(cmd_memsize), response);
	cmd_memsize(COMMAND_TYPE_PARAM, "firmware", response);
	__INFO("%s=%s\n", _get_name(cmd_memsize), response);
	cmd_memsize(COMMAND_TYPE_PARAM, "buffer", response);
	__INFO("%s=%s\n", _get_name(cmd_memsize), response);
	cmd_memsize(COMMAND_TYPE_PARAM, "backup", response);
	__INFO("%s=%s\n", _get_name(cmd_memsize), response);

	return COMMAND_RESPONSE_MISSING;
}

int BootloaderCommander::cmd_restart(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);
	UNUSED(resp);

	wait_ms(100);
	HAL_NVIC_SystemReset();

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_target(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	strcpy(resp, TOSTRING(__BUILD_TARGET__));

	return COMMAND_RESPONSE_OK;

}

int BootloaderCommander::cmd_fullid(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	// fullid
	char full_id[32];
	get_fullid(full_id);
	strcpy(resp, full_id);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_launch_reset(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	int rc = 0;

	rc = Configurator::set_launched(0);
	if(rc){
		strcpy(resp, "error");
		return COMMAND_RESPONSE_ERROR;
	}

	strcpy(resp, "ok");

	return COMMAND_RESPONSE_OK;

}

int BootloaderCommander::cmd_defaults(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	BinManager::set_defaults(1);
	Configurator::set_defaults(1);
	MqttManager::set_defaults(1);

	strcpy(resp, "ok");

	return COMMAND_RESPONSE_OK;
}

#if BYZANCE_SUPPORTS_DEVLIST
int BootloaderCommander::cmd_devlist_clear(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	uint32_t cnt = 0;
	_extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_DEVLIST_COUNTER, &cnt, EXTMEM_SIZEOF_DEVLIST_COUNTER);
	strcpy(resp, "ok");

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_devlist_counter(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	_extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_DEVLIST_COUNTER, &tmp32, EXTMEM_SIZEOF_DEVLIST_COUNTER);
	sprintf(resp, "%u", tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_devlist_list(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);
	UNUSED(arg);

	// TODO: implement

}
#endif

int BootloaderCommander::cmd_normal_mqtt_hostname(cmd_type_t type, const char* arg, char* resp){

	if (type == COMMAND_TYPE_PARAM){

		MqttManager::set_hostname(MQTT_NORMAL, arg);

	}

	MqttManager::get_hostname(MQTT_NORMAL, resp);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_normal_mqtt_port(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	// write
	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		MqttManager::set_port(MQTT_NORMAL, tmp32);

	}

	MqttManager::get_port(MQTT_NORMAL, &tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_backup_mqtt_hostname(cmd_type_t type, const char* arg, char* resp){

	if (type == COMMAND_TYPE_PARAM){

		MqttManager::set_hostname(MQTT_BACKUP, arg);

	}

	MqttManager::get_hostname(MQTT_BACKUP, resp);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_backup_mqtt_port(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		MqttManager::set_port(MQTT_BACKUP, tmp32);

	}

	MqttManager::get_port(MQTT_BACKUP, &tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_mqtt_username(cmd_type_t type, const char* arg, char* resp){

	UNUSED(resp);

	if (type == COMMAND_TYPE_PARAM){

		MqttManager::set_username(arg);

	}

	MqttManager::get_username(resp);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_mqtt_password(cmd_type_t type, const char* arg, char* resp){

	UNUSED(resp);

	if (type == COMMAND_TYPE_PARAM){

		MqttManager::set_password(arg);

	}

	MqttManager::get_password(resp);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_alias(cmd_type_t type, const char* arg, char* resp){

	if (type == COMMAND_TYPE_PARAM){

		Configurator::set_alias(arg);

	}

	Configurator::get_alias(resp);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_mac(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	// expected in format XX:XX:XX:XX:XX:XX
	char saved_mac_ascii[32] = {0};
	get_mac_address(saved_mac_ascii);

	if (type == COMMAND_TYPE_PARAM){

		char saved_mac_bytes[16];

		mac_to_bytes(saved_mac_ascii, saved_mac_bytes);

		// some MAC addres is already set
		if((saved_mac_bytes[0]!=0xFF) || (saved_mac_bytes[1]!=0xFF) || (saved_mac_bytes[2]!=0xFF) || (saved_mac_bytes[3]!=0xFF) || (saved_mac_bytes[4]!=0xFF) || (saved_mac_bytes[5]!=0xFF)){

			// just continue and return current mac, not the new one

		// setting MAC for the first time
		} else {

			// raw data
			// 6 octets + termination null
			// todo define velikosti
			char received_mac_bytes[16];

			// string substr(pos, len)
			// expected in format XX:XX:XX:XX:XX:XX
			mac_to_bytes(arg, received_mac_bytes);

			OTP::write(0, 0, received_mac_bytes[0]);
			OTP::write(0, 1, received_mac_bytes[1]);
			OTP::write(0, 2, received_mac_bytes[2]);
			OTP::write(0, 3, received_mac_bytes[3]);
			OTP::write(0, 4, received_mac_bytes[4]);
			OTP::write(0, 5, received_mac_bytes[5]);

		}

	}

	//get_mac_address(saved_mac_ascii);
	strcpy(resp, saved_mac_ascii);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_blreport(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_blreport(tmp32);

	}

	Configurator::get_blreport(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_wdenable(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){
		tmp32 = atoi(arg);
		Configurator::set_wdenable(tmp32);

	}

	Configurator::get_wdenable(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_wdtime(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_wdtime(tmp32);

	}

	Configurator::get_wdtime(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_autobackup(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_autobackup(tmp32);

	}

	Configurator::get_autobackup(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_netsource(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		if(strcmp(arg, "disabled")==0){
			tmp32 = NETSOURCE_DISABLED;
			Configurator::set_netsource(tmp32);
		} else if(strcmp(arg, "ethernet")==0){
			tmp32 = NETSOURCE_ETHERNET;
			Configurator::set_netsource(tmp32);
		} else if(strcmp(arg, "wifi")==0){
			tmp32 = NETSOURCE_WIFI;
			Configurator::set_netsource(tmp32);
		} else if(strcmp(arg, "6lowpan")==0){
			tmp32 = NETSOURCE_6LOWPAN;
			Configurator::set_netsource(tmp32);
		} else {
			strcpy(resp, "invalid argument");
			return COMMAND_RESPONSE_ERROR;
		}

	}

	Configurator::get_netsource(&tmp32);

	switch(tmp32){
	case NETSOURCE_DISABLED:
		strcpy(resp, "disabled");
		break;
	case NETSOURCE_ETHERNET:
		strcpy(resp, "ethernet");
		break;
	case NETSOURCE_WIFI:
		strcpy(resp, "wifi");
		break;
	case NETSOURCE_6LOWPAN:
		strcpy(resp, "6lowpan");
		break;
	default:
		strcpy(resp, "unknown");
	}

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_configured(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){
		tmp32 = atoi(arg);
		Configurator::set_configured(tmp32);

	}

	Configurator::get_configured(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_timestamp(cmd_type_t type, const char* arg, char* resp){

	time_t timestamp = 0;

	if (type == COMMAND_TYPE_PARAM){

		timestamp = atoi(arg);
		set_time(timestamp);

	}

	time(&timestamp);
	sprintf(resp, "%u", (unsigned int)timestamp);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_webview(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){
		tmp32 = atoi(arg);
		Configurator::set_webview(tmp32);

	}

	Configurator::get_webview(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_webport(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_webport(tmp32);

	}

	Configurator::get_webport(&tmp32);
	sprintf(resp, "%u", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_timeoffset(cmd_type_t type, const char* arg, char* resp){

	int tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_timeoffset(tmp32);

	}

	Configurator::get_timeoffset(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_timesync(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_timesync(tmp32);

	}

	Configurator::get_timesync(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_lowpanbr(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_lowpanbr(tmp32);

	}

	Configurator::get_lowpanbr(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;
}

int BootloaderCommander::cmd_restartbl(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_restartbl(tmp32);

	}

	Configurator::get_restartbl(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;

}

int BootloaderCommander::cmd_revision(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = get_revision();

		// already set
		if(tmp32!=0xFFFFFFFF){

			sprintf(resp, "revision already set: %08X", get_revision());

		// not set yet
		} else {

			tmp32 = strtoul(arg, NULL, 16);

			// save it litte endian
			uint8_t tmp8 = 0;
			tmp8 = (tmp32>> 0)&0xFF;
			OTP::write(1, 0, tmp8);
			tmp8 = (tmp32>> 8)&0xFF;
			OTP::write(1, 1, tmp8);
			tmp8 = (tmp32>>16)&0xFF;
			OTP::write(1, 2, tmp8);
			tmp8 = (tmp32>>24)&0xFF;
			OTP::write(1, 3, tmp8);

		}

	}

	// read back from OTP
	tmp32 = get_revision();
	sprintf(resp, "%08X", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;

}

int BootloaderCommander::cmd_autojump(cmd_type_t type, const char* arg, char* resp){

	uint32_t tmp32 = 0;

	if (type == COMMAND_TYPE_PARAM){

		tmp32 = atoi(arg);
		Configurator::set_autojump(tmp32);

	}

	Configurator::get_autojump(&tmp32);
	sprintf(resp, "%d", (unsigned int)tmp32);

	return COMMAND_RESPONSE_OK;

}

#if BYZANCE_SUPPORTS_WIFI
int BootloaderCommander::cmd_wifi_ssid(cmd_type_t type, const char* arg, char* resp){

	struct_wifi_credentials tmp_credentials;
	_extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_WIFI_CREDENTIALS, &tmp_credentials, sizeof(struct_wifi_credentials));

	// read
	if(type == COMMAND_TYPE_WITHOUT){

	// write
	} else if (type == COMMAND_TYPE_PARAM){

	}
}

int BootloaderCommander::cmd_wifi_pass(cmd_type_t type, const char* arg, char* resp){

	struct_wifi_credentials tmp_credentials;
	_extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_WIFI_CREDENTIALS, &tmp_credentials, sizeof(struct_wifi_credentials));

	// read
	if(type == COMMAND_TYPE_WITHOUT){

	// write
	} else if (type == COMMAND_TYPE_PARAM){

	}
}
#endif

int BootloaderCommander::cmd_info(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);

	struct_binary bin_tmp;

	resp[0] = 0x00;

	if(strcmp(arg, "bootloader")==0){

		BinManager::get_bin_info(BIN_COMPONENT_BOOTLOADER, &bin_tmp);
		strcat(resp, arg);
		strcat(resp, "-");
		show_binary(&bin_tmp, resp);
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "firmware")==0){

		BinManager::get_bin_info(BIN_COMPONENT_FIRMWARE, &bin_tmp);
		strcat(resp, arg);
		strcat(resp, "-");
		show_binary(&bin_tmp, resp);
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "backup")==0){

		BinManager::get_bin_info(BIN_COMPONENT_BACKUP, &bin_tmp);
		strcat(resp, arg);
		strcat(resp, "-");
		show_binary(&bin_tmp, resp);
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "buffer")==0){

		BinManager::get_bin_info(BIN_COMPONENT_BUFFER, &bin_tmp);
		strcat(resp, arg);
		strcat(resp, "-");
		show_binary(&bin_tmp, resp);
		return COMMAND_RESPONSE_OK;
	}

	strcpy(resp, "error");
	return COMMAND_RESPONSE_ERROR;

}

int BootloaderCommander::cmd_memsize(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);

	resp[0] = 0x00;

	if(strcmp(arg, "bootloader")==0){
		strcat(resp, arg);
		strcat(resp, "-");
		sprintf(resp + strlen(arg) + 1, "%d", (unsigned int)INTMEM_SIZEOF_BOOTLOADER);
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "firmware")==0){
		strcat(resp, arg);
		strcat(resp, "-");
		sprintf(resp + strlen(arg) + 1, "%d", (unsigned int)INTMEM_SIZEOF_FIRMWARE);
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "backup")==0){
		strcat(resp, arg);
		strcat(resp, "-");
		sprintf(resp + strlen(arg) + 1, "%d", (unsigned int)EXTMEM_SIZEOF_BACKUP);
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "buffer")==0){
		strcat(resp, arg);
		strcat(resp, "-");
		sprintf(resp + strlen(arg) + 1, "%d", (unsigned int)EXTMEM_SIZEOF_BUFFER);
		return COMMAND_RESPONSE_OK;

	}

	strcpy(resp, "error");
	return COMMAND_RESPONSE_ERROR;

}

int BootloaderCommander::cmd_format(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);

	if(strcmp(arg, "intmem")==0){

		_intmem->erase(INTPART_FIRMWARE, INTMEM_END);

		strcpy(resp, "ok");

		return COMMAND_RESPONSE_OK;

	} else if (strcmp(arg, "extmem")==0){

		for(uint32_t i=0; i<SPIFLASH_SECTORS; i++){
			_extmem->erase_sectors(i, 1);

			// callback for external led blinking
			if(_led_blink){
				_led_blink.call();
			}

		}

		BinManager::set_defaults(1);
		Configurator::set_defaults(1);
		MqttManager::set_defaults(1);

		strcpy(resp, "ok");

		return COMMAND_RESPONSE_OK;
	}

	strcpy(resp, "unsupported");

	return COMMAND_RESPONSE_ERROR;


}

int BootloaderCommander::cmd_firmware(cmd_type_t type, const char* arg, char* resp){

	UNUSED(type);

	if(strcmp(arg, "backup")==0){

		int rc = 0;

		rc = BinManager::backup_start(true);
		do{
			rc = BinManager::yield();
		} while (rc);

		strcpy(resp, "ok");
		return COMMAND_RESPONSE_OK;

	} else if(strcmp(arg, "restore")==0){

		uint32_t rc = 0;

		rc = BinManager::flash_firmware(MODE_RESTORE);

		if(rc){
			return COMMAND_RESPONSE_ERROR;
		}

		strcpy(resp, "ok");
		return COMMAND_RESPONSE_OK;

	}

	strcpy(resp, "error");

	return COMMAND_RESPONSE_ERROR;

}

const char* BootloaderCommander::type_to_str(cmd_type_t type){

	if(type==COMMAND_TYPE_WITHOUT){
		return "-";
	} else if (type == COMMAND_TYPE_PARAM){
		return "=";
	} else if (type == COMMAND_TYPE_BOTH){
		return "-/=";
	}

	return "";
}

void BootloaderCommander::show_binary(struct_binary* bin, char* resp){
	UNUSED(bin);
	char buffer [256];

	sprintf(buffer, "size:%u;",	(unsigned int)bin->size);
	strcat(resp, buffer);
	sprintf(buffer, "crc:0x%08X;", (unsigned int)bin->crc);
	strcat(resp, buffer);
	sprintf(buffer, "build_id:%s;",	bin->build_id);
	strcat(resp, buffer);
	sprintf(buffer, "version:%s;", bin->version);
	strcat(resp, buffer);
	sprintf(buffer, "timestamp:%u;", (unsigned int)bin->timestamp);
	strcat(resp, buffer);
	sprintf(buffer, "usr_version:%s;",	bin->usr_version);
	strcat(resp, buffer);
	sprintf(buffer, "usr_name:%s;",		bin->usr_name);
	strcat(resp, buffer);

	switch(bin->state){
	case BINSTRUCT_STATE_INVALID:
		strcat(resp, "state:invalid;");
		break;
	case BINSTRUCT_STATE_VALID:
		strcat(resp, "state:valid;");
		break;
	case BINSTRUCT_STATE_ERASING:
		strcat(resp, "state:erasing;");
		break;
	case BINSTRUCT_STATE_ERASED:
		strcat(resp, "state:erased;");
		break;
	default:
		strcat(resp, "state:unknown;");
		break;
	}

	return;

}

void BootloaderCommander::attach_led_blink(void (*function)(void)) {
	_led_blink = callback(function);
}

const char* BootloaderCommander::_get_name(int (*fp)(cmd_type_t type, const char*, char*)){

	for(uint32_t i = 0; i<_cmdcount; i++){

		if(commands[i].fp == fp){
			return commands[i].name;
		}
	}

	return NULL;
}

#endif /* BYZANCE_BOOTLOADER */
