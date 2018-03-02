#include "byzance.h"

#define SERIAL_BUFFER_SIZE  100
#define HANDSHAKE_TIMEOUT	3000

enum state {
	OK = 0,
	SERIAL_CONNECTION_ERROR,
	CLOUD_CONNECTION_ERROR
};

struct ioda{

	int state_serial 			= OK;
	int state_cloud 			= OK;
	float voltage 				= 5.0;
	float CPU_temp				= 0.0;
	int connected_time			= 0;
	int uptime 					= 0;
	std::string ip 				= "0.0.0.0";

};



// Global variables need for communication over serial
char buffer[SERIAL_BUFFER_SIZE];
char buffer_copy[SERIAL_BUFFER_SIZE];
char *parser;
int buff_pointer;
char c;

int loop_cnt 					 	= 0;

// Flags
volatile int ack_cloud 					= 0;
int complete_line					= 0;
int reported 						= 0;
int state_monitor_allowed 			= 1;
int report_over_serial 				= 1;

// IO
DigitalOut ledRed(X01);
DigitalOut ledOrn(X03);
DigitalOut ledGrn(X05);


// Bus definitions
Serial pc(SERIAL_TX,SERIAL_RX); // PC TX, RX
Serial ioda1_serial(X06,X07);  // UART3 TX, RX

// Virtual IO
MESSAGE_INPUT(handshake_in, INTEGER, {
	pc.printf("[RECIEVED MESSAGE CLOUD] \"%i\" \n", arg1);
	ack_cloud = arg1;
});

MESSAGE_OUTPUT(handshake, INTEGER);


Thread state_monitor;
Ticker ioda1_monitor;
ioda ioda1;
ioda ioda2;


/**
 * Thread function used for state indication
 */
void statemonitor_function() {

	while (1) {

		if(state_monitor_allowed){

			// Everything OK
			if((ioda2.state_cloud == OK) && (ioda2.state_serial == OK)) {
				ledGrn = 1;
			} else {
				ledGrn = 0;
			}

			// Cloud connecton
			if(ioda2.state_cloud == OK){
				ledOrn = 0;
			}else{
				ledOrn = !ledOrn;
			}

			// Serial connection
			if(ioda2.state_serial == OK){
				ledRed = 0;
			}else{
				ledRed = !ledRed;
			}

		}

		Thread::wait(300);
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

void ioda1_monitor_fnc(){

	if ((ioda1.state_serial == OK) && !reported){
		ioda1.state_serial = SERIAL_CONNECTION_ERROR;
		report_over_serial = 0;
	}else if (ioda1.state_serial == SERIAL_CONNECTION_ERROR && reported){
		ioda1.state_serial = OK;
		report_over_serial = 1;
	}

	// Reset ioda1 reported flag
	reported = 0;
}

/**
 * Parse line read from serial
 */
void parse_line(){

	// Line format: words separate with ':'
	// if start with c -> command, s-> status

	// ip = strtok(line_buffer,":");
	// uptime = strtok(0,":");

	// parse first word
	parser = strtok(buffer_copy,":");

	if (parser[0] == 'c'){

		pc.printf("Incoming command from IODA1\n");
		Console::log("Incoming command from IODA1");

	}else if(parser[0] == 's'){

		pc.printf("Incoming status report from IODA1\n");
		Console::log("Incoming status report from IODA1");

		// Set reported flag
		reported = 1;

	}else{

		pc.printf("Incoming line from IODA1 not reckognized\n");
		Console::log("Incoming line from IODA1 not reckognized");

	}

}

void rx_interrupt(){

	while((ioda1_serial.readable()) && ((buff_pointer < SERIAL_BUFFER_SIZE) && (complete_line == 0))){

		// Read char from serial
		c = ioda1_serial.getc();

		// detect end of line
		if ((c == '\n') && (complete_line == 0)){

			complete_line = 1;

		}else if (complete_line == 0) {

			buffer[buff_pointer] = c;
			buff_pointer++;

		}
	}
}

void pre_init(){

	ByzanceLogger::init(&pc);
	pc.baud(115200);

	ByzanceLogger::set_level(DEBUG_LEVEL_TRACE);
	ByzanceLogger::enable_prefix(false);

}

void init(){


	pc.printf("init value of voltage %f \n", ioda1.voltage);
	complete_line = 0;
	ioda1_serial.baud(115200);
	ioda1_serial.attach(&rx_interrupt, Serial::RxIrq);


	state_monitor.start(statemonitor_function);
	state_monitor.set_priority(osPriorityIdle);


	ioda1_monitor.attach(&ioda1_monitor_fnc, 15.0);

}


void loop() {


	// If the line is ready copy buffer and process the line
	if (complete_line || (buff_pointer == (SERIAL_BUFFER_SIZE-1))){

		// Disable serial line interrupts
		NVIC_DisableIRQ(USART3_IRQn);

		if (buff_pointer > 0){
			memcpy(&buffer_copy, &buffer, buff_pointer);
		}
		// Set buffer pointer
		complete_line = 0;
		buff_pointer = 0;

		// Enadble serial interrupts
		NVIC_EnableIRQ(USART3_IRQn);

		// Send acknoledge and change flag to process line
		ioda1_serial.putc('!');

		// process line
		parse_line();

	}


	if(!report_over_serial && (ioda2.state_cloud == OK)){
		// Serial broken - report over cloud

	}

	// Test cloud communication
	if (loop_cnt > 30){

		if(cloud_com_test() == -1){
			ioda2.state_cloud = CLOUD_CONNECTION_ERROR;
		}else{
			ioda2.state_cloud = OK;
		}
		loop_cnt = 0;
	}



	// Display changes


	Thread::wait(200);
	loop_cnt++;
}
