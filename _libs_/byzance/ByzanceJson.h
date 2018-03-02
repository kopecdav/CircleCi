#ifndef BYZANCE_JSON_H
#define BYZANCE_JSON_H

#include "mbed.h"
#include "ByzanceLogger.h"
#include "byzance_debug.h"
#include "byzance_config.h"
#include "MqttBuffer.h"
#include "picojson.h"
#include "error_codes.h"

/**
* \class ByzanceJson 
* \brief brief TO DO.  
* 
* Description TO DO 
*/
class ByzanceJson {

	public:
		
		/**
		* Validate 
		* \param
		* \param 
		* \return
		*/
		static uint32_t validate(message_struct *msg, picojson::value& json);
		
		/**
		* show 
		* \param
		* \param 
		* \return
		*/
		static void 	show(picojson::value& json);
		
		/**
		* show 
		* \param
		* \param 
		* \return
		*/
		static void 	show(picojson::object& json);

	protected:                              


	private:
		
		/**
		* TO DO  
		*/
		ByzanceJson() {};
		~ByzanceJson() {};
};

#endif /* BYZANCE_JSON_H */
