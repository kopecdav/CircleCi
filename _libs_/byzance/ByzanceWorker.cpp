#include "ByzanceWorker.h"

REGISTER_DEBUG(Worker);

// outcomming data
string 					ByzanceWorker::_outcomming_topic;
picojson::object		ByzanceWorker::_outcomming_json;

NetworkInterface*		ByzanceWorker::_itf;


bool 					ByzanceWorker::instanceFlag = false;
ByzanceWorker*			ByzanceWorker::single = NULL;

MsgIdBuffer*			ByzanceWorker::_ids;

ErrorResolver*			ByzanceWorker::_resolver;

ByzanceWorker * ByzanceWorker::init(){
	if(!instanceFlag){
		single = new ByzanceWorker();
		instanceFlag = true;

		MQTTCommands::init(&_outcomming_json);
		MQTTCommands::attach_done(&_request_done);

		MQTTSettings::init(&_outcomming_json);
		MQTTSettings::attach_done(&_request_done);

		MQTTInfo::init(&_outcomming_json);
		MQTTInfo::attach_done(&_request_done);

		ByzanceWorker::_ids = new MsgIdBuffer();

		ByzanceWorker::_resolver = new ErrorResolver();

	}
	return single;
}

void ByzanceWorker::init_itf(NetworkInterface* itf){
	_itf	= itf;
	MQTTInfo::init_itf(_itf);
}

/*
 * ByzanceWorker třídí příchozí zprávy a zpracovává je,
 * podle toho, co v těch zprávách je a do jakých topiců patří
 *
 * return 0 - vše OK
 * return číslo - chyba
 * todo: dopsat lepší dokumentaci :)
 */
char ByzanceWorker::process(message_struct* msg_in){

	std::size_t pos;
	_outcomming_json.clear();
	_outcomming_topic.clear();

	// empty topic error
	if( msg_in->topic.length() == 0){
		__ERROR("empty topic\n", msg_in->topic.length());
		send_response(ERROR_CODE_UNKNOWN_TOPIC, &_outcomming_json, &_outcomming_topic);
		return 1;
	}

	// empty payload error
	if(msg_in->payload.length() == 0){
		__ERROR("empty payload\n", msg_in->payload.length());
		send_response(ERROR_CODE_UNKNOWN_TOPIC, &_outcomming_json, &_outcomming_topic);
		return 1;
	}

	/************************************************************
	 *
	 *   VOLÁNÍ REGISTROVANÝCH CALLBACKŮ
	 *
	 ************************************************************/

	/************************************************************
	 *   D_IN - vyvolá se interrupt v main
	 ************************************************************/

	constexpr uint32_t din_len = strlen("d_in/");
	pos=msg_in->topic.find("d_in/");
	if (pos==0){

		__DEBUG("pos=%d\n", pos);

		string digital_pin_name = msg_in->topic.substr(din_len, msg_in->topic.find("/", din_len) - din_len);
		__DEBUG("name =%s\n", digital_pin_name.c_str());


		char digital_pin_value = msg_in->payload.at(0);
		__DEBUG("firstPayloadChar=%c\n", digital_pin_value);

		bool val = !(digital_pin_value == '0' || digital_pin_value == 'f' || digital_pin_value == 'F');

		for(unsigned int i=0; i<ByzanceCore::_digitalInputs->size(); ++i) {
			if (strcmp((*ByzanceCore::_digitalInputs)[i].get_name(), digital_pin_name.c_str()) == 0) {
				(*ByzanceCore::_digitalInputs)[i].call(val);
			}
		}

		return 0;
	} else {
		__TRACE("neni to din\n");
	}

	/************************************************************
	 *   A_IN - vyvolá se interrupt v main
	 ************************************************************/
	constexpr uint32_t ain_len = strlen("a_in/");
	pos=msg_in->topic.find("a_in/");
	if (pos==0){

		__DEBUG("pos=%d\n", pos);

		string name = msg_in->topic.substr(ain_len, msg_in->topic.find("/", ain_len) - ain_len);
		__DEBUG("name =%s\n", name.c_str());

		float val = atof(msg_in->payload.c_str());
		__DEBUG("val=%f\n", val);

		for(unsigned int i=0; i<ByzanceCore::_analogInputs->size(); ++i) {
			if (strcmp((*ByzanceCore::_analogInputs)[i].getName(), name.c_str()) == 0) {
				(*ByzanceCore::_analogInputs)[i].call(val);
			}
		}

		return 0;
	} else {
		__TRACE("neni to ain\n");
	}

	/************************************************************
	 *   M_IN - vyvolá se interrupt v main
	 ************************************************************/
	constexpr uint32_t min_len = strlen("m_in/");
	pos=msg_in->topic.find("m_in/");
	if (pos==0){
		//string name = msg_in->topic.substr(msg_in->topic.rfind("/") + 1);
		string name = msg_in->topic.substr(min_len, msg_in->topic.find("/", min_len) - min_len);

		__DEBUG("name=%s\n", name.c_str());

		for(unsigned int i=0; i<ByzanceCore::_messageInputs->size(); ++i) {
			if (strcmp((*ByzanceCore::_messageInputs)[i].get_name(), name.c_str()) == 0) {
				ByzanceParser bp((char*)msg_in->payload.data(), msg_in->payload.length());

				int rc = bp.validate();
				if(rc){
					__ERROR("validate fail=%d\n", rc);
				} else {
					__ERROR("calling callback\n");
					(*ByzanceCore::_messageInputs)[i].call(&bp);
				}
			}
		}

		return 0;
	} else {
		__TRACE("neni to min\n");
	}

	/************************************************************
	 *
	 *   NÁSLEDUJÍCÍ ZPRÁVY MAJÍ V SOBĚ JSON
	 *
	 ************************************************************/

	uint32_t rc;

	static string subtopic;
	static picojson::value 	json_data_incomming;

	string mid;

	// do odchozího topicu se zkopíruje příchozí
	// *_in/* se nahradí za *_out/*

	_outcomming_json.clear();
	mid.clear();

	/*
	 * VALIDATION OF JSON AND MSGID
	 * IF INVALID, SEND ERROR TO HOMER
	 */
	rc = ByzanceJson::validate(msg_in, json_data_incomming);
	if(rc){

		__ERROR("invalid JSON, rc=%d\n", rc);

		// nahradím _in/ za _out/
		_create_outcomming_topic(msg_in->topic, _outcomming_topic);

		// if msg id is present but there is some other error remaining, add msg id to response
		if(rc!=ERROR_CODE_JSON_MSGID_MISSING){

			mid.assign(json_data_incomming.get("mid").get<string>());
			_outcomming_json["mid"] = picojson::value(mid.c_str());
		}

		// nahlásí zrovna chybu
		send_response(rc, &_outcomming_json, &_outcomming_topic);

		return 1;
	};

	__INFO("json ok\n");

	// nahradím _in/ za _out/
	_create_outcomming_topic(msg_in->topic, _outcomming_topic);

	// mid presence is guaranteed after validation
	mid.assign(json_data_incomming.get("mid").get<string>());
	_outcomming_json["mid"] = picojson::value(mid.c_str());

	// rc 0 -> zprava v bufferu neexistuje
	// rc 1 -> zprava existuje a nebude se znova pridavat
	rc = _ids->add(mid);
	if(rc){

		__WARNING("duplicitni mid, zahazuju\n");
		return 1;

	} else {
		__DEBUG("add state = %d, mid=%s\n", rc, mid.c_str());
	}

	/************************************************************
	 *   COMMAND_IN - provede se command
	 ************************************************************/
	pos=msg_in->topic.find("command_in/");
	if (pos==0){

		// vyrobí se subtopic, do kterého se usekne z topicu command_in/
		subtopic = msg_in->topic.substr(pos + strlen("command_in/"));

		MQTTCommands::process(json_data_incomming, subtopic.c_str());

	}

	/************************************************************
	 *   SETTINGS_IN - provede se settings
	 ************************************************************/
	pos=msg_in->topic.find("settings_in/");
	if (pos==0){

		// vyrobí se subtopic, do kterého se usekne z topicu command_in/
		subtopic = msg_in->topic.substr(pos + strlen("settings_in/"));

		MQTTSettings::process(json_data_incomming, subtopic.c_str());

	}

	/************************************************************
	 *   INFO_IN - provede se info
	 ************************************************************/
	pos=msg_in->topic.find("info_in/");
	if (pos==0){

		// vyrobí se subtopic, do kterého se usekne z topicu command_in/
		subtopic = msg_in->topic.substr(pos + strlen("info_in/"));

		MQTTInfo::process(json_data_incomming, subtopic.c_str());

	}

	return 1;
}

