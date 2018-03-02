#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "mbed.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "support_functions.h"

#define CONNECTOR_MESSAGE_SIZE 512

// CRC for incomming messages is required
#define CONNECTOR_CRC_REQUIRED 1

/**
* \class Connector
* \brief brief TO DO
* 
* Description TO DO
*/
class Connector {

	public:

		/**
		* Connector 
		*
		* \param
		* \param 
		*/
		Connector(Serial *serial);

		/**
		* prefix
		*
		* \param
		* \param 
		* \return
		*/
		void prefix(char* prefix);

		/**
		* read 
		*
		* \param
		* \param 
		* \return
		*/
		int read(char* msg);

		/**
		* write
		*
		* \param
		* \param 
		* \return
		*/
		int write(const char* format, ...);

		/**
		* lock 
		*
		* \param
		* \param 
		* \return
		*/
		int lock();

		/**
		* unlock 
		*
		* \param
		* \param 
		* \return
		*/
		int unlock();

		/**
		* get_locked 
		*
		* \param
		* \param 
		* \return
		*/
		bool get_locked();

		/**
		* attach_incomming 
		*
		* \param
		* \param 
		*/
	    void attach_incomming(void (*function)(void));
	
	    template<typename T>
	    void attach_incomming(T *object, void (T::*member)(void)) {
	    	_msg_incomming.attach(object, member);
	    }

	protected:

		/**
		*  _received_char
		*
		* \param
		* \param 
		*/
		void	_received_char();

	private:

		Callback<void(void)>	_msg_incomming;

		Serial*	_serial;
		string	_msg;
		string	_msg_buffered;
		bool	_msg_valid;

		volatile bool	_busy;

		volatile bool	_locked;
  
};

#endif /* CONSOLE_H */
