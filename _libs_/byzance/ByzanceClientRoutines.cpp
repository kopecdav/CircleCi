#include "ByzanceClientRoutines.h"

bool client_fill_topics(char* client_id, char** topics){

	topics[0] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_DIGITAL) + 1);
	sprintf(topics[0], "%s%s", client_id, CLIENT_TOPIC_DIGITAL);

	topics[1] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_ANALOG) + 1);
	sprintf(topics[1], "%s%s", client_id, CLIENT_TOPIC_ANALOG);

	topics[2] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_MESSAGE) + 1);
	sprintf(topics[2], "%s%s", client_id, CLIENT_TOPIC_MESSAGE);

	topics[3] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_COMMAND) + 1);
	sprintf(topics[3], "%s%s", client_id, CLIENT_TOPIC_COMMAND);

	topics[4] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_SETTINGS) + 1);
	sprintf(topics[4], "%s%s", client_id, CLIENT_TOPIC_SETTINGS);

	topics[5] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_INFO) + 1);
	sprintf(topics[5], "%s%s", client_id, CLIENT_TOPIC_INFO);

	topics[6] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_CONSOLE) + 1);
	sprintf(topics[6], "%s%s", client_id, CLIENT_TOPIC_CONSOLE);

	topics[7] = (char*)malloc(strlen(client_id) + strlen(CLIENT_TOPIC_DEVICE) + 1);
	sprintf(topics[7], "%s%s", client_id, CLIENT_TOPIC_DEVICE);

	return 0;
}

bool client_clear_buffer(MqttBuffer *buffer){
	if(buffer->lock() == MQTT_BUFFER_OK){
		buffer->clear();
		buffer->unlock();
		return 0;
	}

	return 1;
}
