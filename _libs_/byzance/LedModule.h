/*
 * LedModule.h
 *
 *  Created on: 20. 9. 2016
 *      Author: Viktor, Martin
 */

#ifndef BYZANCE_LEDMODULE_H_
#define BYZANCE_LEDMODULE_H_

#include "byzance_debug.h"
#include "ByzanceLogger.h"

// Definice moznych barevnych kodu RGB LED na GEN3 HW (GEN2.0 a GEN2.1 s oddelenymi LED (green, red, sys) nic takoveho nepotrebuje)
enum led_color_enum {
	LED_COLOR_RED =							((uint32_t) 0b100),
	LED_COLOR_GREEN =						((uint32_t) 0b010),
	LED_COLOR_BLUE =						((uint32_t) 0b001),
	LED_COLOR_ORANGE =						((uint32_t) 0b110),
	LED_COLOR_VIOLET =						((uint32_t) 0b101),
	LED_COLOR_CYAN =						((uint32_t) 0b011),
	LED_COLOR_WHITE =						((uint32_t) 0b111),
	LED_COLOR_BLACK =						((uint32_t) 0b000)
};

// Definice barevnych a blikacich kodu pro zde vyjmenovane stavy systemu
#define		LED_CODE_CONNECTED				((uint32_t) 0b1100)
#define		LED_COLOR_CONNECTED				LED_COLOR_GREEN

#define		LED_CODE_DISCONNECTED			((uint32_t) 0b11111111111111111000000)
#define		LED_COLOR_DISCONNECTED			LED_COLOR_GREEN

#define		LED_CODE_BUSY					((uint32_t) 0b10)
#define		LED_COLOR_BUSY					LED_COLOR_VIOLET


/**
* \class LedModule
* \brief brief TO DO 
*
* Long description TO DO
*/
class LedModule{
public:
	
	/** init
	*
	* \param
	* \param
	* \return
	*/
	static bool				init(PinName r, PinName g, PinName b, bool inverted = 0);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static bool 			initialised();

	/** 
	*
	* \param
	* \param
	* \return
	*/
	static bool 			set(uint8_t color);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static bool 			set(uint8_t color, uint32_t sequence);

protected:

	/** light
	*
	* \param
	* \param
	* \return
	*/
	static void 			light(uint8_t color);
	
	/** update  
	*
	* \param
	* \param
	* \return
	*/
	static void 			update(void);

	static DigitalOut		*_led_red;
	static DigitalOut		*_led_grn;
	static DigitalOut 		*_led_blu;

	static uint8_t			_color;
	static uint32_t			_sequence;
	static int8_t			_bit_cntr;
	static int8_t			_highest_bit;

	static bool 			_led_inverted;

	static bool				_initialised;

	static Ticker			*_led_ticker;

private:
	LedModule(){};
    ~LedModule(){};
};


#endif /* BYZANCE_LEDMODULE_H_ */


