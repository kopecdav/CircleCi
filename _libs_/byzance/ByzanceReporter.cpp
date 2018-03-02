#include "mbed.h"

#include "ByzanceReporter.h"

void ByzanceReporter::report(event_t event){

	// vyrobí JSON s povinnýma labelama
	picojson::object	json_object_outcomming;
	message_struct		msg;

	// in every case change to proper value!!!
	msg.topic.assign("report_out/");

	switch(event){
	case REPORT_EVENT_WELCOME:
		json_object_outcomming["device"] = picojson::value(TOSTRING_TARGET(__BUILD_TARGET__));
		msg.topic += "connected";
		break;
	default:
		return;
	}


	// serializuje a odešle do brokeru
	string str = picojson::value(json_object_outcomming).serialize();
	msg.payload = str;

	ByzanceClient::send_message(&msg);

}

void ByzanceReporter::report_state(bin_component_t comp, bin_state_t state){

	// vyrobí JSON s povinnýma labelama
	picojson::object	json_object_outcomming;
	message_struct		msg;
	char buffer[32];

	// in every case change to proper value!!!
	msg.topic.assign("report_out/state");

	// parse state to string
	switch(state){
	case BINSTRUCT_STATE_INVALID:
		strcpy(buffer, "invalid");
		break;
	case BINSTRUCT_STATE_VALID:
		strcpy(buffer, "valid");
		break;
	case BINSTRUCT_STATE_ERASING:
		strcpy(buffer, "erasing");
		break;
	case BINSTRUCT_STATE_ERASED:
		strcpy(buffer, "erased");
		break;
	}

	// create json
	switch(comp){
	case BIN_COMPONENT_BOOTLOADER:
		json_object_outcomming["bootloader"] = picojson::value(buffer);
		break;
	case BIN_COMPONENT_FIRMWARE:
		json_object_outcomming["firmware"] = picojson::value(buffer);
		break;
	case BIN_COMPONENT_BACKUP:
		json_object_outcomming["backup"] = picojson::value(buffer);
		break;
	case BIN_COMPONENT_BUFFER:
		json_object_outcomming["buffer"] = picojson::value(buffer);
		break;
	}

	// serializuje a odešle do brokeru
	string str = picojson::value(json_object_outcomming).serialize();
	msg.payload = str;

	ByzanceClient::send_message(&msg);

}
