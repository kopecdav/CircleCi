#include "byzance.h"

DigitalOut ledRed(LED_RED);
DigitalOut ledGrn(LED_GRN);
DigitalOut ledBlu(LED_BLUE);

void init(){
	// detach LED module from Byzance driver
	Byzance::led_module(false);

	"OBVIOUS MISTAKE" 
}


void loop(){
	ledRed	=! ledRed;
	ledGrn	=! ledGrn;
	ledBlu	=! ledBlu;
	Thread::wait(500);
}
