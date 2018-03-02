#ifndef MQTTCOMMANDS_H
#define MQTTCOMMANDS_H

#include "mbed.h"

#include "IODA_general.h"
#include "ByzanceLogger.h"
#include "MQTTClient.h"
#include "extmem_config.h"

#include "error_codes.h"

// kvuli resetartu, ktery potrebuje vyblit odpoved driv nez se restartuje
#include "ByzanceClient.h"

#include "byzance_debug.h"

#include "picojson.h"
#include "base64.h"
#include <math.h> // round()

#include "struct_binary.h"
#include "enum_component.h"
#include "struct_worker_result.h"

#include "BinManager.h"
#include "enum_binstate.h"
#include "struct_mqtt_topic.h"

/** 
* \class MQTTCommands
* \brief brief TO DO 
* 
* Long description TO DO 
*/
class MQTTCommands {

public:

    /** Init
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
     * @param json_object_outcomming TODO doplnit
     * @param subtopic_incomming TODO doplnit
     *
     */
	static void		process(picojson::value& json_data_incomming, const char* subtopic_incomming);

	/** TODO add brief
     *
     * TODO add long description
     *
     * @param worker_result
     *
     */
	static void attach_done(void (*function)(worker_result*));

	/** TODO add brief
     *
     * TODO add long description
     *
     * @param worker_result
     *
     */
	template<typename T>
	static void attach_done(T *object, void (T::*member)(worker_result*));

	/*
	 * NEW
	 */
	static void		upload_start(picojson::value& json_data_incomming);
	static void		upload_end(picojson::value& json_data_incomming);
	static void		upload_data(picojson::value& json_data_incomming);
	static void		update_start(picojson::value& json_data_incomming);

	static void		system_restart(picojson::value& json_data_incomming);
	static void		system_ping(picojson::value& json_data_incomming);
	static void		system_blink(picojson::value& json_data_incomming);

protected:

	static worker_result		_result;

	static uint32_t		_bin_crc_calculated;
	static uint32_t		_bin_crc_received;
	static uint32_t		_part_index_expected;

	// callbacks
	static Callback<void(worker_result*)>	_req_done;

	static size_t 		_cmdcount;

private:
    
    MQTTCommands() {};
    ~MQTTCommands() {};
};

/*
 * LIST OF COMMANDS
 */
const struct_mqtt_topic cmd_topics[] = {
	{"upload/start", 	MQTTCommands::upload_start},
	{"upload/end",	 	MQTTCommands::upload_end},
	{"upload/data",	 	MQTTCommands::upload_data},
	{"update/start", 	MQTTCommands::update_start},
	{"system/restart",	MQTTCommands::system_restart},
	{"system/ping", 	MQTTCommands::system_ping},
	{"system/blink", 	MQTTCommands::system_blink},
};

#endif /* MQTTCOMMANDS_H */
