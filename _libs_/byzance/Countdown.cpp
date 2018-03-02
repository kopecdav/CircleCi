#include "Countdown.h"

REGISTER_DEBUG(countdown);

Countdown::Countdown(void){
	__TRACE("%p: constructor without param\n", this);
}

Countdown::~Countdown(void){
	__TRACE("%p: destructor; remaining: t=%d\n", this, interval_end_ms - t.read_ms());
}

Countdown::Countdown(unsigned long ms) {
	__TRACE("%p: constructor to %u ms\n", this, ms);
    countdown_ms(ms);   
}

void Countdown::countdown(unsigned long seconds) {
	__TRACE("%p: method to %u ms\n", this, seconds);
    countdown_ms(seconds * 1000L);
}

void Countdown::countdown_ms(unsigned long ms) {

	__TRACE("%p: setting to %u ms\n", this, ms);

	t.stop();
    interval_end_ms = ms;
    t.reset();
    t.start();
}

bool Countdown::expired() {

	// countdown not set -> expired
	if(interval_end_ms == 0){
		__TRACE("%p: expired: not set\n", this);
		return 1;

	// timer is lower than 0
	} else if (t.read_ms() < 0){
		__TRACE("%p: expired: expired 0\n", this);
		return 1;

	} else if ((unsigned long)t.read_ms() >= interval_end_ms){
		__TRACE("%p: expired: expired\n", this);
		return 1;

	// not expired
	} else {
		__TRACE("%p: expired: not expired; t=%d\n", this, interval_end_ms - t.read_ms());
		return 0;
	}
}

unsigned long Countdown::left_ms() {

    if(interval_end_ms>t.read_ms()){
    	__TRACE("%p: remaining: t=%d\n", this, interval_end_ms - t.read_ms());
    	return interval_end_ms - t.read_ms();
    }
    
    __TRACE("%p: remaining 0\n", this);

    return 0;
    
}
