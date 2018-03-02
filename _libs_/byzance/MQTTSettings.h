#ifndef MQTTSETTINGS_H
#define MQTTSETTINGS_H

#include "mbed.h"

#include "IODA_general.h"
#include "ByzanceLogger.h"
#include "MQTTClient.h"
#include "extmem_config.h"
#include "picojson.h"
#include "error_codes.h"
#include <stdlib.h> /* strtol */
#include "struct_mqtt.h"
#include "struct_wifi_credentials.h"
#include "struct_worker_result.h"
#include "Console.h"
#include "struct_mqtt_topic.h"

/**
* \class MQTTSettings 
* \brief brief TO DO 
*
* Long description TO DO 
*/
class MQTTSettings {

public:

    /** TODO add brief
     *
     * TODO add long description
     *
     * @param json_outcomming TODO doplnit
     *
     */
	static void		init(picojson::object* json_outcomming);

	/** TODO add brief
     *
     * TODO add long description
     *
     * @param json_data_incomming TODO doplnit
     * @param subtopic_incomming TODO doplnit
     *
     */
	static void		process(picojson::value& json_data_incomming, const char* subtopic_incomming);

	static void alias(picojson::value& json_data_incomming);
	static void datetime(picojson::value& json_data_incomming);
	static void timeoffset(picojson::value& json_data_incomming);
	static void timesync(picojson::value& json_data_incomming);
	static void autobackup(picojson::value& json_data_incomming);
	static void blreport(picojson::value& json_data_incomming);
	static void wdenable(picojson::value& json_data_incomming);
	static void wdtime(picojson::value& json_data_incomming);
	static void backuptime(picojson::value& json_data_incomming);
	static void lowpanbr(picojson::value& json_data_incomming);
	static void autojump(picojson::value& json_data_incomming);
	static void restartbl(picojson::value& json_data_incomming);
	static void normal_mqtt_connection(picojson::value& json_data_incomming);
	static void backup_mqtt_connection(picojson::value& json_data_incomming);
	static void mqtt_username(picojson::value& json_data_incomming);
	static void mqtt_password(picojson::value& json_data_incomming);
	static void console(picojson::value& json_data_incomming);
	static void webview(picojson::value& json_data_incomming);
	static void webport(picojson::value& json_data_incomming);
	static void netsource(picojson::value& json_data_incomming);

	/** TODO add brief
	 *
	 * TODO add long description
	 *
	 * @param worker_result TODO doplnit
	 *
	 */
	static void attach_done(void (*function)(worker_result*));

    /** TODO add brief
	 *
	 * TODO add long description
	 *
	 * @param worker_result TODO doplnit
	 *
	 */
	template<typename T>
	static void attach_done(T *object, void (T::*member)(worker_result*));

protected:

	static worker_result		_result;

	static Callback<void(worker_result*)> _req_done;

	static size_t 		_cmdcount;

private:
    
    MQTTSettings() {};
    ~MQTTSettings() {};
};

/*
 * LIST OF SETTINGS TOPICS
 */
const struct_mqtt_topic set_topics[] = {
	{"alias", 					MQTTSettings::alias},
	{"datetime",				MQTTSettings::datetime},
	{"timeoffset",				MQTTSettings::timeoffset},
	{"timesync", 				MQTTSettings::timesync},
	{"autobackup",				MQTTSettings::autobackup},
	{"blreport", 				MQTTSettings::blreport},
	{"wdenable", 				MQTTSettings::wdenable},
	{"wdtime", 					MQTTSettings::wdtime},
	{"backuptime", 				MQTTSettings::backuptime},
	{"lowpanbr", 				MQTTSettings::lowpanbr},
	{"autojump", 				MQTTSettings::autojump},
	{"restartbl", 				MQTTSettings::restartbl},
	{"normal_mqtt_connection", 	MQTTSettings::normal_mqtt_connection},
	{"backup_mqtt_connection", 	MQTTSettings::backup_mqtt_connection},
	{"mqtt_username", 			MQTTSettings::mqtt_username},
	{"mqtt_password", 			MQTTSettings::mqtt_password},
	{"console", 				MQTTSettings::console},
	{"webview", 				MQTTSettings::webview},
	{"webport", 				MQTTSettings::webport},
	{"netsource", 				MQTTSettings::netsource},
};

#endif /* MQTTSETTINGS_H */