uint32_t ByzanceWorker::send_response(uint32_t rc, picojson::object* json, string* topic){

	static message_struct msg_out;

	/*
	 * CREATE RESPONSE JSON
	 */
	if(!rc){
		(*json)["status"] = picojson::value("ok");
	} else {

		char err_buff[64];
		_resolver->get_name(rc, err_buff);

		__ERROR("command error: %s (code=%d)\n", err_buff, rc);
		(*json)["status"] = picojson::value("error");

		// todo: vložit název chybového kódu
		(*json)["error"] = picojson::value(err_buff);
		(*json)["error_code"] = picojson::value((double)rc);
	}

	/*
	 * SERIALIZE RESPONSE
	 */
	msg_out.payload.assign(picojson::value(*json).serialize(false));

	/*
	 * SEND RESPONSE
	 */
	if(!msg_out.payload.empty()){
		msg_out.topic = *topic;

		__INFO("topic_out=%s\n", msg_out.topic.c_str());

		ByzanceClient::send_message(&msg_out);

	} else {
		__ERROR("response serialize error\n");
	}

	return 0;
}

/*
 * @return true		- proběhlo to OK
 * @return false	- neproběhlo to OK
 */
bool ByzanceWorker::_create_outcomming_topic(string& in, string& out){

	std::size_t pos;

	out.assign(in);

	pos = out.find("_in/");
	if (pos != string::npos ) {
		out.replace(pos, 4, "_out/" );   // 4 = length("_in/)
		return true;
	}

	return false;

}

void ByzanceWorker::_request_done(worker_result* result){

	// tady by se měl zkopírovat příchozí do odchozího msgid
	// a in topic do out topicu

	ByzanceWorker::send_response(result->rc, result->json_out, &_outcomming_topic);
}

