#include "ByzanceJson.h"

REGISTER_DEBUG(json);

/**
 * @brief	Validation of JSON
 * @param	Pointer to message_struct structure
 * @param	json; if validation is succesfull, json::value is created
 * @retval	== 0 - OK
 * 			!= 0 - ERROR_CODE
 */
uint32_t ByzanceJson::validate(message_struct *msg, picojson::value& json){

	// zvaliduju příchozí data, jestli jsou JSON libovolného typu
	string err = picojson::parse(json, msg->payload);
	// validace proběhla bez problémů, je to JSON
	if(err.empty()){

		// příchozí JSON je typu OBJEKT
		if (json.is<picojson::object>()) {

			// zkontroluju, jestli je přítomen povinný label mid
			if(json.contains("mid")){
				return ERROR_CODE_OK;

			// incomming json doesn't contain label mid
			} else {
				return ERROR_CODE_JSON_MSGID_MISSING;
			}

		// incomming json isn't object
		} else {
			return ERROR_CODE_JSON_NOT_OBJECT;
		}

	// incomming json isn't valid
	} else {
		return ERROR_CODE_JSON_INVALID;
	}

}

/**
 * @brief	Show JSON
 * @param	picojson::value object
 * @retval	none
 */
void ByzanceJson::show(picojson::value& json){
	__DEBUG("*** JSON VALUE *********************\n");
	const picojson::object& o = json.get<picojson::object>();
	for (picojson::object::const_iterator i = o.begin(); i != o.end(); ++i) {
		__DEBUG("* %s - %s\n", i->first.c_str(), i->second.to_str().c_str());
	}
	__DEBUG("************************************\n");
}

/**
 * @brief	Show JSON
 * @param	picojson::object object
 * @retval	none
 */
void ByzanceJson::show(picojson::object& json){
	__DEBUG("*** JSON OBJECT ********************\n");
	for (picojson::object::const_iterator i = json.begin(); i != json.end(); ++i) {
		__DEBUG("* %s - %s\n", i->first.c_str(), i->second.to_str().c_str());
	}
	__DEBUG("************************************\n");
}
