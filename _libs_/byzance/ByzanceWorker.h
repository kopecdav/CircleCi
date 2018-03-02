#ifndef BYZANCE_WORKER_H
#define BYZANCE_WORKER_H

#include "mbed.h"
#include "byzance_debug.h"
#include "support_functions.h"
#include "MqttBuffer.h"
#include "ByzanceJson.h"
#include "ByzanceCore.h"
#include "struct_worker_result.h"

#include "ExtMem.h"
#include "extmem_config.h"
#include "ByzanceClient.h"
#include "MQTTInfo.h"
#include "MQTTSettings.h"
#include "MQTTCommands.h"

#include "byzance_config.h"
#include "MqttBuffer.h"

#include "picojson.h"

#include "error_codes.h"
#include "enum_workerstate.h"

#include "enum_infostate.h"
#include "enum_settingstate.h"
#include "ErrorResolver.h"

/**
* \class ByzanceWorker
* \brief brief TO DO 
*
* Description TO DO
*/
class ByzanceWorker{

	public:
		
		/**
		* init
		*
		* \return
		*/
		static ByzanceWorker* init();
		
		/**
		* init_itf
		*
		* \param
		* \param 
		* \return
		*/
		static void init_itf(NetworkInterface* itf);

		/**
		* process
		*
		* \param
		* \param 
		* \return
		*/
		char process(message_struct* msg_in);

		~ByzanceWorker(){
	        instanceFlag = false;
	    }

	protected:
		
		/**
		* send_response
		*
		* \param
		* \param 
		* \return
		*/
		static uint32_t	send_response(uint32_t rc, picojson::object* json, string* topic);

		/**
		* _create_outcomming_topic
		*
		* \param
		* \param 
		* \return
		*/
		static bool		_create_outcomming_topic(string& in, string& out);

		/**
		* _request_done
		*
		* \param
		* \param 
		* \return
		*/
		static void		_request_done(worker_result* result);

		// outcomming data
		static string 			_outcomming_topic;
		static picojson::object _outcomming_json;

		static NetworkInterface		*_itf;

	private:
		static ByzanceWorker *single;
		static bool instanceFlag;
		
	    static MsgIdBuffer*		_ids;
	    static ErrorResolver*	_resolver;
		
		ByzanceWorker() {};
		//~ByzanceWorker() {};
};

#endif /* BYZANCE_WORKER_H */
