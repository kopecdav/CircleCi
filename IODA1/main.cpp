#include "byzance.h"
#define HANDSHAKE_TIMEOUT 	5000		 // 5000 ms

enum state {
	OK = 0,
	SERIAL_CONNECTION_ERROR,
	CLOUD_CONNECTION_ERROR
};

// IO 
DigitalOut ledRed(X01);
DigitalOut ledOrn(X03);
DigitalOut ledGrn(X05);
InterruptIn btn(X00);

// Bus definition
Serial pc(SERIAL_TX, SERIAL_RX); // Serial communication with terminal UART4 (PC10(Y00), PC11(Y01)) 
Serial ioda2(X06, X07);  // Serial communiation with IODA2 UART1()

// Thread
Thread state_monitor;

// Global variables
volatile int state_cloud				= OK;
volatile int state_serial 				= OK;
volatile int ack_cloud 					= 0;
volatile int ack_serial 				= 0;
volatile bool button_state 				= false;
volatile bool digout_state 				= 0;
volatile int liveness_test 				= 0;
volatile int state_monitor_allowed 		= 1;
int loop_cnt = 0;

// Virtual IO
MESSAGE_INPUT(handshake_in, INTEGER, {
	pc.printf("[RECIEVED MESSAGE CLOUD] \"%i\" \n", arg1);
	ack_cloud = arg1;
});

/*
DIGITAL_INPUT(led_red, {
	ledRed = value;
	pc.printf("led_green: %d \n", value)
	;
});
*/

MESSAGE_OUTPUT(handshake, INTEGER)

// STRING INT INT FLOAT FLOAT
MESSAGE_OUTPUT(report_state_mTRING, INTEGER, INTEGER, FLOAT, FLOAT);
DIGITAL_OUTPUT(digout);

/**
 * Thread function used for state indication
 */
void statemonitor_function() {

	while (1) {

		if(state_monitor_allowed){

			// Everything OK
			if((state_cloud == OK) && (state_serial == OK)) {
				ledGrn = 1;
			} else {
				ledGrn = 0;
			}

			// Cloud connecton
			if(state_cloud == OK){
				ledOrn = 0;
			}else{
				ledOrn = !ledOrn;
			}

			// Serial connection
			if(state_serial == OK){
				ledRed = 0;
			}else{
				ledRed = !ledRed;
			}

		}

		Thread::wait(300);
	}

}

/**
 * Callback called when button 1 is pushed, trigger test 1 
 */
void test_liveness() {

	liveness_test = 1;
	pc.printf("[LIVENESS TEST] triggered \n");

}

/**
 * Report state of the IODA, send a message with uptime to cloud
 */
int report_state() {

	uint32_t up_t;
	float voltage		= 0.0;
	float temp			= 0.0;
	int waiting 		= 0;
	Timer t;

	time_t connected = ByzanceCore::connected_time();
	const char* ip_add = Byzance::get_ip_address();
	Byzance::get_uptime(&up_t);
	Byzance::get_supply_voltage(&voltage);
	Byzance::get_core_temp(&temp);

	uint32_t days = up_t / 86400;
	uint32_t hours = (up_t - (days * 86400)) / 3600;
	uint32_t min = (up_t - (days * 86400) - (hours * 3600)) / 60;

	pc.printf("CONNECTED TIME %ld \n", connected);

	pc.printf("[STATE REPORT] \t ip_address: %s \n, \t uptime: %d days  %dh  %dm : %d \n \t voltage: %f \n \t CPU temp: %f \n",ip_add, days, hours, min, up_t % 60, voltage, temp);
	pc.printf("[STATE REPORT] send data to IODA2 \n");
	Console::log("[STATE REPORT]: ip_address %s, uptime %d days %d : %d : %d, voltage: %f, CPU temp : %f \n",ip_add, days, hours, min, up_t % 60, voltage, temp);


	// STRING INT INT FLOAT FLOAT
	report_state_msg(ip_add, up_t, connected, voltage, temp);

	t.start();
	ack_serial = 0;
	ioda2.printf("s:%s:%d:%d:%f:%f\n", ip_add, up_t, connected, voltage, temp);

	while (waiting < HANDSHAKE_TIMEOUT) {

			if (ack_serial != 0) {
				break;
			}

		waiting = t.read_ms();
		Thread::wait(10);

	}
	t.stop();


	if (ack_serial > 0){

		pc.printf("[STATE REPORT] Serial acknoledge recieved\n");
		return waiting;

	}else{

		pc.printf("[ERROR] IODA2 did not acknoledge a serial communication\n");
		return -1;

	}

}

