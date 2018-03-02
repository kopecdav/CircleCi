#include "TripleSevenSeg.h"

REGISTER_DEBUG(TripleSevenSeg);

TripleSevenSeg::TripleSevenSeg(){
	_initialized = false;

	_driver = new TCA6424A();

	_driver->initialize(X07, X06); 	// I2C SDA, I2C SCL
	__TRACE("TCA6424A driver initialized.\n");

	if(!_driver->testConnection()){
		__ERROR("TCA6424A drive test connection failed - library initialization failed.\n");
		return;
	}

	// Set all pin as TCA6424A_OUT (1 = in, 0 = out)
	_driver->setAllDirection(0, 0, 0);
	_driver->setAllPolarity(0x00, 0x00, 0x00);

	_initialized = true;
	__TRACE("TCA6424A driver test connection successful.\n");

	// Clear display after init.
	display_symbol(' ', 0, false);
	display_symbol(' ', 1, false);
	display_symbol(' ', 2, false);
}


bool TripleSevenSeg::is_initialized(void){
	return _initialized;
}

uint32_t TripleSevenSeg::display_symbol(char symbol, uint8_t position, bool dot){
	char data = 0;

	// Is seven segment driver initialized?
	if(!_initialized){
		__ERROR("Seven segment driver not initialized.\n", position);
		return 1;
	}

	// Check LED panel position (ShieldB1L_LED_170920 has 3 digits only).
	if(position > 3){
		__ERROR("Unsupported LED position (allowed 0,1,2 only, entered %d).\n", position);
		return 2;
	}
	__TRACE("Write symbol: %c (0x%02X), position=%d, dot=%d.\n", symbol, symbol, position, dot);


	// Define all supported symbols.
	switch(symbol){
		case '0':
			data = SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F;
			break;
		case '1':
			data = SEG_B + SEG_C;
			break;
		case '2':
			data = SEG_A + SEG_B + SEG_G + SEG_E + SEG_D;
			break;
		case '3':
			data = SEG_A + SEG_B + SEG_C + SEG_D + SEG_G;
			break;
		case '4':
			data = SEG_F + SEG_G + SEG_B + SEG_C;
			break;
		case '5':
			data = SEG_A + SEG_F + SEG_G + SEG_C + SEG_D;
			break;
		case '6':
			data = SEG_A + SEG_F + SEG_E + SEG_D + SEG_C + SEG_G;
			break;
		case '7':
			data = SEG_A + SEG_B + SEG_C;
			break;
		case '8':
			data = SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G;
			break;
		case '9':
			data = SEG_A + SEG_B + SEG_C + SEG_D + SEG_F + SEG_G;
			break;
		case '-':
			data = SEG_G;
			break;
		case ' ':
			data = 0;
			break;
		default:
			__WARNING("Unsupported symbol.\n");
			return 3;
	}

	// Also turn on the decimal dot?
	if(dot){
		data += SEG_DP;
	}

	// Write NEGATIVE data to the specific position (LEDs on = log. 0).
	_driver->writeBank(position, ~data);

	return 0;
}


bool TripleSevenSeg::display_number(float value, uint8_t decimal_positions){
	char txt[15], lcd[10];	// Max. text length is 10 chars (4294967295).
	uint8_t offset = 0;
	uint8_t start_position;
	bool dot = false;


	// Is seven segment driver initialized?
	if(!_initialized){
		__ERROR("Seven segment driver not initialized.\n", decimal_positions);
		return 1;
	}
	__DEBUG("Value to display: %6.3f. Dec. position: %d.\n", value, decimal_positions);


	//Check valid decimal position value
	switch (decimal_positions){
		case 0:
			sprintf(txt, "%3.0f", value);
			start_position = 2;
			break;
		case 1:
			sprintf(txt, "%4.1f", value);
			start_position = 1;
			break;
		case 2:
			sprintf(txt, "%4.2f", value);
			start_position = 0;
			break;

		default:
			__WARNING("Decimal position invalid (%d).\n", decimal_positions);
		case 255:
			sprintf(txt, "%f", value);
			start_position = 0;
			break;

	}
	__TRACE("Value to display ins ascii: %s.\n", txt);

	// Check if value is out of printable range.
	if((value < -99.90) || (value > 999.00)){
		lcd[0] = lcd[1] = lcd[2] = '-';
		__INFO("Value is out of printable range.\n");
	} else{
		// Copy most significant part of text (value) including dot char.
		strncpy(lcd, txt, 4);
	}


	// Print text on segments char by char
	for(uint8_t i = 0; i < 3; i++){
		if((lcd[i + 1] == '.') && (!dot)){
			dot = true;
		}
		display_symbol(lcd[i + offset], 2 - i, dot);

		// Display also decimal dot?
		if(dot){
			offset = 1;
		}
		dot = false;
	}
	return 0;
}
