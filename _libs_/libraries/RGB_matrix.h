/*
 * RGB_matrix.h
 *
 *  Created on: 28. 11. 2017
 *      Author: David Kopecký
 */

#ifndef RGB_MATRIX_H_
#define RGB_MATRIX_H_

#include "mbed.h"
#include "HD44780_6x8.h"
#include <string>
//#include <iostream>

enum RGB_MATRIX_COLOR {
	NONE = 0,
	BLUE,
	GREEN,
	LIGHT_BLUE,
	RED,
	PINK,
	YELLOW,
	WHITE
};




/**
 * \brief Třída ovládající maticový RGB diplej
 *
 */
class RGB_matrix{


	public:

	/**
	 * Inicializuje RGB display,
	 * \param Výstupní piny pomocí kterých bude zařízení diplay ovládat
	 */
	static void Init(PinName Pin_R1, PinName Pin_R2,PinName Pin_G1 ,PinName Pin_G2, PinName Pin_B1,PinName Pin_B2,
			PinName Pin_CLK,PinName Pin_LAT, PinName Pin_OE, PinName Pin_A,PinName Pin_B,PinName Pin_C,PinName Pin_D);

	/**
	 * Nastaví barvu, kterou bude display při dalších výpisech používat
	 *\param ENUM RGB_MATRIX_COLOR
	 */
	static void set_color(char color);

	/**
	* Nastaví barvu, kterou bude display při dalších výpisech používat v pozadí
	*\param ENUM RGB_MATRIX_COLOR
	*/
	static void set_background(char color);

	/**
	 *  Nastaví barvu na konkrétním pixelu
	 * \param pozice x - řádek
	 * \param pozice y - sloupec
	 * \param c - barva pixelu (0-7, 0 = zhasnuto)
	 */
	static void set_pixel(uint16_t x ,uint16_t y, uint16_t c);

	/**
	 * Vykreslí znak z knihovny znaků na danou pozici (pozice určena od levého horního rohu)
	 * \param pozice x - řádek
	 * \param pozice y - sloupec
	 * \param c - znak který se má vykreslit
	 */
	static char put_char(int x,int y, char c);

	/**
	 * Vypíše zadaný text na zadaný řádek (1-4).
	 * \param *str pole charů k výpisu (doporučuji pracovat se datovým typem string a při volání funkce do argumentu zadat &str[0])
	 * \param line_number - řádek na který se text vypíše
	 */
	static void put_line(char* str, int line_number);


	/**
	 * Vypíše zadaný text na zadaný řádek (1-4).
	 * \param *str pole charů k výpisu (doporučuji pracovat se datovým typem string a při volání funkce do argumentu zadat &str[0])
	 * \param mins počet min vypsaných vpravo matice
	 * \param line_number - řádek na který se text vypíše
	 */
	static void put_row(char* str,uint16_t mins, int line_number);

	/**
	 * metoda která sekvenčně vykreslí jeden řádek. Tato funkce je sama volána tickerem každých 500 mikrosekund po inicializaci a
	 * není potřeba jí volat v uživatelském kódu.
	 */
	static void paint();

	/**
	 * Vykreslí všechny řádky
	 *
	 */
	static void paint_all();

	/**
	 * Funkce odpojí ticker používaný k sekvenčnímu vykreslování řádků
	 */
	static void detach_ticker();

	/**
	 *  Funkce znovu připojí ticker, používaný k sekvenčnímu vykreslování řádků
	 */
	static void attach_ticker();



	private:

	static BusOut* ABCD; // Row address.
	static DigitalOut* CLK;    //  Data clock    - rising edge
	static DigitalOut* LAT;    //  Data latch    - active low (pulse up after data load)
	static DigitalOut* OE;     //  Output enable - active low (hold high during data load, bring low after LAT pulse)
	static DigitalOut* R1;     //  RED   Serial in for upper half
	static DigitalOut* R2;     //  RED   Serial in for lower half
	static DigitalOut* G1;      //  GREEN Serial in for upper half
	static DigitalOut* G2;      //  GREEN Serial in for lower half
	static DigitalOut* B1;      //  BLUE  Serial in for upper half
	static DigitalOut* B2;      //  BLUE  Serial in for lower half

	static Ticker*	painter;

	static uint16_t row_cnt;
	static uint16_t cycle_cnt;
	static uint16_t gm[128][6]; // Buffer with 128*6*16 bits. Graphics memory if you like.
	static char _background;
	static char _color;

	/*
	 * Vykreslý zadaný řádek na display
	 * \param Row - číslo řádku
	 */
	static void write_row(uint16_t Row);



};

#endif /* RGB_MATRIX_H_ */

