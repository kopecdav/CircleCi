/* Copyright (c) 2012 cstyles, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef WS2812_H
#define WS2812_H

#include "mbed.h"

// modulation for 1/0
#define WS11	0b11101110
#define WS10	0b11101000
#define WS01	0b10001110
#define WS00	0b10001000
#define WS0		0b11000000
#define WS1		0b11110000
// spi freq
#define SPICLK 8000000 // orig was 5000000

// sizeof spi byte in bits
#define SPIBPF 8

// how many bytes of memory does one color needs
#define COLORBUFF SPIBPF*3

//!Library for the WS2812 RGB LED with integrated controller
/*!
The WS2812 is controller that is built into a range of LEDs
*/
class WS2812
{
public:
    //!Creates an instance of the class.
    /*!
    Connect WS2812 address addr using SPI MOSI pins
    */
    WS2812(PinName d, int size);

    /*!
    Destroys instance.
    */
    ~WS2812();

    //!Reads the current temperature.
    /*!
    Reads the temperature register of the LM75B and converts it to a useable value.
    */
    void write (int buf[]);
    void write_offsets (int buf[],int r_offset=0, int g_offset=0, int b_offset=0);
    void setAll(int colour);

    void useII(int d);
    void setII(unsigned char II);


private:

    int				_size;
    uint8_t			_II;
    int				_use_II;
    SPI				*_spi;
    char			*_data_buffer;
	volatile bool	_busy;

	event_callback_t _spi_done_callback;

    void _fill_buffer (int color, char* buffer);

    void _flush_buffer(char* buffer);

	void _done(int event);

};

#endif
