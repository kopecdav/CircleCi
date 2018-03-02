#include "MQTTSettings.h"

REGISTER_DEBUG(MQTTSettings);

worker_result		MQTTSettings::_result;

Callback<void(worker_result*)>	MQTTSettings::_req_done;

size_t 		MQTTSettings::_cmdcount;

void MQTTSettings::init(picojson::object* json_outcomming){
	_result.json_out = json_outcomming;

	// https://stackoverflow.com/questions/1898657/result-of-sizeof-on-array-of-structs-in-c
	_cmdcount = sizeof set_topics / sizeof set_topics[0];
}

void MQTTSettings::process(picojson::value& json_data_incomming, const char* subtopic_incomming){

	char* poscmd;

	for(uint32_t i = 0; i<_cmdcount; i++){

		// find requested command in command set
		poscmd = strstr(subtopic_incomming, set_topics[i].name);

		// doplnit druhou část
		if(poscmd==NULL){
			continue;
		}

		// restart can match restartbl, so check whole lengths
		if(strlen(poscmd)!=strlen(set_topics[i].name)){
			continue;
		}

		__INFO("topic found -> %s\n", set_topics[i].name);

		set_topics[i].fp(json_data_incomming);

		return;
	}

	__ERROR("unknown topic\n");

	// not found
	_result.rc = ERROR_CODE_UNKNOWN_TOPIC;
	_req_done(&_result);
	return;

}

void MQTTSettings::alias(picojson::value& json_data_incomming){

	__INFO("alias\n");

	char buff[64];

	strcpy(buff, json_data_incomming.get("value").get<string>().c_str());

	int rc = Configurator::set_alias(buff);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	// label "value" is copied to response
	(*_result.json_out)["value"] = picojson::value(json_data_incomming.get("value").get<string>());
	__DEBUG(" alias = %s\n", buff);
	_req_done(&_result);
	return;

}

void MQTTSettings::datetime(picojson::value& json_data_incomming){

	__INFO("datetime\n");

	uint32_t tmp32 = 0;
	int rc = 0;

	rc = Configurator::get_timesync(&tmp32);

	if((!rc)&&(tmp32)){
		time_t timestamp;
		timestamp = (uint32_t)(round(json_data_incomming.get("value").get<double>()));
		set_time(timestamp);
		__DEBUG(" time is updated to %u\n",(unsigned int)timestamp);
		_result.rc = ERROR_CODE_OK;

	} else {
		_result.rc = ERROR_CODE_SETTINGS_TIME_REFUSED;
	}

	// label "value" is copied to response
	(*_result.json_out)["value"] = picojson::value(json_data_incomming.get("value").get<double>());

	_req_done(&_result);
	return;

}

