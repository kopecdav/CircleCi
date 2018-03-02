/*
 * TrippleSevenSeg.h
 *
 *  Created on: 17. 10. 2017
 *      Author: Viktor
 */

#ifndef LIBRARIES_TRIPPLESEVENSEG_H_
#define LIBRARIES_TRIPPLESEVENSEG_H_

#include "mbed.h"
#include "TCA6424A.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"

#define SEG_A		(0x01 << 3)
#define SEG_B		(0x01 << 1)
#define SEG_C		(0x01 << 2)
#define SEG_D		(0x01 << 6)
#define SEG_E		(0x01 << 7)
#define SEG_F		(0x01 << 5)
#define SEG_G		(0x01 << 4)
#define SEG_DP		(0x01 << 0)


class TripleSevenSeg {
    public:
		TripleSevenSeg();

		/**
		 * Is driver initialized?
		 *
		 */
		bool is_initialized(void);

		/**
		 * Display float value on seven segment display. Supports positive and negative numbers in range from -99 to 999.
		 *
		 * @param value Value to display.
		 * @param decimal_positions Maximal number of displayed decimal positions.
		 */
		bool display_number(float value, uint8_t decimal_positions);

		/**
		 * Display supported symbol to LED segment at specified position with optional decimal point.
		 * @param symbol Supported symbol.
		 * @param position Position on LED display.
		 * @param show decimal point
		 *
		 */
		uint32_t display_symbol(char symbol, uint8_t position, bool dot);
	protected:


    private:
		TCA6424A * _driver;
		bool _initialized;
};



#endif /* LIBRARIES_TRIPLESEVENSEG_H_ */
