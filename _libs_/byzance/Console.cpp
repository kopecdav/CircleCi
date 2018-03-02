#include "Console.h"

bool Console::_enabled = 0;

bool Console::error(const char* format, ...){

	if(!_enabled) return 0;

	char buffer[CONSOLE_MESSAGE_SIZE];

	// sprintf to buffer
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, CONSOLE_MESSAGE_SIZE, format, args);
	va_end (args);

	Console::send(CONSOLE_LOGTYPE_ERROR, buffer);

	return 1;
}

bool Console::warn(const char* format, ...){

	if(!_enabled) return 0;

	char buffer[CONSOLE_MESSAGE_SIZE];

	// sprintf to buffer
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, CONSOLE_MESSAGE_SIZE, format, args);
	va_end (args);

	Console::send(CONSOLE_LOGTYPE_WARN, buffer);

	return 1;
}

bool Console::info(const char* format, ...){

	if(!_enabled) return 0;

	char buffer[CONSOLE_MESSAGE_SIZE];

	// sprintf to buffer
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, CONSOLE_MESSAGE_SIZE, format, args);
	va_end (args);

	Console::send(CONSOLE_LOGTYPE_INFO, buffer);

	return 1;
}

bool Console::log(const char* format, ...){

	if(!_enabled) return 0;

	char buffer[CONSOLE_MESSAGE_SIZE];

	// sprintf to buffer
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, CONSOLE_MESSAGE_SIZE, format, args);
	va_end (args);

	Console::send(CONSOLE_LOGTYPE_LOG, buffer);

	return 1;
}

void Console::enable(bool en){
	_enabled = en;
}

bool Console::enabled(){
	return _enabled;
}

bool Console::send(Logtype_t type, char* data){

	if(!_enabled) return 0;

	// create JSON
	picojson::object json;
	switch(type){
	case CONSOLE_LOGTYPE_ERROR:
		json["error"] = picojson::value(data);
		break;
	case CONSOLE_LOGTYPE_WARN:
		json["warn"] = picojson::value(data);
		break;
	case CONSOLE_LOGTYPE_INFO:
		json["info"] = picojson::value(data);
		break;
	case CONSOLE_LOGTYPE_LOG:
		json["log"] = picojson::value(data);
		break;
	}

	string str = picojson::value(json).serialize();

	message_struct 	msg;
	msg.topic+="console_out/message";
	msg.payload = str;

	ByzanceClient::send_message(&msg);

	return 0;
}
