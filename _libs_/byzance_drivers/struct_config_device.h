#ifndef STRUCT_CONFIG_DEVICE_H
#define STRUCT_CONFIG_DEVICE_H

#include <stdint.h>

struct struct_config_device {  
  uint32_t  flashflag;          
  uint32_t  bootloader_report;
  uint32_t  configured;
  uint32_t  launched;
  char      alias[64];  
  char      trusted;
};

#endif /* STRUCT_CONFIG_DEVICE_H */ 
