/*
 * MsgIdBuffer.h
 *
 *  Created on: 27. 10. 2016
 *      Author: Viktor
 */

#ifndef BYZANCE_MSGIDBUFFER_H_
#define BYZANCE_MSGIDBUFFER_H_

#include "mbed.h"
#include <string>
#include <deque>
#include "MqttBuffer.h"
#include "ByzanceLogger.h"


#define	MESSAGE_ID_BUFF_CAPACITY	((uint32_t) 4)
#define MESSAGE_ID_BUFF_DBG_EN		(0)


#if MESSAGE_ID_BUFF_DBG_EN
	#define			MESSAGE_ID_BUFF_DBG(...);  		ByzanceLogger::debug(__VA_ARGS__);
#else
	#define			MESSAGE_ID_BUFF_DBG(...);		{}
#endif


/** 
* \struct msg_id_struct
* \brief brief TO DO 
*
* Long description TO DO 
*/
struct msg_id_struct {
   string 		msg_id;
   uint8_t		cnt;
};

/** 
* \class MsgIdBuffer
* \brief brief TO DO 
*
* Long description TO DO 
*/
class MsgIdBuffer{
	public:
		
		/** Constructor 
		* 
		* \param
		*/
		MsgIdBuffer();
		~MsgIdBuffer() {};

		/** 
		*  
		* \param
		* \return
		*/
		uint32_t add(string & input_string);
		
		/** 
		*  
		* \param
		* \return
		*/
		uint32_t remove(string & input_string);
		
		/** 
		*  
		* \param
		* \return
		*/
		uint32_t state(string & input_string);


	protected:

		// interni vektor, do ktereho se ukladaji vsechna data
		deque<msg_id_struct>	_msg_list;

};


#endif /* BYZANCE_MSGIDBUFFER_H_ */
