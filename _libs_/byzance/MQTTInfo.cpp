#include "MQTTInfo.h"
#include "LowpanBR.h"
#include "IODA_general.h"
#include "HttpServer.h"

REGISTER_DEBUG(MQTTInfo);

NetworkInterface*	MQTTInfo::_itf;

worker_result		MQTTInfo::_result;

Callback<void(worker_result*)>	MQTTInfo::_req_done;

size_t 		MQTTInfo::_cmdcount;

void MQTTInfo::init(picojson::object* json_outcomming){

	_result.json_out = json_outcomming;

	// https://stackoverflow.com/questions/1898657/result-of-sizeof-on-array-of-structs-in-c
	_cmdcount = sizeof inf_topics / sizeof inf_topics[0];

}

void MQTTInfo::init_itf(NetworkInterface* itf){
	_itf	= itf;
}

void MQTTInfo::process(picojson::value& json_data_incomming, const char* subtopic_incomming){

	char* poscmd;

	for(uint32_t i = 0; i<_cmdcount; i++){

		// find requested command in command set
		poscmd = strstr(subtopic_incomming, inf_topics[i].name);

		// doplnit druhou část
		if(poscmd==NULL){
			continue;
		}

		// restart can match restartbl, so check whole lengths
		if(strlen(poscmd)!=strlen(inf_topics[i].name)){
			continue;
		}

		__INFO("topic found -> %s\n", inf_topics[i].name);

		inf_topics[i].fp(json_data_incomming);

		return;

	}

	__ERROR("unknown topic\n");

	// not found
	_result.rc = ERROR_CODE_UNKNOWN_TOPIC;
	_req_done(&_result);
	return;

}

void MQTTInfo::target(picojson::value& json_data_incomming){

	__TRACE("homer is requesting target\n");

	(*_result.json_out)["target"] = picojson::value(TOSTRING_TARGET(__BUILD_TARGET__));
	__INFO("target = %s\n", TOSTRING_TARGET(__BUILD_TARGET__));

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::cpuload(picojson::value& json_data_incomming){
	__TRACE("homer is requesting cpuload\n");

	(*_result.json_out)["cpuload"] = picojson::value((double)ByzanceCore::cpu_load);
	__INFO("cpuload = %u\n", ByzanceCore::cpu_load);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;
}

void MQTTInfo::alias(picojson::value& json_data_incomming){

	__TRACE("homer is requesting alias\n");

	char buff[64];
	int rc = Configurator::get_alias(buff);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		(*_result.json_out)["alias"] = picojson::value(buff);
		__INFO("alias = %s\n", buff);
		_result.rc = ERROR_CODE_OK;
	}


	_req_done(&_result);
	return;
}

