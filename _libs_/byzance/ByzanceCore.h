#ifndef _BYZANCE_CORE_H
#define _BYZANCE_CORE_H

#include "mbed.h"
#include "rtos.h"

#include "ByzanceAnalogInputRegistration.h"
#include "ByzanceDigitalInputRegistration.h"
#include "ByzanceMessageInputRegistration.h"

#include "ByzanceSerializer.h"
#include "ByzanceParser.h"
#include "macros_byzance.h"

#include "byzance_config.h"

#include "MqttBuffer.h"
#include "LedModule.h"
#include "BinManager.h"

#include <vector>
#include <string>

/*
 * TODO předělat
 */
typedef enum
{
	BYZANCE_OK						= 0x00, // vsechno v pohode
	BYZANCE_IP_ERROR				= 0x01, // nepodarilo se pripojit k TCP/IP socketu brokeru
	BYZANCE_MQTT_CONNECTION_ERROR	= 0x02, // nepodarilo se neco s MQTT brokerem
	BYZANCE_MQTT_SUBSCRIBE_ERROR	= 0x03, // nepodril se subscribe na nektery topic
	BYZANCE_MQTT_CREDENTIAL_ERROR	= 0x04, // chybne uzivatelske udaje MQTT brokeru
	BYZANCE_LINKED_DOWN				= 0x05, // chybne uzivatelske udaje MQTT brokeru
	BYZANCE_INVALID_INTERFACE		= 0x06, // interface je neco jineho, nez wifi nebo ethernet
	BYZANCE_GENERAL_ERROR			= 0xFF	// vseobecna chyba syntaxe
} Byzance_Err_TypeDef;

class ByzanceDigitalInputRegistration;
class ByzanceAnalogInputRegistration;
class ByzanceMessageInputRegistration;
class ByzanceSerializer;

/**
* \class ByzanceCore
* \brief TO DO.
*
* Description TO DO
*/
class ByzanceCore {
public:

	/**
	 * Init idle hook and ticker (used for computing CPU LOAD)
	 * TO DO
	 */
	static void hook_attach(void);
	static void ticker_init(void);

	/**
	 * Outputs functions
	 */
    static int	digital_output_set_value(const char *name, bool value);
    static int	analog_output_set_value(const char *name, float value);
    static int	message_output_set_value(const char *name, ByzanceSerializer* serializer);

    /**
     * When shit happens
     * TO DO
	 */
    static void catastropic_failure(int code = 0);

	/**
     * TO DO
	 */
    static time_t restart(time_t sec = 0);

	/**
     * TO DO
	 */
    static void connected_start();

	/**
     * TO DO
	 */
    static void connected_stop();

	/**
     * TO DO
	 */
    static time_t connected_time();

	/**
     * TO DO
	 */
    static void	led_module(bool state);
  
    /**
     * Byzance registrations
     */
    static void add_digital_input_registration(ByzanceDigitalInputRegistration *registration);
	
	/**
     * Byzance registrations
     */
    static void add_analog_input_registration(ByzanceAnalogInputRegistration *registration);  
    
	/**
     * Byzance registrations
     */
	static void add_message_input_registration(ByzanceMessageInputRegistration *registration);
    
    /*
     * Byzance vectors
     */
    static std::vector<ByzanceDigitalInputRegistration>                                                  *_digitalInputs;
    static std::vector<ByzanceAnalogInputRegistration>                                                   *_analogInputs;
    static std::vector<ByzanceMessageInputRegistration>                                                  *_messageInputs;

    static char			*_fullID;

    /*
     * Load of MCU 0-100%
     */
    static uint8_t 		cpu_load;

    // how long is device running from last restart (in seconds)
    static time_t		uptime;

    static bool			restart_pending;

    static time_t		restart_sec;

    // software request for restart
    /** TODO add brief
     *
     * TODO add long description
     *
     * @param bool
     *
     */
    static void attach_restart_follows(void (*function)(void));

    /** TODO add brief
     *
     * TODO add long description
     *
     * @param bool
     *
     */
    template<typename T>
    static void attach_restart_follows(T *object, void (T::*member)(void)) {
    	_restart_follows_callback.attach(object, member);
    }

    /** BinBusy callback
     *
     * BinManager is busy
     *
     * @param bool
     *
     */
    static void attach_bin_busy(void (*function)(bool));

    /** TODO add brief
     *
     * TODO add long description
     *
     * @param bool
     *
     */
    template<typename T>
    static void attach_bin_busy(T *object, void (T::*member)(bool)) {
    	_bin_busy_callback.attach(object, member);
    }

protected:

    /**
	*is called by ticker
	*/
	static void ticker_call(void);

    // how long is device connected to Homer
    static time_t		_connected;

    // automatically increase connected time
    static bool 		_connected_increase;
	
    static bool			_led_core_controlled;

	/**
	* is called when MCU is in idle -> computes CPU LOAD
	*/
	static void idle_hook(void);

    static Ticker		_ticker;

    // internal restart pending callback
    static Callback<void(void)>		_restart_follows_callback;

    // BinManager is busy - last state
    static bool _bin_busy;

    // BinManager is busy
    static Callback<void(bool)>		_bin_busy_callback;

private:
	/**
	* TO DO
	*/
    ByzanceCore() {};
    ~ByzanceCore() {};
};

#endif /* _BYZANCE_CORE_H */
