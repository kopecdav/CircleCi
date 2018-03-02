#ifndef CONSOLE_H
#define CONSOLE_H

#include "mbed.h"

#include "ByzanceClient.h"
#include "ByzanceLogger.h"
#include "picojson.h"
#include "MqttBuffer.h"

#ifndef CONSOLE_DEBUGGING_LEVEL
#define CONSOLE_DEBUGGING_LEVEL		DEBUG_LEVEL_NONE
#endif

#if CONSOLE_DEBUGGING
	#if CONSOLE_DEBUGGING_LEVEL >= DEBUG_LEVEL_LOG
		#define CONSOLE_LOG(...);		ByzanceLogger::log(__VA_ARGS__);
	#else
		#define CONSOLE_LOG(...);		{}
	#endif

	#if CONSOLE_DEBUGGING_LEVEL >= DEBUG_LEVEL_ERROR
		#define CONSOLE_ERROR(...);		ByzanceLogger::error(__VA_ARGS__);
	#else
		#define CONSOLE_ERROR(...);		{}
	#endif

	#if CONSOLE_DEBUGGING_LEVEL >= DEBUG_LEVEL_WARNING
		#define CONSOLE_WARNING(...);	ByzanceLogger::warning(__VA_ARGS__);
	#else
		#define CONSOLE_WARNING(...);	{}
	#endif

	#if CONSOLE_DEBUGGING_LEVEL >= DEBUG_LEVEL_INFO
		#define CONSOLE_INFO(...);		ByzanceLogger::info(__VA_ARGS__);
	#else
		#define CONSOLE_INFO(...);	{}
	#endif

	#if CONSOLE_DEBUGGING_LEVEL >= DEBUG_LEVEL_DEBUG
		#define CONSOLE_DEBUG(...);		ByzanceLogger::debug(__VA_ARGS__);
	#else
		#define CONSOLE_DEBUG(...);		{}
	#endif

	#if CONSOLE_DEBUGGING_LEVEL >= DEBUG_LEVEL_TRACE
		#define CONSOLE_TRACE(...);		ByzanceLogger::trace(__VA_ARGS__);
	#else
		#define CONSOLE_TRACE(...);		{}
	#endif

#else
	#define CONSOLE_LOG(...);		{}
	#define CONSOLE_ERROR(...);		{}
	#define CONSOLE_WARNING(...);	{}
	#define CONSOLE_INFO(...);		{}
	#define CONSOLE_DEBUG(...);		{}
	#define CONSOLE_TRACE(...);		{}
#endif

typedef enum {
	CONSOLE_LOGTYPE_ERROR	= 0x00,
	CONSOLE_LOGTYPE_WARN	= 0x01,
	CONSOLE_LOGTYPE_INFO	= 0x02,
	CONSOLE_LOGTYPE_LOG		= 0x03
} Logtype_t;

#define CONSOLE_MESSAGE_SIZE	128

/** 
* \class Console
* \brief TO DO 
* 
* Description
*/
class Console {

	// list of classes that can access protected functions
	friend class Byzance;
	friend class MQTTInfo;
	friend class MQTTSettings;

	public:
		
		/** 
		* error
		*
		* \param
		* \param 
		* \return
		*/
		static bool error(const char* format, ...);
		
		/** 
		* warn
		*
		* \param
		* \param 
		* \return
		*/
		static bool warn(const char* format, ...);
		
		/** 
		* info
		* 
		* \param
		* \param 
		* \return
		*/
		static bool info(const char* format, ...);
		
		/** 
		* log
		* 
		* \param
		* \param 
		* \return
		*/
		static bool log(const char* format, ...);

		/** 
		* enabled
		* 
		* \param
		* \param 
		* \return
		*/
		static bool enabled();

	protected:
		
		/** 
		* enable
		* 
		* \param
		* \param 
		* \return
		*/
		static void enable(bool en);

	private:
		
		/** 
		* send
		* 
		* \param
		* \param 
		* \return
		*/
		static bool send(Logtype_t type, char* data);

		static bool _enabled;

		/**
		 * Private constructor and destructor
		 */
		Console() {};
		~Console() {};
};

#endif /* CONSOLE_H */
