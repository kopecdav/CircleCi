#ifndef MQTTINFO_H
#define MQTTINFO_H

#include "mbed.h"
#include "ByzanceCore.h"

#include "ExtMem.h"
#include "ByzanceLogger.h"
#include "extmem_config.h"
#include "picojson.h"
#include "error_codes.h"
#include "EthernetInterface.h"
#include "struct_binary.h"
#include "struct_mqtt.h"
#include "struct_wifi_credentials.h"
#include "struct_worker_result.h"
#include "support_functions.h"
#include "Console.h"
#include "BinManager.h"
#include "Configurator.h"
#include "MqttManager.h"
#include "struct_mqtt_topic.h"

/** 
* \class MQTTInfo 
* \brief brief TO DO 
*
* Long description TO DO 
*/
class MQTTInfo {

public:

    /** Init
     *
     * TODO add long description
     *
     * @param json_outcomming TODO doplnit
     */
	static void		init(picojson::object* json_outcomming);

	static void		init_itf(NetworkInterface* itf);

	/** Process
     *
     * TODO add long description
     *
     * @param json_data_incomming TODO doplnit
     * @param subtopic_incomming TODO doplnit
     */
	static void		process(picojson::value& json_data_incomming, const char* subtopic_incomming);

	static void	target(picojson::value& json_data_incomming);
	static void	cpuload(picojson::value& json_data_incomming);
	static void	alias(picojson::value& json_data_incomming);
	static void	blreport(picojson::value& json_data_incomming);
	static void	wdenable(picojson::value& json_data_incomming);
	static void	wdtime(picojson::value& json_data_incomming);
	static void	backuptime(picojson::value& json_data_incomming);
	static void	lowpanbr(picojson::value& json_data_incomming);
	static void	autojump(picojson::value& json_data_incomming);
	static void	restartbl(picojson::value& json_data_incomming);
	static void	version(picojson::value& json_data_incomming);
	static void	mac(picojson::value& json_data_incomming);
	static void	ip(picojson::value& json_data_incomming);
	static void	memsize(picojson::value& json_data_incomming);
	static void	datetime(picojson::value& json_data_incomming);
	static void	timeoffset(picojson::value& json_data_incomming);
	static void	timesync(picojson::value& json_data_incomming);
	static void	uptime(picojson::value& json_data_incomming);
	static void	autobackup(picojson::value& json_data_incomming);
	static void	state(picojson::value& json_data_incomming);
	static void	normal_mqtt_connection(picojson::value& json_data_incomming);
	static void	backup_mqtt_connection(picojson::value& json_data_incomming);
	static void	console(picojson::value& json_data_incomming);
	static void	webview(picojson::value& json_data_incomming);
	static void	webport(picojson::value& json_data_incomming);
	static void	netsource(picojson::value& json_data_incomming);
	static void	revision(picojson::value& json_data_incomming);
	static void	threads(picojson::value& json_data_incomming);

	/** TODO add brief
     *
     * TODO add long description
     *
     * @param worker_result TODO doplnit
     */
	static void 	attach_done(void (*function)(worker_result*));

	/** TODO add brief
     *
     * TODO add long description
     *
     * @param worker_result TODO doplnit
     */
	template<typename T>
	static void 	attach_done(T *object, void (T::*member)(worker_result*));
   
protected:

	static NetworkInterface*	_itf;

	static worker_result		_result;

	static Callback<void(worker_result*)>	_req_done;

	static size_t 		_cmdcount;

private:
    
    MQTTInfo() {};
    ~MQTTInfo() {};
};

/*
 * LIST OF INFO TOPICS
 */
const struct_mqtt_topic inf_topics[] = {
	{"target", 					MQTTInfo::target},
	{"cpuload",	 				MQTTInfo::cpuload},
	{"alias",	 				MQTTInfo::alias},
	{"blreport", 				MQTTInfo::blreport},
	{"wdenable",				MQTTInfo::wdenable},
	{"wdtime", 					MQTTInfo::wdtime},
	{"backuptime", 				MQTTInfo::backuptime},
	{"lowpanbr", 				MQTTInfo::lowpanbr},
	{"autojump", 				MQTTInfo::autojump},
	{"restartbl", 				MQTTInfo::restartbl},
	{"version",					MQTTInfo::version},
	{"mac",						MQTTInfo::mac},
	{"ip", 						MQTTInfo::ip},
	{"memsize",					MQTTInfo::memsize},
	{"datetime", 				MQTTInfo::datetime},
	{"timeoffset", 				MQTTInfo::timeoffset},
	{"timesync", 				MQTTInfo::timesync},
	{"uptime", 					MQTTInfo::uptime},
	{"autobackup", 				MQTTInfo::autobackup},
	{"state", 					MQTTInfo::state},
	{"normal_mqtt_connection", 	MQTTInfo::normal_mqtt_connection},
	{"backup_mqtt_connection", 	MQTTInfo::backup_mqtt_connection},
	{"console",					MQTTInfo::console},
	{"webview",					MQTTInfo::webview},
	{"webport",					MQTTInfo::webport},
	{"netsource", 				MQTTInfo::netsource},
	{"revision", 				MQTTInfo::revision},
	{"threads",					MQTTInfo::threads},

};

#endif /* MQTTINFO_H */
