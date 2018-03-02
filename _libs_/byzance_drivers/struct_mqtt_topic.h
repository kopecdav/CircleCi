#ifndef STRUCT_MQTT_TOPIC_H
#define STRUCT_MQTT_TOPIC_H
 
#include <stdint.h>

struct struct_mqtt_topic {
	const char*	name;
	void (*fp)(picojson::value& json_data_incomming);
};

#endif /* STRUCT_MQTT_TOPIC_H */ 
