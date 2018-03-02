#ifndef STRUCT_BINARY_H
#define STRUCT_BINARY_H

#include <stdint.h>
#include "enum_binstate.h"
//sizeof(struct_binary) ==  4+4+48+32+4+32+32 = 156


#define BINARY_BUILD_ID_MAX_LENGTH		((uint8_t) 48)	// usually 36 characters
#define BINARY_VERSION_MAX_LENGTH		((uint8_t) 32)	//
#define BINARY_USR_VERSION_MAX_LENGTH	((uint8_t) 32)	//
#define BINARY_USR_NAME_MAX_LENGTH		((uint8_t) 32)	//

struct struct_binary {

	uint32_t	size;
	uint32_t	crc;
	char		build_id[BINARY_BUILD_ID_MAX_LENGTH];
	char		version[BINARY_VERSION_MAX_LENGTH];
	uint32_t	timestamp;
	char		usr_version[BINARY_USR_VERSION_MAX_LENGTH];
	char		usr_name[BINARY_USR_NAME_MAX_LENGTH];
  
	bin_state_t state;
};

#endif /* STRUCT_BINARY_H */ 