void MQTTSettings::timeoffset(picojson::value& json_data_incomming){

	__INFO("timeoffset\n");

	int rc = 0;

	// must be signed
	int tmp32 = 0;

	tmp32 = (int)json_data_incomming.get("value").get<double>();
	__ERROR("rcvd timeoffset %d\n", tmp32);

	rc = Configurator::set_timeoffset(tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value((double)tmp32);
	__INFO("timeoffset = %d\n", (unsigned int)tmp32);
	_req_done(&_result);
	return;

}

void MQTTSettings::timesync(picojson::value& json_data_incomming){

	__INFO("timesync\n");
	bool val = json_data_incomming.get("value").get<bool>();

	int rc = Configurator::set_timesync(val);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value(val);
	__DEBUG(" timesync is %d\n", val);
	_req_done(&_result);
	return;

}

void MQTTSettings::autobackup(picojson::value& json_data_incomming){

	__INFO("autobackup\n");
	bool val = json_data_incomming.get("value").get<bool>();

	int rc = Configurator::set_autobackup(val);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value(val);
	__DEBUG(" autobackup is %d\n", val);
	_req_done(&_result);
	return;

}

void MQTTSettings::blreport(picojson::value& json_data_incomming){

	__INFO("blreport\n");
	bool val = json_data_incomming.get("value").get<bool>();

	int rc = Configurator::set_blreport(val);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value(val);
	__DEBUG(" blreport is %d\n", val);
	_req_done(&_result);
	return;

}

void MQTTSettings::wdenable(picojson::value& json_data_incomming){

	__INFO("wdenable\n");
	bool val = json_data_incomming.get("value").get<bool>();

	int rc = Configurator::set_wdenable(val);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value(val);
	__DEBUG(" wdenable is %d\n", val);
	_req_done(&_result);
	return;

}

void MQTTSettings::wdtime(picojson::value& json_data_incomming){

	__INFO("wdtime\n");

	int rc = 0;
	uint32_t tmp32 = 0;

	tmp32 = (unsigned int)json_data_incomming.get("value").get<double>();

	rc = Configurator::set_wdtime(tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value((double)tmp32);
	__INFO("wdtime = %u\n", (unsigned int)tmp32);
	_req_done(&_result);
	return;

}

void MQTTSettings::backuptime(picojson::value& json_data_incomming){

	/*
	 * Dummy
	 */
	__INFO("backuptime\n");

	int rc = 0;
	uint32_t tmp32 = 0;

	tmp32 = (unsigned int)json_data_incomming.get("value").get<double>();

	(*_result.json_out)["value"] = picojson::value((double)tmp32);
	__INFO("backuptime = %u\n", (unsigned int)tmp32);
	_req_done(&_result);
	return;


}

void MQTTSettings::lowpanbr(picojson::value& json_data_incomming){

	__INFO("lowpanbr\n");
	bool val = json_data_incomming.get("value").get<bool>();

	int rc = Configurator::set_lowpanbr(val);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value(val);
	__DEBUG(" lowpanbr is %d\n", val);
	_req_done(&_result);
	return;

}

void MQTTSettings::autojump(picojson::value& json_data_incomming){

	__INFO("autojump\n");

	int rc = 0;
	uint32_t tmp32 = 0;

	tmp32 = (unsigned int)json_data_incomming.get("value").get<double>();

	rc = Configurator::set_autojump(tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value((double)tmp32);
	__INFO("autojump = %u\n", (unsigned int)tmp32);
	_req_done(&_result);
	return;

}

void MQTTSettings::restartbl(picojson::value& json_data_incomming){

	__INFO("restartbl\n");
	bool val = json_data_incomming.get("value").get<bool>();

	int rc = Configurator::set_restartbl(val);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else {
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["restartbl"] = picojson::value(val);
	__DEBUG("restartbl is %d\n", val);
	_req_done(&_result);
	return;


}

void MQTTSettings::normal_mqtt_connection(picojson::value& json_data_incomming){

	__INFO("normal_mqtt_connection\n");

	string buffer;
	buffer = json_data_incomming.get("value").get<string>();

	std::size_t pos;
	pos=buffer.find(":");
	if (pos!=std::string::npos){

		mqtt_connection tmp_mqtt;

		// všechno před dvojtečkou je hostname
		strcpy(tmp_mqtt.hostname, buffer.substr(0, pos).c_str());
		// všechno za dvojtečkou je port
		// Stack Overflow: Any time you think about atoi, use strtol instead
		tmp_mqtt.port		= strtoul((char*)buffer.substr(pos+1).c_str(), NULL, 10);

		MqttManager::set_hostname(MQTT_NORMAL, tmp_mqtt.hostname);
		MqttManager::set_port(MQTT_NORMAL, tmp_mqtt.port);

		__INFO("normal_mqtt_connection: hostname:port = %s:%u\n", tmp_mqtt.hostname, tmp_mqtt.port);

		(*_result.json_out)["value"] = picojson::value(buffer);

		_result.rc = ERROR_CODE_OK;
		_req_done(&_result);
		return;

	} else {

		_result.rc = ERROR_CODE_GENERAL_ERROR;
		_req_done(&_result);
		return;
	}

}

void MQTTSettings::backup_mqtt_connection(picojson::value& json_data_incomming){

	__INFO("backup_mqtt_connection\n");

	string buffer;
	buffer = json_data_incomming.get("value").get<string>();

	std::size_t pos;
	pos=buffer.find(":");
	if (pos!=std::string::npos){

		mqtt_connection tmp_mqtt;

		// všechno před dvojtečkou je hostname
		strcpy(tmp_mqtt.hostname, buffer.substr(0, pos).c_str());
		// všechno za dvojtečkou je port
		// Stack Overflow: Any time you think about atoi, use strtol instead
		tmp_mqtt.port		= strtoul((char*)buffer.substr(pos+1).c_str(), NULL, 10);

		MqttManager::set_hostname(MQTT_BACKUP, tmp_mqtt.hostname);
		MqttManager::set_port(MQTT_BACKUP, tmp_mqtt.port);

		__INFO("backup_mqtt_connection: hostname:port = %s:%u\n", tmp_mqtt.hostname, tmp_mqtt.port);

		(*_result.json_out)["value"] = picojson::value(buffer);

		_result.rc = ERROR_CODE_OK;
		_req_done(&_result);
		return;

	} else {

		_result.rc = ERROR_CODE_GENERAL_ERROR;
		_req_done(&_result);
		return;
	}

}

void MQTTSettings::mqtt_username(picojson::value& json_data_incomming){

	__INFO("mqtt_username\n");

	char buff[64];

	strcpy(buff, json_data_incomming.get("value").get<string>().c_str());

	int rc = MqttManager::set_username(buff);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	// label "value" is copied to response
	(*_result.json_out)["value"] = picojson::value(json_data_incomming.get("value").get<string>());
	__DEBUG(" mqtt_username = %s\n", buff);
	_req_done(&_result);
	return;

}

void MQTTSettings::mqtt_password(picojson::value& json_data_incomming){

	__INFO("mqtt_password\n");

	char buff[64];

	strcpy(buff, json_data_incomming.get("value").get<string>().c_str());

	int rc = MqttManager::set_password(buff);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	// label "value" is copied to response
	(*_result.json_out)["value"] = picojson::value(json_data_incomming.get("value").get<string>());
	__DEBUG(" mqtt_password = %s\n", buff);
	_req_done(&_result);
	return;

}

void MQTTSettings::console(picojson::value& json_data_incomming){

	__INFO("console\n");

	bool en;
	en = json_data_incomming.get("value").get<bool>();
	Console::enable(en);

	(*_result.json_out)["value"] = picojson::value(en);

	if(en){
		__INFO("console enabled\n");
	} else {
		__INFO("console disabled\n");
	}

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTSettings::webview(picojson::value& json_data_incomming){

	int rc = 0;
	uint32_t tmp32 = 0;

	tmp32 = json_data_incomming.get("value").get<bool>();

	rc = Configurator::set_webview(tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value((bool)tmp32);
	__INFO("webview = %d\n",tmp32);
	_req_done(&_result);
	return;


}

void MQTTSettings::webport(picojson::value& json_data_incomming){

	__INFO("webport\n");

	int rc = 0;
	uint32_t tmp32 = 0;

	tmp32 = (unsigned int)json_data_incomming.get("value").get<double>();

	rc = Configurator::set_webport(tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value((double)tmp32);
	__INFO("webport = %u\n", (unsigned int)tmp32);
	_req_done(&_result);
	return;

}

void MQTTSettings::netsource(picojson::value& json_data_incomming){

	__INFO("netsource\n");

	int rc = 0;
	uint32_t tmp32 = 0;

	char buff[64];

	strcpy(buff, json_data_incomming.get("value").get<string>().c_str());

	if(strcmp(buff, "disabled")==0){

		tmp32 = NETSOURCE_DISABLED;
		__INFO("setting netsource = disabled\n");

	} else if(strcmp(buff, "ethernet")==0){

		tmp32 = NETSOURCE_ETHERNET;
		__INFO("setting netsource = ethernet\n");

	} else if(strcmp(buff, "wifi")==0){

		tmp32 = NETSOURCE_WIFI;
		__INFO("setting netsource = wifi\n");

	} else if(strcmp(buff, "6lowpan")==0){

		tmp32 = NETSOURCE_6LOWPAN;
		__INFO("setting netsource = 6lowpan\n");

	}

	rc = Configurator::set_netsource(tmp32);
	if(rc){
		_result.rc = ERROR_CODE_LOCK_ERROR;
	}else{
		_result.rc = ERROR_CODE_OK;
	}

	(*_result.json_out)["value"] = picojson::value(buff);

	_req_done(&_result);
	return;

}

void MQTTSettings::attach_done(void (*function)(worker_result*)) {
	__TRACE("attaching done callback\n");
	_req_done = callback(function);
}
