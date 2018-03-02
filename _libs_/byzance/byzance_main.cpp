#include "byzance_main.h"    

#include "mbed.h"

#include "IODA_general.h"

REGISTER_DEBUG(main);

/*
 * Special function which is called before everything
 * Used e.g for logger, because it needs to be enabled before Byzance::init() and connect
 */
__weak void pre_init(){

}

/*
 * This is for user implementation
 */
__weak void init(){

}

/*
 * This is for user implementation
 */
__weak void loop(){

}

bool byzance_disabled = false;

/*
 * Real main
 */
int main(int argc, char* argv[]){

	UNUSED(argc);
	UNUSED(argv);

	pre_init();

	if(!Byzance::is_disabled()){
		Byzance::init();
    	Byzance::connect();
	}

    osThreadSetPriority(osThreadGetId(), osPriorityNormal);

    // init for user
    init();

    // Main loop
	while(true){

		__TRACE("going to loop\n");

		// loop for user
		loop();

		Thread::yield();
    }
}
