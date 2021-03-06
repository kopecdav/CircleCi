#ifndef ERROR_RESOLVER_H
#define ERROR_RESOLVER_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "error_codes.h"
#include "macros_bootloader.h"

#define MACRO_NAME(x)	#x

struct struct_error {
	uint32_t     code;
	const char*  name;
};

class ErrorResolver{
  public:
    ErrorResolver();
    int get_name(uint32_t code, char* name);
    
  private:
     uint32_t _errcount; 

};

/*
 * LIST OF ERRORS
 */
const struct_error errors[] = {
	{ERROR_CODE_OK,								MACRO_NAME(ERROR_CODE_OK)},
	{ERROR_CODE_UNKNOWN_TOPIC,					MACRO_NAME(ERROR_CODE_UNKNOWN_TOPIC)},
	{ERROR_CODE_MISSING_LABEL,					MACRO_NAME(ERROR_CODE_MISSING_LABEL)},
	{ERROR_CODE_UNKNOWN_LABEL,					MACRO_NAME(ERROR_CODE_UNKNOWN_LABEL)},
	{ERROR_CODE_MALLOC_ERROR,					MACRO_NAME(ERROR_CODE_MALLOC_ERROR)},
	{ERROR_CODE_LOCK_ERROR,						MACRO_NAME(ERROR_CODE_LOCK_ERROR)},
	{ERROR_CODE_GENERAL_ERROR,					MACRO_NAME(ERROR_CODE_GENERAL_ERROR)},
	{ERROR_CODE_JSON_INVALID,					MACRO_NAME(ERROR_CODE_JSON_INVALID)},
	{ERROR_CODE_JSON_MSGID_MISSING,				MACRO_NAME(ERROR_CODE_JSON_MSGID_MISSING)},
	{ERROR_CODE_JSON_NOT_OBJECT, 				MACRO_NAME(ERROR_CODE_JSON_NOT_OBJECT)},
	{ERROR_CODE_COMMAND_UNKNOWN_COMPONENT,		MACRO_NAME(ERROR_CODE_COMMAND_UNKNOWN_COMPONENT)},
	{ERROR_CODE_COMMAND_UNKNOWN_FULLID, 		MACRO_NAME(ERROR_CODE_COMMAND_UNKNOWN_FULLID)},
	{ERROR_CODE_COMMAND_UNKNOWN_SHORTID, 		MACRO_NAME(ERROR_CODE_COMMAND_UNKNOWN_SHORTID)},
	{ERROR_CODE_COMMAND_UNEXPECTED_PART, 		MACRO_NAME(ERROR_CODE_COMMAND_UNEXPECTED_PART)},
	{ERROR_CODE_COMMAND_CRC_ERROR, 				MACRO_NAME(ERROR_CODE_COMMAND_CRC_ERROR)},
	{ERROR_CODE_COMMAND_ADD_FAILED, 			MACRO_NAME(ERROR_CODE_COMMAND_ADD_FAILED)},
	{ERROR_CODE_COMMAND_REMOVE_FAILED, 			MACRO_NAME(ERROR_CODE_COMMAND_REMOVE_FAILED)},
	{ERROR_CODE_COMMAND_DEVICE_BACKUP_FAILED,	MACRO_NAME(ERROR_CODE_COMMAND_DEVICE_BACKUP_FAILED)},
	{ERROR_CODE_COMMAND_INVALID_DEVICE_STATE,	MACRO_NAME(ERROR_CODE_COMMAND_INVALID_DEVICE_STATE)},
	{ERROR_CODE_COMMAND_BIN_DUPLICITY, 			MACRO_NAME(ERROR_CODE_COMMAND_BIN_DUPLICITY)},
	{ERROR_CODE_COMMAND_ERASE_IN_PROGRESS, 		MACRO_NAME(ERROR_CODE_COMMAND_ERASE_IN_PROGRESS)},
	{ERROR_CODE_COMMAND_AUTOBACKUP_ENABLED,		MACRO_NAME(ERROR_CODE_COMMAND_AUTOBACKUP_ENABLED)},
	{ERROR_CODE_COMMAND_BIN_MISSING,			MACRO_NAME(ERROR_CODE_COMMAND_BIN_MISSING)},
	{ERROR_CODE_COMMAND_BIN_INVALID,			MACRO_NAME(ERROR_CODE_COMMAND_BIN_INVALID)},
	{ERROR_CODE_SETTINGS_HOSTNAME_ERROR,		MACRO_NAME(ERROR_CODE_SETTINGS_HOSTNAME_ERROR)},
	{ERROR_CODE_SETTINGS_TIME_REFUSED,			MACRO_NAME(ERROR_CODE_SETTINGS_TIME_REFUSED)},
	{ERROR_CODE_EXTMEM_READ_ERROR,				MACRO_NAME(ERROR_CODE_EXTMEM_READ_ERROR)},
	{ERROR_CODE_EXTMEM_WRITE_ERROR,				MACRO_NAME(ERROR_CODE_EXTMEM_WRITE_ERROR)},
	{ERROR_CODE_EXTMEM_ERASE_ERROR,				MACRO_NAME(ERROR_CODE_EXTMEM_ERASE_ERROR)},
	{ERROR_CODE_INTMEM_READ_ERROR,				MACRO_NAME(ERROR_CODE_INTMEM_READ_ERROR)},
	{ERROR_CODE_INTMEM_WRITE_ERROR,				MACRO_NAME(ERROR_CODE_INTMEM_WRITE_ERROR)},
	{ERROR_CODE_INTMEM_ERASE_ERROR,				MACRO_NAME(ERROR_CODE_INTMEM_ERASE_ERROR)},
};


#endif /* ERROR_RESOLVER_H */
