#include "LedModule.h"

DigitalOut*		LedModule::_led_red;
DigitalOut*		LedModule::_led_grn;
DigitalOut*		LedModule::_led_blu;

uint8_t			LedModule::_color;
uint32_t		LedModule::_sequence;
int8_t			LedModule::_bit_cntr;
int8_t			LedModule::_highest_bit;

bool 			LedModule::_led_inverted;

Ticker			*LedModule::_led_ticker = NULL;

bool			LedModule::_initialised;

REGISTER_DEBUG(LedModule);

/*
 * @return true		- inited right now
 * @return false	- inited earlier
 */
bool LedModule::init(PinName r, PinName g, PinName b, bool inverted){

	// return true after first init
	// otherwise don't init again and return false
	if (_initialised){
		__TRACE("init skipped\n");
		return false;
	}

	__TRACE("init follows\n");

	_led_red = new DigitalOut(r);
	_led_grn = new DigitalOut(g);
	_led_blu = new DigitalOut(b);

	_led_inverted = inverted;

	// default state
	if(_led_inverted){
		*_led_red = 1;
		*_led_grn = 1;
		*_led_blu = 1;
	} else {
		*_led_red = 0;
		*_led_grn = 0;
		*_led_blu = 0;
	}

	_led_ticker = new Ticker();

	_color = LED_COLOR_BLACK;
	_sequence = 0;
	_bit_cntr = 0;
	_highest_bit = 0;

	_initialised = true;

	return true;
}

bool LedModule::initialised(){
	return _initialised;
}

/**
 * Nastavuje stav dane LED podle predaneho blikaciho kodu.
 *
 * @param uint8_t color - 3b dlouhy barevny kod, viz *.h soubor k teto funkci
 * @param uint32_t sequence - 32b dlouhy binarni kod blikaciho kodu
 */
bool LedModule::set(uint8_t color, uint32_t sequence){

	if(!_initialised){
		__TRACE("not initialised\n");
		return 0;
	}

	// there is no change -> return
	if((_color==color)&&(_sequence == sequence)){
		__TRACE("unchanged; color=0x%08X, sequence=0x%08X, highest_bit=%d\n", _color, _sequence, _highest_bit);
		return 0;
	}

	// find highest bit of sequence
    for (uint8_t var = 0; var < sizeof(sequence)*8; var++){
        if ((sequence & (1 << var)) != 0){
            _highest_bit = var;
        }
    }

    // update variables
	_color = color;
    _sequence = sequence;
    _bit_cntr = _highest_bit;

    // start ticker
	_led_ticker->attach(&update, (float)0.1);

    __TRACE("updated; color=0x%08X, sequence=0x%08X, highest_bit=%d\n", _color, _sequence, _highest_bit);

	return 1;
}

bool LedModule::set(uint8_t color){

	if(!_initialised){
		__TRACE("not initialised\n");
		return 0;
	}

	// disable ticker -> there is no sequence to handle
	if(_led_ticker!=NULL){
		_led_ticker->detach();
	}

	// there is no change -> return
	if(_color==color){
		__TRACE("unchanged; color=0x%08X", _color);
		return 0;
	}

	// update internal state
	_color = color;

	light(_color);

	__TRACE("updated; color=0x%08X\n", _color);

	return 1;
}

void LedModule::light(uint8_t color){

	bool val_r;
	bool val_g;
	bool val_b;

	// new red color
	if(color & 0b100){
		val_r = 1;
	} else {
		val_r = 0;
	}

	// new green color
	if(color & 0b010){
		val_g = 1;
	} else {
		val_g = 0;
	}

	// new blue color
	if(color & 0b001){
		val_b = 1;
	} else {
		val_b = 0;
	}

	// invert?
	if(_led_inverted){
		val_r = !val_r;
		val_g = !val_g;
		val_b = !val_b;
	}

	// set pins
	*_led_red = val_r;
	*_led_grn = val_g;
	*_led_blu = val_b;
}

void LedModule::update(void) {

	bool bit;

	// read next bit from sequence
	bit = _sequence & (1 << _bit_cntr);

	// sequence bit is set -> defined color should be enabled
	if(bit){
		light(_color);
	// bit is disabled -> disabled led module
	} else {
		light(LED_COLOR_BLACK);
	}

	// decrement or reset counter
	if(_bit_cntr > 0){
		_bit_cntr--;
	} else {
		_bit_cntr = _highest_bit;
	}

}