void MQTTInfo::blreport(picojson::value& json_data_incomming){

	__TRACE("homer is requesting blreport\n");

	uint32_t tmp32 = 0;

	int rc = Configurator::get_blreport(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{

		if(tmp32){
			(*_result.json_out)["blreport"] = picojson::value(true);
		} else {
			(*_result.json_out)["blreport"] = picojson::value(false);
		}

		__INFO("blreport = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;
}

void MQTTInfo::wdenable(picojson::value& json_data_incomming){

	__TRACE("homer is requesting wdenable\n");

	uint32_t tmp32 = 0;

	int rc = Configurator::get_wdenable(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{

		if(tmp32){
			(*_result.json_out)["wdenable"] = picojson::value(true);
		} else {
			(*_result.json_out)["wdenable"] = picojson::value(false);
		}

		__INFO("wdenable = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::wdtime(picojson::value& json_data_incomming){

	__TRACE("homer is requesting wdtime\n");

	uint32_t tmp32 = 0;

	int rc = Configurator::get_wdtime(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		(*_result.json_out)["wdtime"] = picojson::value((double)tmp32);

		__INFO("wdtime = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;
}

void MQTTInfo::backuptime(picojson::value& json_data_incomming){

	__TRACE("homer is requesting backuptime\n");

/*
* dummy
*/

	(*_result.json_out)["backuptime"] = picojson::value((double)60);

	__INFO("backuptime = %d\n", (int)60);

	_result.rc = ERROR_CODE_OK;

	_req_done(&_result);
	return;

}

void MQTTInfo::lowpanbr(picojson::value& json_data_incomming){

	__TRACE("homer is requesting lowpanbr\n");

	uint32_t tmp32 = 0;

	int rc = Configurator::get_lowpanbr(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		if(tmp32){
			(*_result.json_out)["lowpanbr"] = picojson::value(true);
		} else {
			(*_result.json_out)["lowpanbr"] = picojson::value(false);
		}

		__INFO("lowpanbr = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::autojump(picojson::value& json_data_incomming){

	__TRACE("homer is requesting autojump\n");

	uint32_t tmp32 = 0;

	int rc = Configurator::get_autojump(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		(*_result.json_out)["autojump"] = picojson::value((double)tmp32);

		__INFO("autojump = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;
}

void MQTTInfo::restartbl(picojson::value& json_data_incomming){

	__TRACE("homer is requesting restartbl\n");

	uint32_t tmp32 = 0;

	int rc = Configurator::get_restartbl(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		(*_result.json_out)["restartbl"] = picojson::value((double)tmp32);

		__INFO("restartbl = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}

	_req_done(&_result);
	return;

}

void MQTTInfo::version(picojson::value& json_data_incomming){

	char buffer[32];
	string component;
	struct_binary tmp_bin;

	__TRACE("version: homer is requesting component version\n");

	// compulsory label "component"
	if(!json_data_incomming.contains("component")){
		_result.rc = ERROR_CODE_MISSING_LABEL;
		_req_done(&_result);
		return;
	}

	// copy from JSON label to string
	component.assign(json_data_incomming.get("component").get<string>());
	// copy component to response JSON
	(*_result.json_out)["component"] = picojson::value(component.c_str());

	// load corresponding component info from extmem
	if(strcmp(component.c_str(), "firmware")==0){
		__INFO("version: component FIRMWARE\n");
		BinManager::get_bin_info(BIN_COMPONENT_FIRMWARE, &tmp_bin);
	} else if(strcmp(component.c_str(), "bootloader")==0){
		__INFO("version: component BOOTLOADER\n");
		BinManager::get_bin_info(BIN_COMPONENT_BOOTLOADER, &tmp_bin);
	} else if(strcmp(component.c_str(), "backup")==0){
		__INFO("version: component BACKUP\n");
		BinManager::get_bin_info(BIN_COMPONENT_BACKUP, &tmp_bin);
	} else if(strcmp(component.c_str(), "buffer")==0){
		__INFO("version: component BUFFER\n");
		BinManager::get_bin_info(BIN_COMPONENT_BUFFER, &tmp_bin);
	} else {
		_result.rc = ERROR_CODE_UNKNOWN_LABEL;
		_req_done(&_result);
		return;
	}

	(*_result.json_out)["size"] = picojson::value((double)tmp_bin.size);
	__INFO("version: size = %d\n", tmp_bin.size);

	(*_result.json_out)["crc"] = picojson::value((double)tmp_bin.crc);
	__INFO("version: crc = 0x%08X\n", tmp_bin.crc);

	(*_result.json_out)["build_id"] = picojson::value(tmp_bin.build_id);
	__INFO("version: build_id = %s\n", tmp_bin.build_id);

	(*_result.json_out)["version"] = picojson::value(tmp_bin.version);
	__INFO("version: version = %s\n", tmp_bin.version);

	(*_result.json_out)["timestamp"] = picojson::value((double)tmp_bin.timestamp);
	__INFO("version: timestamp = %u\n", tmp_bin.timestamp);

	(*_result.json_out)["usr_version"] = picojson::value(tmp_bin.usr_version);
	__INFO("version: usr_version = %s\n", tmp_bin.usr_version);

	(*_result.json_out)["usr_name"] = picojson::value(tmp_bin.usr_name);
	__INFO("version: usr_name = %s\n", tmp_bin.usr_name);

	char buff[16];

	BinManager::name_state(tmp_bin.state, buff);
	(*_result.json_out)["state"] = picojson::value(buff);
	__INFO("version: state - %s\n", buff);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;
}

void MQTTInfo::mac(picojson::value& json_data_incomming){

	__TRACE("homer is requesting MAC address\n");

	// determine mac address
	char mac_addr[32];
	get_mac_address(mac_addr);
	(*_result.json_out)["mac"] = picojson::value(mac_addr);
	__INFO("mac: %s\n", mac_addr);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::ip(picojson::value& json_data_incomming){

	__TRACE("homer is requesting IP\n");

	// todo: nejaky define
	const char *ip;
	ip = _itf->get_ip_address();

	(*_result.json_out)["ip"] = picojson::value(ip);
	__INFO("ip: %s\n", ip);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::memsize(picojson::value& json_data_incomming){

	__TRACE("homer is requesting memsize\n");

	// maximální velikost bootloaderu v bytech
	(*_result.json_out)["bootloader"] = picojson::value((double)INTMEM_SIZEOF_BOOTLOADER);
	__INFO("memsize: bootloader = %d\n", INTMEM_SIZEOF_BOOTLOADER);

	// maximální velikost firmware v bytech
	(*_result.json_out)["firmware"] = picojson::value((double)INTMEM_SIZEOF_BOOTLOADER);
	__INFO("memsize: firmware = %d\n", INTMEM_SIZEOF_BOOTLOADER);

	// maximální velikost backupu v bytech
	(*_result.json_out)["buffer"] = picojson::value((double)EXTMEM_SIZEOF_BACKUP);
	__INFO("memsize: backup = %d\n", EXTMEM_SIZEOF_BACKUP);

	// maximální velikost bufferu pro binárky v bytech
	(*_result.json_out)["buffer"] = picojson::value((double)EXTMEM_SIZEOF_BUFFER);
	__INFO("memsize: buffer = %d\n", EXTMEM_SIZEOF_BUFFER);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::datetime(picojson::value& json_data_incomming){

	__TRACE("homer is requesting datetime\n");

	time_t timestamp = 0;
	time(&timestamp);

	(*_result.json_out)["datetime"] = picojson::value((double)timestamp);
	__INFO("datetime = %u\n", (unsigned int)timestamp);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::timeoffset(picojson::value& json_data_incomming){

	__TRACE("homer is requesting timeoffset\n");

	int tmp32 = 0;

	int rc = Configurator::get_timeoffset(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		(*_result.json_out)["timeoffset"] = picojson::value((double)tmp32);

		__INFO("timeoffset = %d\n", (int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::timesync(picojson::value& json_data_incomming){

	__TRACE("homer is requesting timesync\n");

	uint32_t tmp32 = 0;
	int rc = Configurator::get_timesync(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		if(tmp32){
			(*_result.json_out)["timesync"] = picojson::value(true);
		} else {
			(*_result.json_out)["timesync"] = picojson::value(false);
		}

		__INFO("timesync = %d\n", tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::uptime(picojson::value& json_data_incomming){

	__TRACE("homer is requesting uptime\n");

	(*_result.json_out)["uptime"] = picojson::value((double)ByzanceCore::uptime);
	__INFO("uptime = %u\n", (unsigned int)ByzanceCore::uptime);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::autobackup(picojson::value& json_data_incomming){

	__TRACE("homer is requesting autobackup\n");

	uint32_t tmp32 = 0;
	int rc = Configurator::get_autobackup(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		if(tmp32){
			(*_result.json_out)["autobackup"] = picojson::value(true);
			__INFO("autobackup = true\n");
		} else {
			(*_result.json_out)["autobackup"] = picojson::value(false);
			__DEBUG("autobackup = false\n");
		}

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::state(picojson::value& json_data_incomming){

	bin_state_t state;

	char buff[16];

	__TRACE("homer is requesting state\n");

	/*
	 * BUFFER
	 */
	BinManager::get_state(BIN_COMPONENT_BUFFER, &state);
	BinManager::name_state(state, buff);
	(*_result.json_out)["buffer"] = picojson::value(buff);
	__INFO("state buffer - %s\n", buff);

	/*
	 * BACKUP
	 */
	BinManager::get_state(BIN_COMPONENT_BACKUP, &state);
	BinManager::name_state(state, buff);
	(*_result.json_out)["backup"] = picojson::value(buff);
	__INFO("state backup - %s\n", buff);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::normal_mqtt_connection(picojson::value& json_data_incomming){

	mqtt_connection tmp_mqtt;

	__TRACE("homer is requesting normal_mqtt_connection\n");

	MqttManager::get_connection(MQTT_NORMAL, &tmp_mqtt);

	string buffer;
	char port_char[10];
	sprintf(port_char, "%d", (unsigned int)tmp_mqtt.port);

	buffer+=tmp_mqtt.hostname;
	buffer+=":";
	buffer+=port_char;

	(*_result.json_out)["connection"] = picojson::value(buffer);
	__INFO("normal_mqtt_connection %s\n", buffer.c_str());

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::backup_mqtt_connection(picojson::value& json_data_incomming){

	mqtt_connection tmp_mqtt;

	__TRACE("homer is requesting backup_mqtt_connection\n");

	MqttManager::get_connection(MQTT_BACKUP, &tmp_mqtt);

	string buffer;
	char port_char[10];
	sprintf(port_char, "%u", (unsigned int)tmp_mqtt.port);

	buffer+=tmp_mqtt.hostname;
	buffer+=":";
	buffer+=port_char;

	(*_result.json_out)["connection"] = picojson::value(buffer);
	__INFO("backup_mqtt_connection = %s\n", buffer.c_str());

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::console(picojson::value& json_data_incomming){

	__TRACE("homer is requesting console\n");

	bool en;
	en = Console::enabled();

	(*_result.json_out)["console"] = picojson::value(en);

	if(en){
		__INFO("console enabled\n");
	} else {
		__INFO("console disabled\n");
	}

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::webview(picojson::value& json_data_incomming){

	__TRACE("homer is requesting webview\n");

	uint32_t tmp32 = 0;
	int rc = Configurator::get_webview(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		if(tmp32){
			(*_result.json_out)["webview"] = picojson::value(true);
		} else {
			(*_result.json_out)["webview"] = picojson::value(false);
		}

		__INFO("webview = %d\n", tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::webport(picojson::value& json_data_incomming){

	__TRACE("homer is requesting webport\n");

	uint32_t tmp32 = 0;
	int rc = Configurator::get_webport(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		(*_result.json_out)["webport"] = picojson::value((double)tmp32);

		__INFO("webport = %u\n", (unsigned int)tmp32);

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::netsource(picojson::value& json_data_incomming){

	__TRACE("homer is requesting netsource\n");

	uint32_t tmp32 = 0;
	int rc = Configurator::get_netsource(&tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{

		switch(tmp32){
		case NETSOURCE_DISABLED:
			(*_result.json_out)["netsource"] = picojson::value("disabled");
			__INFO("netsource = disabled\n");
			break;
		case NETSOURCE_ETHERNET:
			(*_result.json_out)["netsource"] = picojson::value("ethernet");
			__INFO("netsource = ethernet\n");
			break;
		case NETSOURCE_WIFI:
			(*_result.json_out)["netsource"] = picojson::value("wifi");
			__INFO("netsource = wifi\n");
			break;
		case NETSOURCE_6LOWPAN:
			(*_result.json_out)["netsource"] = picojson::value("6lowpan");
			__INFO("netsource = 6lowpan\n");
			break;
		default:
			(*_result.json_out)["netsource"] = picojson::value("unknown");
			__INFO("netsource = unknown\n");
		}

		_result.rc = ERROR_CODE_OK;
	}
	_req_done(&_result);
	return;

}

void MQTTInfo::revision(picojson::value& json_data_incomming){

	__TRACE("homer is requesting revision\n");

	char buffer[32];
	sprintf(buffer, "0x%02X", (unsigned int)get_revision());

	(*_result.json_out)["revision"] = picojson::value(buffer);

	__INFO("revision = %s\n", buffer);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;
}

void MQTTInfo::threads(picojson::value& json_data_incomming){

	__TRACE("homer is requesting revision\n");

	char buffer[256], buffer2[64];


	sprintf(buffer, "%d %%", ByzanceCore::cpu_load);
	(*_result.json_out)["_CPU load_"] = picojson::value(buffer);

	//////// ByzanceClient ////////
	get_name_state(ByzanceClient::get_state(), buffer2);
	sprintf(buffer, "size=%4d, free=%4d, used=%4d, max=%4d, %s",
			(int)ByzanceClient::get_stack_size(),
			(int)ByzanceClient::get_free_stack(),
			(int)ByzanceClient::get_used_stack(),
			(int)ByzanceClient::get_max_stack(),
			buffer2);
	(*_result.json_out)["ByzanceClient"] = picojson::value(buffer);

	//////// LowpanBR ////////
	/*get_name_state(LowpanBR::get_thread_state(), buffer2);
	sprintf(buffer, "size=%4d, free=%4d, used=%4d, max=%4d, %s",
			(int)LowpanBR::get_stack_size(),
			(int)LowpanBR::get_free_stack(),
			(int)LowpanBR::get_used_stack(),
			(int)LowpanBR::get_max_stack(),
			buffer2);
	(*_result.json_out)["LowpanBR"] = picojson::value(buffer);*/


	//////// Byzance ////////
	get_name_state(Byzance::get_state(), buffer2);
	sprintf(buffer, "size=%4d, free=%4d, used=%4d, max=%4d, %s",
			(int)Byzance::get_stack_size(),
			(int)Byzance::get_free_stack(),
			(int)Byzance::get_used_stack(),
			(int)Byzance::get_max_stack(),
			buffer2);
	(*_result.json_out)["Byzance"] = picojson::value(buffer);


	//////// HttpServer ////////
	get_name_state(HttpServer::get_state(), buffer2);
	sprintf(buffer, "size=%4d, free=%4d, used=%4d, max=%4d, %s",
			(int)HttpServer::get_stack_size(),
			(int)HttpServer::get_free_stack(),
			(int)HttpServer::get_used_stack(),
			(int)HttpServer::get_max_stack(),
			buffer2);
	(*_result.json_out)["HttpServer"] = picojson::value(buffer);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTInfo::attach_done(void (*function)(worker_result*)) {
	__TRACE("attaching done callback\n");
	_req_done = callback(function);
}
