/*
 * watchdog.h
 *
 *  Created on: 17. 11. 2016
 *      Author: Viktor
 */

#ifndef BYZANCE_DRIVERS_WATCHDOG_H_
#define BYZANCE_DRIVERS_WATCHDOG_H_

#include "mbed.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

// Typy prenasenych binarek.
enum RESET_SOURCE_TYPE : uint8_t{
	RESET_SOURCE_PIN = 0,
	RESET_SOURCE_POR,
	RESET_SOURCE_SW,
	RESET_SOURCE_IWD,
	RESET_SOURCE_WWD,
	RESET_SOURCE_LOW_POWER,
	RESET_SOURCE_UNKNOWN = 0xFF
};

class Watchdog {
public:
	static Watchdog* instance();
	void configure(float timeout);
    void service();
    // WatchdogCausedReset identifies if the cause of the system
    // reset was the Watchdog
    RESET_SOURCE_TYPE caused_reset();
    RESET_SOURCE_TYPE caused_reset(char *txt);

private:
	static Watchdog* _inst;
	Watchdog();
	RESET_SOURCE_TYPE reset_source;
};

#endif /* BYZANCE_DRIVERS_WATCHDOG_H_ */
