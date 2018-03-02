#include "ByzanceCore.h"
#include "ByzanceClient.h"

REGISTER_DEBUG(core);

// static definitions
std::vector<ByzanceDigitalInputRegistration>                                                     *ByzanceCore::_digitalInputs;
std::vector<ByzanceAnalogInputRegistration>                                                      *ByzanceCore::_analogInputs;
std::vector<ByzanceMessageInputRegistration>                                                     *ByzanceCore::_messageInputs;

Callback<void(void)>	ByzanceCore::_restart_follows_callback;

Callback<void(bool)>	ByzanceCore::_bin_busy_callback;

#define MAX_HOOK_IODAG3	10460631
#define MAX_HOOK_DEVICE 1758912

#define MAX_HOOK MAX_HOOK_IODAG3

Ticker		ByzanceCore::_ticker;
uint32_t	hook_cnt;
uint8_t 	ByzanceCore::cpu_load = 0;

time_t		ByzanceCore::uptime = 0;

time_t		ByzanceCore::_connected = 0;

bool 		ByzanceCore::_connected_increase = false;

bool		ByzanceCore::restart_pending = 0;

time_t 		ByzanceCore::restart_sec = 0;

bool		ByzanceCore::_led_core_controlled = 1;

bool		ByzanceCore::_bin_busy;

void ByzanceCore::hook_attach(void){
	Thread::attach_idle_hook(&ByzanceCore::idle_hook);
}

void ByzanceCore::ticker_init(void){
	ByzanceCore::_ticker.attach(&ByzanceCore::ticker_call, 1.0);
}

// is called every second
void ByzanceCore::ticker_call(void){

	/*
	 * Calculate CPU usage
	 */
	if(hook_cnt>MAX_HOOK){
		hook_cnt=MAX_HOOK;
	}

	cpu_load = (uint8_t)((hook_cnt*100)/MAX_HOOK);
	cpu_load=100-cpu_load;

	hook_cnt = 0;

	uptime++;

	/*
	 * Solve pending restart
	 */
	if(restart_pending){

		restart_sec--;

		if(restart_sec){
			__WARNING("restart will follow in %d sec\n", restart_sec);
		} else {

			// notify user that restart will follow
			if(_restart_follows_callback){
				_restart_follows_callback.call();
			}

			// in callback user could increment restart time
			// check again if happened so or restart should follow
			if(!restart_sec){
				__WARNING("restarting right now\n");
				// wait to write previous warning
				Thread::wait(10);
				HAL_NVIC_SystemReset();
			} else {
				__WARNING("restart time has been incremented to %d sec\n", restart_sec);
			}
		}
	}

	/*
	 * LED module
	 */

	if(_led_core_controlled){
		if(BinManager::is_busy()){
			LedModule::set(LED_COLOR_BUSY, LED_CODE_BUSY);

			// bin manager is busy and wasn't busy -> call callback about change
			if(!_bin_busy){
				if(_bin_busy_callback){
					_bin_busy_callback.call(true);
				}
				_bin_busy = true;
			}

		} else {
			if(ByzanceClient::is_connected()){
				LedModule::set(LED_COLOR_CONNECTED, LED_CODE_CONNECTED);
			} else {
				LedModule::set(LED_COLOR_DISCONNECTED, LED_CODE_DISCONNECTED);
			}

			// bin manager is't busy and was busy -> call callback about change
			if(_bin_busy){
				if(_bin_busy_callback){
					_bin_busy_callback.call(false);
				}

				_bin_busy = false;
			}
		}
	} else {
		// disable LED module
		LedModule::set(LED_COLOR_BLACK);
	}

	if(_connected_increase){
		_connected++;
	}

}

void ByzanceCore::idle_hook(void){
	hook_cnt++;
}

/*
 * S touto funkcí si zaregistruju ditigální vstup, který pak bude vidět v Blocku
 */
void ByzanceCore::add_digital_input_registration(ByzanceDigitalInputRegistration *registration) {
	if (ByzanceCore::_digitalInputs == NULL) {
		ByzanceCore::_digitalInputs = new vector<ByzanceDigitalInputRegistration>;
	}
    ByzanceCore::_digitalInputs->push_back(*registration);
}

