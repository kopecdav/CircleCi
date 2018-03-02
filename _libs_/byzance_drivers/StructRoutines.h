#ifndef STRUCT_ROUTINES_H
#define STRUCT_ROUTINES_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "struct_defaults.h"
#include "struct_binary.h"
#include "struct_mqtt.h"
#include "struct_config_ioda.h"
#include "struct_config_device.h"
#include "struct_wifi_credentials.h"
#include "struct_wifi_module.h"

bool 		struct_is_empty(void *data, uint32_t size);
uint32_t	struct_compare(void *str1, void *str2, uint32_t size);
void 		struct_clean(void *data, uint32_t size);

void defaults_struct_bin(struct_binary *bin);
void defaults_mqtt_connection(mqtt_connection *conn);
void defaults_mqtt_credentials(mqtt_credentials *cred);
void defaults_struct_config_ioda(struct_config_ioda *conf);
void defaults_struct_config_device(struct_config_device *conf);
void defaults_struct_wifi_module(struct_wifi_module *wifi);
void defaults_struct_wifi_credentials(struct_wifi_credentials *wifi);

#endif /* STRUCT_ROUTINES_H */
