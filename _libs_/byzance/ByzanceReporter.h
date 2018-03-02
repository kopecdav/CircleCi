#ifndef BYZANCE_REPORTER_H
#define BYZANCE_REPORTER_H

#include "picojson.h"
#include "ByzanceClient.h"
#include "enum_component.h"
#include "enum_binstate.h"

#ifndef REPORTER_DEBUGGING_LEVEL
#define REPORTER_DEBUGGING_LEVEL		DEBUG_LEVEL_NONE
#endif

#if REPORTER_DEBUGGING
	#if REPORTER_DEBUGGING_LEVEL >= DEBUG_LEVEL_LOG
		#define REPORTER_LOG(...);		ByzanceLogger::log(__VA_ARGS__);
	#else
		#define REPORTER_LOG(...);		{}
	#endif

	#if REPORTER_DEBUGGING_LEVEL >= DEBUG_LEVEL_ERROR
		#define REPORTER_ERROR(...);		ByzanceLogger::error(__VA_ARGS__);
	#else
		#define REPORTER_ERROR(...);		{}
	#endif

	#if REPORTER_DEBUGGING_LEVEL >= DEBUG_LEVEL_WARNING
		#define REPORTER_WARNING(...);	ByzanceLogger::warning(__VA_ARGS__);
	#else
		#define REPORTER_WARNING(...);	{}
	#endif

	#if REPORTER_DEBUGGING_LEVEL >= DEBUG_LEVEL_INFO
		#define REPORTER_INFO(...);		ByzanceLogger::info(__VA_ARGS__);
	#else
		#define REPORTER_INFO(...);	{}
	#endif

	#if REPORTER_DEBUGGING_LEVEL >= DEBUG_LEVEL_DEBUG
		#define REPORTER_DEBUG(...);		ByzanceLogger::debug(__VA_ARGS__);
	#else
		#define REPORTER_DEBUG(...);		{}
	#endif

	#if REPORTER_DEBUGGING_LEVEL >= DEBUG_LEVEL_TRACE
		#define REPORTER_TRACE(...);		ByzanceLogger::trace(__VA_ARGS__);
	#else
		#define REPORTER_TRACE(...);		{}
	#endif

#else
	#define REPORTER_LOG(...);		{}
	#define REPORTER_ERROR(...);		{}
	#define REPORTER_WARNING(...);	{}
	#define REPORTER_INFO(...);		{}
	#define REPORTER_DEBUG(...);		{}
	#define REPORTER_TRACE(...);		{}
#endif

/**
* Enum description To Do 
*/
typedef enum {
	REPORT_EVENT_WELCOME
} event_t;

/**
* \class ByzanceReporter
* \brief brief TO DO.
*
* Long description To Do 
*/
class ByzanceReporter {
	public:
		
		/**
		* report
		*
		* \param
		*/
		static void report(event_t event);
		
		/**
		* report_state
		*
		* \param
		* \param 
		*/
		static void report_state(bin_component_t comp, bin_state_t state);

	protected:

		ByzanceReporter();
		~ByzanceReporter();

};

#endif /* BYZANCE_REPORTER_H */