/*
 * S touto funkcí si zaregistruju analogový vstup, který pak bude vidět v Blocku
 */
void ByzanceCore::add_analog_input_registration(ByzanceAnalogInputRegistration *registration) {
	if (ByzanceCore::_analogInputs == NULL) {
		ByzanceCore::_analogInputs = new vector<ByzanceAnalogInputRegistration>;
	}
    ByzanceCore::_analogInputs->push_back(*registration);
}

/*
 * S touto funkcí si zaregistruju message vstup, který pak bude vidět v Blocku
 */
void ByzanceCore::add_message_input_registration(ByzanceMessageInputRegistration *registration) {
	if (ByzanceCore::_messageInputs == NULL) {
		ByzanceCore::_messageInputs = new vector<ByzanceMessageInputRegistration>;
	}
    ByzanceCore::_messageInputs->push_back(*registration);
}

/*
 * Funkce slouží k odeslání digitální hodnoty
 */
int ByzanceCore::digital_output_set_value(const char *name, bool value) {

	int rc = 0;

	char 				buffer[MQTT_MAX_SINGLE_BUFF];
    message_struct 		msg;

    snprintf(buffer, MQTT_MAX_TOPIC_SIZE, "d_out/%s", name);
	msg.topic = buffer;
	snprintf(buffer, MQTT_MAX_PAYLOAD_SIZE, (value ? "1" : "0"));
	msg.payload = buffer;

	rc = ByzanceClient::send_message(&msg);

	return rc;

}

/*
 * Funkce slouží k odeslání analogové hodnoty z Yody do Homera (Blocka)
 */
int ByzanceCore::analog_output_set_value(const char *name, float value) {

	int rc = 0;

	char 				buffer[MQTT_MAX_SINGLE_BUFF];
    message_struct 		msg;

    snprintf(buffer, MQTT_MAX_TOPIC_SIZE, "a_out/%s", name);
	msg.topic = buffer;
	snprintf(buffer, MQTT_MAX_PAYLOAD_SIZE, "%f", value);
	msg.payload = buffer;

	rc = ByzanceClient::send_message(&msg);

	return rc;

}

/*
 * Funkce slouží k odeslání message z Yody do Homera (Blocka)
 */
int ByzanceCore::message_output_set_value(const char *name, ByzanceSerializer* serializer) {

	int rc = 0;

	// message validation failed
	if(serializer->validate()){
		__ERROR("serialization failed\n");
		return -1;
	}

	char 				buffer[MQTT_MAX_SINGLE_BUFF];
	message_struct 		msg;
	snprintf(buffer, MQTT_MAX_TOPIC_SIZE, "m_out/%s", name);
	msg.topic = buffer;
	msg.payload.assign(serializer->getValue(), serializer->getValueLen());

	rc = ByzanceClient::send_message(&msg);

	BYZANCE_UNUSED_ARG(serializer);

	return rc;
}

void ByzanceCore::catastropic_failure(int code){

	error("catastrophic failure; code = %d\n", code);
}

time_t ByzanceCore::restart(time_t sec){

	if(!restart_pending){
		__WARNING("restart will follow in %d sec\n", sec);
		restart_pending = true;
	}

	// if requested timeout is bigger than current timeout, its updated to the bigger one
	if(sec>restart_sec){
		__WARNING("restart timeout was updated from %d to %d sec\n", restart_sec, sec);
		restart_sec = sec;
	} else {
		__WARNING("restart timeout is already set to %d sec\n", restart_sec);
	}

	return restart_sec;
}

void ByzanceCore::connected_start(){
	_connected_increase = true;
}

void ByzanceCore::connected_stop(){
	_connected_increase = false;
	_connected = 0;
}

time_t ByzanceCore::connected_time(){
	return _connected;
}

void ByzanceCore::led_module(bool state){
	_led_core_controlled = state;
}

void ByzanceCore::attach_restart_follows(void (*function)(void)) {
	ByzanceCore::_restart_follows_callback = callback(function);
}

void ByzanceCore::attach_bin_busy(void (*function)(bool)) {
	ByzanceCore::_bin_busy_callback = callback(function);
}