/**
 * Test communication with cloud by sending a message. This message will be sent back by cloud
 * and IODA will compute latency of this communication
 * 
 * process: IODA send a random number as a SYN signal and wait for acknowledge signal (ACK) which
 * is equal to (SYN + 1). If the signal is recieved before timeout, handshake is succesfull and 
 * send back second ACK back to cloud ACK + 1 
 * 
 * \return - integer latency in ms or -1 in tha case of error
 */
int cloud_com_test() {

	int syn = rand() % 100 + 1;
	int waiting = 0;
	bool get_ack = false;
	Timer t;

	// Send SYN handshake signal 
	pc.printf("[CLOUD_COM_TEST] Send SYN signla: %d \n", syn);
	handshake(syn);


	//ledRed = 1;
	t.start();
	while (waiting < HANDSHAKE_TIMEOUT) {

		if (ack_cloud != 0) {
			get_ack = true;
			break;
		}

		waiting = t.read_ms();
		Thread::wait(10);

	}

	t.stop();
	//ledRed = 0;

	if (get_ack && (ack_cloud == (syn + 1))) {

		pc.printf("[CLOUD_COM_TEST] Recieved ACK : %d \n", ack_cloud);
		pc.printf("[CLOUD_COM_TEST] Handshake was succesfull with latency %d \n", waiting);

		// Send ACK back to cloud
		handshake(ack_cloud + 1);
		ack_cloud = 0;
		return waiting;

	} else if (get_ack) {

		pc.printf("[CLOUD_COM_TEST] Recieved ACK : %d \n", ack_cloud);
		pc.printf("[ERROR] Handshake was not succesfull - wrong acknowledge %d \n ", ack_cloud);
		ack_cloud = 0;
		return -1;

	} else {

		pc.printf("[ERROR] Handshake was not succesfull - TIMEOUT \n");
		ack_cloud = 0;
		return -1;

	}

}

void rx_interrupt() {

	while (ioda2.readable()) {

		// Read char from serial
		char c = ioda2.getc();
		// acknoledge flag
		if (c == '!'){
			ack_serial = 1;
		}
	}
}


void init() {

	pc.baud(115200);
	ioda2.baud(115200);
	ioda2.attach(&rx_interrupt, Serial::RxIrq);

	// Add callback to button 1  
	btn.fall(&test_liveness);
	btn.mode(PullUp); 					// toto musi byt za attachnuti callbacku

	// Turn on state monitor
	state_monitor.start(statemonitor_function);
	state_monitor.set_priority(osPriorityIdle);

}

void loop() {

	// Start test on button click
	if (liveness_test) {

		liveness_test = 0;
		state_monitor_allowed = 0;

		int wait_time = 150;
		int number_of_blinks = 4;

		ledRed = 0;
		ledGrn = 0;
		ledOrn = 0;

		for(int i=0; i < number_of_blinks; i++){

			ledGrn = 1;
			ledOrn = 0;
			ledRed = 0;
			Thread::wait(wait_time);
			ledGrn = 0;
			ledOrn = 1;
			ledRed = 0;
			Thread::wait(wait_time);
			ledGrn = 0;
			ledOrn = 0;
			ledRed = 1;
			Thread::wait(wait_time);

		}

		ledRed = 0;
		ledGrn = 0;
		ledOrn = 0;

		state_monitor_allowed = 1;

	}

	if (loop_cnt >= 30) {

		// Report IODA1 state to display driver (IODA2) by serial line 
		if (report_state() == -1){
			state_serial = SERIAL_CONNECTION_ERROR;
		}else{
			state_serial = OK;
		}

		if (cloud_com_test() == -1) {
			state_cloud = CLOUD_CONNECTION_ERROR;
		}else{
			state_cloud = OK;
		}

		// Reset loop counter
		loop_cnt = 0;
	}

	Thread::wait(200);
	loop_cnt++;

}

