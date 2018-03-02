#include "Watchdog.h"

Watchdog* Watchdog::_inst = NULL;

REGISTER_DEBUG(Watchdog);

Watchdog* Watchdog::instance(){
	if(_inst == NULL){
		_inst = new Watchdog();
		if(_inst == NULL){
			__ERROR("unable to allocate watchdog instance\n");
		}
	}

	return _inst;
}

/**
 * Read source of the previous restart.
 */
Watchdog::Watchdog() {

	__TRACE("constructor\n");

    // capture the cause of the previous reset
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
    	reset_source = RESET_SOURCE_IWD;
    } else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST)) {
    	reset_source = RESET_SOURCE_PIN;
    }else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST)) {
    	reset_source = RESET_SOURCE_POR;
    }else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
    	reset_source = RESET_SOURCE_SW;
    }else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) {
    	reset_source = RESET_SOURCE_WWD;
    }else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
    	reset_source = RESET_SOURCE_LOW_POWER;
    } else{
    	reset_source = RESET_SOURCE_UNKNOWN;
    }
}

/**
 * Configure the timeout for the Watchdog.
 * This configures the Watchdog service and starts it. It must
 * be serviced before the timeout, or the system will be restarted.
 *
 * @param timeout  - in seconds, as a floating point number
 */
void Watchdog::configure(float timeout) {

	__TRACE("configure\n");

	if(timeout==0){
		__WARNING("disabled\n");
		return;
	}

#if defined(TARGET_BYZANCE_IODAG3E)
    #define LSI_FREQ 		((uint32_t)32000)		// Zdrojova frekvence WD periferie.
#else
	#error "Neznamy target (Watchdog.cpp)"
#endif

    uint16_t prescaler_code;
    uint16_t prescaler;
    uint16_t reload_value;
    // float calculated_timeout;

    if ((timeout * (LSI_FREQ / 4)) < 0x7FF) {
        prescaler_code = IWDG_PRESCALER_4;
        prescaler = 4;
    }
    else if ((timeout * (LSI_FREQ / 8)) < 0xFF0) {
        prescaler_code = IWDG_PRESCALER_8;
        prescaler = 8;
    }
    else if ((timeout * (LSI_FREQ / 16)) < 0xFF0) {
        prescaler_code = IWDG_PRESCALER_16;
        prescaler = 16;
    }
    else if ((timeout * (LSI_FREQ / 32)) < 0xFF0) {
        prescaler_code = IWDG_PRESCALER_32;
        prescaler = 32;
    }
    else if ((timeout * (LSI_FREQ / 64)) < 0xFF0) {
        prescaler_code = IWDG_PRESCALER_64;
        prescaler = 64;
    }
    else if ((timeout * (LSI_FREQ / 128)) < 0xFF0) {
        prescaler_code = IWDG_PRESCALER_128;
        prescaler = 128;
    }
    else {
        prescaler_code = IWDG_PRESCALER_256;
        prescaler = 256;
    }

    // specifies the IWDG Reload value. This parameter must be a number between 0 and 0x0FFF.
    reload_value = (uint32_t)(timeout * (LSI_FREQ / prescaler));

    // calculated_timeout = ((float)(prescaler * reload_value)) / LSI_FREQ;
    // ByzanceLogger::info("WATCHDOG set with prescaler: %d reload value: 0x%X - timeout: %fs\n",prescaler, reload_value, calculated_timeout);

    IWDG->KR = 0x5555;			// Disable write protection of IWDG registers
    IWDG->PR = prescaler_code;   // Set PR value
    IWDG->RLR = reload_value;    // Set RLR value
    IWDG->KR = 0xAAAA;    		// Reload IWDG
    IWDG->KR = 0xCCCC;    		// Start IWDG - See more at: http://embedded-lab.com/blog/?p=9662#sthash.6VNxVSn0.dpuf

    service();
}


/**
 *  Feed the dog. Service the Watchdog so it does not cause a system reset
 */
void Watchdog::service() {

	__TRACE("HAF!\n");

    IWDG->KR = 0xAAAA;         //Reload IWDG - See more at: http://embedded-lab.com/blog/?p=9662#sthash.6VNxVSn0.dpuf
}

/**
 *  Get the flag to indicate if the watchdog causes the reset.
 */
RESET_SOURCE_TYPE Watchdog::caused_reset() {

	__TRACE("caused_reset\n");

    return reset_source;
}

RESET_SOURCE_TYPE Watchdog::caused_reset(char * txt) {

	__TRACE("caused_reset\n");

	switch (reset_source) {
		case RESET_SOURCE_PIN:
			strcpy(txt, "PIN");
			break;
		case RESET_SOURCE_POR:
			strcpy(txt, "POR");
			break;
		case RESET_SOURCE_SW:
			strcpy(txt, "SW");
			break;
		case RESET_SOURCE_IWD:
			strcpy(txt, "IWD");
			break;
		case RESET_SOURCE_WWD:
			strcpy(txt, "WWD");
			break;
		case RESET_SOURCE_LOW_POWER:
			strcpy(txt, "LOW_POWER");
			break;
		case RESET_SOURCE_UNKNOWN:
			strcpy(txt, "unknown");
			break;
		default:
			strcpy(txt, "default");
			break;
	}

    return reset_source;
}


