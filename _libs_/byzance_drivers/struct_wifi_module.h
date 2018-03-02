#ifndef STRUCT_WIFI_MODULE_H
#define STRUCT_WIFI_MODULE_H

struct struct_wifi_module {            
  char firmware_name;
	char firmware_major;
  char firmware_minor;
  char firmware_patch;
  char mac[32];
  uint32_t espid;
  uint32_t flashid;
  uint32_t flashsize;
  uint32_t flashspeed;
};

#endif /* STRUCT_WIFI_MODULE_H */ 
