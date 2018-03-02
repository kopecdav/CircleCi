#include "WS2812.h"

WS2812::WS2812(PinName d, int size)
//WS2812::WS2812(PinName d, int size) : _spi(d, X12, X10) // output the clock for scope test
{
	_spi = new SPI(d, NC, NC);

    _size = size;
    _spi->format(SPIBPF,1);
    _spi->frequency(SPICLK);
    _use_II = 0; // 0=off,1=use global,2=per pixel
    _II = 0xFF; // set global intensity to full

    _spi_done_callback.attach(this, &WS2812::_done);

    _busy = 0;

    _data_buffer = (char*)malloc(COLORBUFF*_size);
}


WS2812::~WS2812(){
	free(_data_buffer);
}


void WS2812::write(int buf[]){
    // for each of the data points in the buffer
    for (int i = 0; i < _size ; i++) {
        _fill_buffer(buf[i], _data_buffer+24*i);
    }

    _flush_buffer(_data_buffer);

}


void WS2812::write_offsets(int buf[], int r_offset, int g_offset, int b_offset){
    // for each of the data points in the buffer
    for (int i = 0; i < _size ; i++) {

        unsigned int argb = 0x0;
        // index and extract colour fields from IIRRGGBB buf[]
        // 0 = blue, 1 = green, 2 = red, 3 = brightness
        argb |=  (buf[(i+b_offset)%_size] & 0x000000FF);
        argb |= ((buf[(i+g_offset)%_size] & 0x0000FF00));
        argb |= ((buf[(i+r_offset)%_size] & 0x00FF0000));
        argb |= (buf[i] & 0xFF000000);
        _fill_buffer(argb, _data_buffer+24*i);
    }

    _flush_buffer(_data_buffer);
}


void WS2812::setAll(int colour){

    for (int i = 0; i < _size ; i++) {
        _fill_buffer(colour, _data_buffer+COLORBUFF*i);
    }

    _flush_buffer(_data_buffer);
}


void WS2812::useII(int d){
    if (d > 0) {
        _use_II = d;
    } else {
        _use_II = 0;
    }
}


void WS2812::setII(unsigned char II){
    _II = II;
}


void WS2812::_fill_buffer(int color, char* buffer){
	printf("color is %x\n");
    // Outut format : GGRRBB
    // Inout format : IIRRGGBB
    unsigned char agrb[4] = {0x0, 0x0, 0x0, 0x0};

    unsigned char sf; // scaling factor for  II

    // extract colour fields from incoming
    // 0 = blue, 1 = red, 2 = green, 3 = brightness
    agrb[0] = color & 0x000000FF;
    agrb[1] = (color & 0x00FF0000) >> 16;
    agrb[2] = (color & 0x0000FF00) >> 8;
    agrb[3] = (color & 0xFF000000) >> 24;

    // set and intensity scaling factor (global, per pixel, none)
    if (_use_II == 1) {
        sf = _II;
    } else if (_use_II == 2) {
        sf = agrb[3];
    } else {
        sf = 0xFF;
    }

    // Apply the scaling factor to each othe colour components
    for (int clr = 2; clr >= 0; clr--) {
        agrb[clr] = ((agrb[clr] * sf) >> 8);
    }

    // For each colour component G,R,B
    // shift out the data 7..0, writing a SPI frame per bit
    // green=2,red=1,blue=0,

    uint32_t i = 0;

    for (int clr = 2; clr >= 0; clr--) {
    	printf("setting %d color %x\n",clr,agrb[clr]);
    	/*uint8_t val=agrb[clr];

    	for(int j = 0; j<4; j++){
    		switch(val&0x03){
    		case 0:
    			buffer[i] = WS00;
    			break;
    		case 1:
    			buffer[i] = WS01;
    			break;
    		case 2:
    			buffer[i] = WS10;
    			break;
    		case 3:
    			buffer[i] = WS11;
    			break;
    		default:
    			printf("invalid\n");
    		}
    		i++;
    		val=val>>2;
    	}*/

        for (int bit = 7 ; bit >= 0 ; bit--) {
            if (agrb[clr] & (0x1 << bit)) {
                //__spi.write(WS1);
                buffer[i] = WS1;
            } else {
                //__spi.write(WS0);
            	buffer[i] = WS1;
            }
            i++;
        }
    }

}

void WS2812::_flush_buffer(char* buffer){
    _busy = 0;
    _spi->transfer((uint8_t*)buffer, COLORBUFF*_size, (uint8_t*)NULL, 0, _spi_done_callback, SPI_EVENT_COMPLETE);
    while(_busy==0);
}

void WS2812::_done(int event){
	_busy = 1;
}








