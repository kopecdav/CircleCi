/*
 * byzance_spi.h
 *
 *  Created on: 23. 11. 2016
 *      Author: Viktor
 */

#ifndef BYZANCE_DRIVERS_BYZANCESPI_H_
#define BYZANCE_DRIVERS_BYZANCESPI_H_

#include "mbed.h"

class ByzanceSpi: public SPI{
	public:
		ByzanceSpi(PinName mosi, PinName miso, PinName sck);
#if !defined(BYZANCE_BOOTLOADER)
		Mutex					_mutex;
#endif
};

#endif /* BYZANCE_DRIVERS_BYZANCESPI_H_ */
