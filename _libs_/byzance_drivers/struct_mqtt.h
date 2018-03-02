#ifndef STRUCT_MQTT_H
#define STRUCT_MQTT_H
 
#include <stdint.h>

struct mqtt_connection {
  char		hostname[128];
  uint32_t  port;
};

struct mqtt_credentials{
	char	username[48];
	char	password[48];
};

#endif /* STRUCT_MQTT_H */ 
