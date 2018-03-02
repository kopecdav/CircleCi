/*
 * DeviceList.h
 *
 *  Created on: 11. 7. 2016
 *      Author: Viktor, Martin
 */

#include "MqttBuffer.h"
#include "ByzanceJson.h"

REGISTER_DEBUG(MqttBuffer);

MqttBuffer::MqttBuffer(){
	_msg_list.clear();
}

// todo: destruktor!!!

/**
 * @brief	Lock buffer
 * @param	none
 * @retval	==0 for success
 * 			!=0 for error
 */
MqttBuffer_Err_TypeDef MqttBuffer::lock(uint32_t ms){
	osStatus status;
	status = _mutex.lock(ms);
	if(status!=osOK){
		return MQTT_BUFFER_BUSY;
	}

	return MQTT_BUFFER_OK;
}

/**
 * @brief	Lock buffer
 * @param	none
 * @retval	==0 for success
 * 			!=0 for error
 */
MqttBuffer_Err_TypeDef  MqttBuffer::unlock(){

	osStatus status;
	status = _mutex.unlock();
	if(status!=osOK){
		return MQTT_BUFFER_BUSY;
	}

	return MQTT_BUFFER_OK;
}

/**
 * @brief	Get message
 * @param	none
 * @retval	==0 for success
 * 			!=0 for error
 */
MqttBuffer_Err_TypeDef  MqttBuffer::add_message(message_struct* ms){
	// funkce size se lockne sama
	if(_msg_list.size() >= MQTT_BUFFER_CAPACITY){
		return MQTT_BUFFER_FULL;
	}

	// todo: throws exception on failure
	_msg_list.push_back(*ms);

	return MQTT_BUFFER_OK;
}

/**
 * @brief	Get message
 * @param	none
 * @retval	==0 for success
 * 			!=0 for error
 */
MqttBuffer_Err_TypeDef  MqttBuffer::pick_up_message(message_struct *ms){
	if(_msg_list.size()){
		// vyčte první prvek
		*ms = _msg_list.front();
		// smaže první prvek
		// todo: throws exception on failure
		_msg_list.pop_front();
	} else {
		return MQTT_BUFFER_EMPTY;
	}
	return MQTT_BUFFER_OK;
}


/**
 * @brief	Get message
 * @param	none
 * @retval	==0 for success
 * 			!=0 for error
 */
MqttBuffer_Err_TypeDef  MqttBuffer::get_only_message(message_struct *ms){
	if(_msg_list.size()){
		// vyčte první prvek
		*ms = _msg_list.front();
	} else {
		return MQTT_BUFFER_EMPTY;
	}
	return MQTT_BUFFER_OK;
}

/**
 * @brief	Get size of the list
 * @param	none
 * @retval	Number of allocated messages
 */
MqttBuffer_Err_TypeDef MqttBuffer::size(uint32_t *size){
	*size = _msg_list.size();
	return MQTT_BUFFER_OK;
}

/**
 * @brief	Delete all messages in the list
 * @param	none
 * @retval	none
 */
MqttBuffer_Err_TypeDef MqttBuffer::clear(){
	_msg_list.clear();
	return MQTT_BUFFER_OK;
}


/**
 * Overi, jestli dana struktura je validni JSON a vrati jeho mid.
 * @return 0		- OK
 * @return number	- error
 */
uint32_t get_json_msg_id(message_struct * msg_in, string * out_string){
		picojson::value		json_data_temp;

		// Neni to validni JSON
		if(ByzanceJson::validate(msg_in, json_data_temp) != ERROR_CODE_OK){
			return 1;
		}

		*out_string = json_data_temp.get("mid").get<string>();
		return 0;
}
