#include "Connector.h"

REGISTER_DEBUG(connector);

Connector::Connector(Serial *serial){

	_serial = serial;
	_serial->baud(115200);

	// attach incomming serial interrupt
	_serial->attach(callback(this, &Connector::_received_char), Serial::RxIrq);

}

int Connector::read(char* msg){

	size_t len;

	if(_msg_valid){

		len = _msg.length();

		if(!len) return -1;

		strcpy(msg, _msg.c_str());
		_msg_valid = false;

		_msg.clear();

		return len;
	}


	return -1;

}

int Connector::write(const char* format, ...){

	char	buffer[CONNECTOR_MESSAGE_SIZE];
	char	tmp[8];

	uint8_t	crc8 = 0;

	size_t size = 0;

	buffer[0] = 0x00;

	// sprintf to buffer
	va_list args;
	va_start (args, format);
	size += vsnprintf (buffer, CONNECTOR_MESSAGE_SIZE, format, args);
	__TRACE("write: size=%u\n", size);
	va_end (args);

	// trim original line endings
	trim_crlf(buffer);

	// calc msg CRC
	crc8 = calc_crc(buffer);

	__TRACE("write: crc = %d, len=%d\n", crc8, strlen(buffer));

	// stick crc to the end of the message
	// add crlf
	sprintf(tmp, "#%02X\r\n", crc8);
	strcat(buffer, tmp);

	__TRACE("write: after crc and newline: msg = %s, len=%d\n", buffer, strlen(buffer));

	// send it
	_serial->printf(buffer);

	return 0;

}

int Connector::lock(){
	if(_locked)
		return -1;
	_locked = true;
	return 0;
	//_serial->attach(0);
}

int Connector::unlock(){
	_locked = false;

	//_serial->attach(callback(this, &Connector::_received_char), Serial::RxIrq);

	return 0;
}

void Connector::attach_incomming(void (*function)(void)) {
	_msg_incomming = function;
}

bool Connector::get_locked(){
	return _locked;
}

void Connector::_received_char(){

	// todo: implement double buffer instead of locks

	// note: this function is interrupt
	// it's not possible to use LOG_, printf() and wait functions here!!!

	// detach interrupt
	// dont forget to attach it before every return!!!
	_serial->attach(0);

	char c = 0;

	c = _serial->getc();
	if(c=='\n'){

		// find last #
		size_t pos = _msg_buffered.find_last_of("#");

		// no # found (there is no CRC)
		if(pos==string::npos){

			// CRC is required for every message and no CRC received
			// throw away message
			#if CONNECTOR_CRC_REQUIRED
			_msg_valid = false;
			while(!lock());
			_msg.clear();
			_msg_buffered.clear();
			unlock();
			// attach incomming serial interrupt
			_serial->attach(callback(this, &Connector::_received_char), Serial::RxIrq);
			return;
			#endif

		} else {

			// CRC found
			// copy from pos+1 (after hashtag) to the end
			uint8_t crc_expected = strtoul(_msg_buffered.substr(pos+1).c_str(), NULL, 16);
			// cut off crc from the end of the message
			_msg_buffered.assign(_msg_buffered.substr(0, pos));

			uint8_t crc_calculated = 0;
			crc_calculated = calc_crc(_msg_buffered.c_str());

			// bad CRC -> throw message away
			if(crc_expected!=crc_calculated){
				_msg_valid = false;
				while(!lock());
				_msg.clear();
				_msg_buffered.clear();
				unlock();
				// attach incomming serial interrupt
				_serial->attach(callback(this, &Connector::_received_char), Serial::RxIrq);
				return;
			}

		}

		while(!lock());
		_msg.clear();
		_msg = _msg_buffered;
		_msg_buffered.clear();

		unlock();

		_msg_valid = true;
		if(_msg_incomming){
			_msg_incomming.call();
		}

	} else if(c=='\r') {
		// ignoruje \r
	} else {
		_msg_buffered += c;
	}

	// attach incomming serial interrupt
	_serial->attach(callback(this, &Connector::_received_char), Serial::RxIrq);

}
