#include "RGB_matrix.h"

DigitalOut* RGB_matrix::CLK;

BusOut* RGB_matrix::ABCD; // Row address.
DigitalOut* RGB_matrix::LAT;    //  Data latch    - active low (pulse up after data load)
DigitalOut* RGB_matrix::OE;     //  Output enable - active low (hold high during data load, bring low after LAT pulse)
DigitalOut* RGB_matrix::R1;     //  RED   Serial in for upper half
DigitalOut* RGB_matrix::R2;     //  RED   Serial in for lower half
DigitalOut* RGB_matrix::G1;      //  GREEN Serial in for upper half
DigitalOut* RGB_matrix::G2;      //  GREEN Serial in for lower half
DigitalOut* RGB_matrix::B1;      //  BLUE  Serial in for upper half
DigitalOut* RGB_matrix::B2;      //  BLUE  Serial in for lower half

Ticker*		RGB_matrix::painter = NULL;
uint16_t RGB_matrix::row_cnt;
uint16_t RGB_matrix::gm[128][6]; // Buffer with 64*number_of_linesx6 bytes. Graphics memory if you like.
char RGB_matrix::_background;
char RGB_matrix::_color;

/**
 * Inicializuje knihovnu RGB_matrix a fyzické piny desky. Zároveň zapne ticker, který sekvenčně vykresluje řádky displeje
 *
 */
void RGB_matrix::Init(PinName Pin_R1, PinName Pin_R2,PinName Pin_G1 ,PinName Pin_G2, PinName Pin_B1,PinName Pin_B2,
		PinName Pin_CLK,PinName Pin_LAT, PinName Pin_OE, PinName Pin_A,PinName Pin_B,PinName Pin_C,PinName Pin_D) {

	R1 = new DigitalOut(Pin_R1);
	R2 = new DigitalOut(Pin_R2);
	G1 = new DigitalOut(Pin_G1);
	G2 = new DigitalOut(Pin_G2);
	B1 = new DigitalOut(Pin_B1);
	B2 = new DigitalOut(Pin_B2);
	CLK = new DigitalOut(Pin_CLK);
	LAT = new DigitalOut(Pin_LAT);
	ABCD = new BusOut(Pin_A,Pin_B,Pin_C,Pin_D);
	OE = new DigitalOut(Pin_OE);

	//attach ticker
  painter = new Ticker();
  painter->attach(&paint, 0.001);
	row_cnt = 0;
	_color = RED;
	_background = 0;
}


void RGB_matrix::detach_ticker(){
	//painter.attach(&paint, 1000);
	if(painter != NULL){
		delete(painter);
		painter = NULL;
	}
	//painter.detach();
}

void RGB_matrix::attach_ticker(){
	/*
	if(&painter){
		painter.attach(&paint, 0.001);
	}
	*/

	if(painter == NULL){
		painter = new Ticker();
		painter->attach(&paint, 0.001);
	}
}


void RGB_matrix::set_background(char color){
	_background = color;
}


void RGB_matrix::set_color(char color){
	_color = color;
}


void RGB_matrix::set_pixel(uint16_t x ,uint16_t y, uint16_t c){

	// Handle graphix memory, so calling this method wont change

	// Display boundries
	if ((x > 127) || (y > 31)){
		return;
	}

	uint16_t r0,g0,b0;

    r0=(c & 4) >>2; 	// Extract red bit from color
    g0=(c & 2) >>1; 	// Extract green bit from color
    b0=(c & 1); 		// Extract blue bit from color

    // Handle second half of display
	if (y > 15){

		gm[x][3] = (gm[x][3] & ~(1<<(15-(y-16))));
		gm[x][4] = (gm[x][4] & ~(1<<(15-(y-16))));
		gm[x][5] = (gm[x][5] & ~(1<<(15-(y-16))));

		gm[x][3] = (gm[x][3] | (r0<<(15-(y-16))));
		gm[x][4] = (gm[x][4] | (g0<<(15-(y-16))));
		gm[x][5] = (gm[x][5] | (b0<<(15-(y-16))));

	}else{

		gm[x][0] = (gm[x][0] & ~(1<<(15-y)));
		gm[x][1] = (gm[x][1] & ~(1<<(15-y)));
		gm[x][2] = (gm[x][2] & ~(1<<(15-y)));

		gm[x][0] = (gm[x][0] | (r0<<(15-y)));
		gm[x][1] = (gm[x][1] | (g0<<(15-y)));
		gm[x][2] = (gm[x][2] | (b0<<(15-y)));
	}
}



void RGB_matrix::paint()
{

	// Write graphics memory to display

    *OE = 1; 	// Disable output
    write_row(row_cnt);
    *OE = 0;	// Enable output
    row_cnt++;

    if (row_cnt > 15){
      row_cnt = 0;
    }

}

void RGB_matrix::paint_all()
{
    // Write graphics memory to display

	for(uint16_t i=0; i<16; i++){
		*OE = 1; // Disable output
		write_row(i);
		*OE = 0; // Enable output
		wait_us(40);
	}
	*OE=1;
}

char RGB_matrix::put_char(int x,int y, char c){

// Define character library offsets
int offset = 3;
int width_offset = 1;
int width = (int) HD44780_6x8[offset + width_offset + (7*(c - 0x20))];

char col;
for (int i = 0; i <= width; i++){

	 // Chose a character in libray
	 col = HD44780_6x8[(offset + width_offset + (7*(c - 0x20))) + i + 1];

	 // Set pixels
	 for (int j = 0; j < 8; j++){

		 if((col & 1<<j) > 0){
	       set_pixel(x+i,y + j,_color);
		 }else{
		   set_pixel(x+i,y + j,_background);
		 }

	 }


}

return width+1;

}

void RGB_matrix::put_line(char* str, int line_number){

int len = std::strlen(str);
int char_offset = 0;

  for(int i = 0; i < len; i++){

	  char_offset += put_char(char_offset,(line_number-1)*8,str[i]);

  }
}

void RGB_matrix::put_row(char* str,uint16_t mins, int line_number){

	for(int i = 0; i<128; i++){
		for(int j = 0; j<8; j++)
			set_pixel(i,j+(line_number-1)*8,NONE);
	}

	int len = std::strlen(str);
	int char_offset = 0;
	//set_color(WHITE);
	for(int i = 0; i < len; i++){

	  char_offset += put_char(char_offset,(line_number-1)*8,str[i]);

	}
	char_offset=128-6;
	char buffer[50];
	len = sprintf(buffer,"%dmin",mins);
	int i=0;
	//set_color(PINK);
	while(i<len){
	  i++;
	  char_offset -= put_char(char_offset,(line_number-1)*8,buffer[len-i]);
	}
}

void RGB_matrix::write_row(uint16_t Row)
{

    // Write specified row (and row+8) to display. Valid input: 0 to 15.
    *ABCD=15-Row; // Set row address

    for(int col=0; col<128; col++) {
        *R1 = gm[col][0] & (1<<Row);  // Red bit, upper half
        *G1 = gm[col][1] & (1<<Row);  // Green bit, upper half
        *B1 = gm[col][2] & (1<<Row);  // Blue bit, upper half
        *R2 = gm[col][3] & (1<<Row);  // Red bit, lower half
        *G2 = gm[col][4] & (1<<Row);  // Green bit, lower half
        *B2 = gm[col][5] & (1<<Row);  // Blue bit, lower half

        *CLK = 1; // Clock with CLK pin
        *CLK = 0;
    }
   *LAT = 1;  // Latch entire row to output
    *LAT = 0;
}


