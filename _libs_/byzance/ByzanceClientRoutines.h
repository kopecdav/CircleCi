#ifndef BYZANCE_CLIENT_ROUTINES_H_
#define BYZANCE_CLIENT_ROUTINES_H_

#include "mbed.h"
#include "MqttBuffer.h"

/**
 * Nastavení topiců, do kterých se subscribuje klient
 */
#define CLIENT_PREFIX			(const char*)""
#define CLIENT_TOPIC_DIGITAL 	(const char*)"/d_in/#"
#define CLIENT_TOPIC_ANALOG 	(const char*)"/a_in/#"
#define CLIENT_TOPIC_MESSAGE	(const char*)"/m_in/#"
#define CLIENT_TOPIC_COMMAND	(const char*)"/command_in/#"
#define CLIENT_TOPIC_SETTINGS	(const char*)"/settings_in/+"
#define CLIENT_TOPIC_INFO		(const char*)"/info_in/+"
#define CLIENT_TOPIC_CONSOLE	(const char*)"/console_in/#"
#define CLIENT_TOPIC_DEVICE		(const char*)"/device_in/#"

bool client_fill_topics(char* client_id, char** topics);
bool client_clear_buffer(MqttBuffer *buffer);

#endif /* BYZANCE_CLIENT_ROUTINES_H_ */
