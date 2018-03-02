/*
 * MqttBuffer.h
 *
 *  Created on: 5. 9. 2016
 *      Author: Ing. Martin Borýsek
 */

#ifndef MQTT_BUFFER_H_
#define MQTT_BUFFER_H_

#define MQTT_BUFFER_CAPACITY			((uint16_t)4)	// Max. pocet zprav v bufferu

#include "mbed.h"
#include <string>
#include <list>
#include "picojson.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "byzance_debug.h"

/** 
* \struct message_struct 
* \brief brief TO DO 
* 
* Long description TO DO
*/
struct message_struct {
   string topic;
   string payload;
};

typedef enum
{
	MQTT_BUFFER_OK			= 0x00, // vsechno v pohode
	MQTT_BUFFER_BUSY		= 0x01, // buffer neco processi a ma zamknuty zamecek
	MQTT_BUFFER_FULL		= 0x02, // buffer je plny
	MQTT_BUFFER_EMPTY		= 0x03, // buffer je prazdny
	MQTT_BUFFER_ERROR		= 0x04	// vseobecna chyba
} MqttBuffer_Err_TypeDef;

/**
* \class MqttBuffer
* \brief brief TO DO
*
* Long description TO DO 
*/
class MqttBuffer{

	public:
  
		MqttBuffer();
		~MqttBuffer() {};

		// zamknutí nebo odemknutí mqttbufferu,
		// aby bylo možné s ním pracovat bezpečně ve vláknu


		/** Lock buffer
         *
         * @param	none
         * @return	==0 for success
         * @return	!=0 for error
         */
		MqttBuffer_Err_TypeDef	lock(uint32_t ms = 1000);

		/** Lock buffer
         *
         * @param	none
         * @return ==0 for success
         * @return !=0 for error
         */
		MqttBuffer_Err_TypeDef	unlock();


        /** Get message
         *
         * @param	none
         * @return ==0 for success
         * @return !=0 for error
         */
		MqttBuffer_Err_TypeDef	add_message(message_struct* ms);

		/** Get message
         *
         * @param	none
         * @return ==0 for success
         * @return !=0 for error
         */
		MqttBuffer_Err_TypeDef	pick_up_message(message_struct *ms);

		/** Get message
         *
         * @param	none
         * @return ==0 for success
         * @return !=0 for error
         */
		MqttBuffer_Err_TypeDef  get_only_message(message_struct *ms);


        /**	Delete all messages in the list
         *
         * @param	none
         * @return	none
         */
		MqttBuffer_Err_TypeDef	clear();

		/** Get size of the list
         *
         * @param	none
         * @return	Number of allocated messages
         */
		MqttBuffer_Err_TypeDef	size(uint32_t *size);

	protected:

		// interni vektor, do ktereho se ukladaji vsechna data
		list<message_struct>	_msg_list;
		Mutex _mutex;
    
};


uint32_t get_json_msg_id(message_struct * msg_in, string * out_string);

#endif /* MQTT_BUFFER_H_ */
