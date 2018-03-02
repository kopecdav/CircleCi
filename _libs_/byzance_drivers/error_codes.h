#ifndef _BYZANCE_ERROR_CODES_H
#define _BYZANCE_ERROR_CODES_H

/*********************************************
 * README:
 *
 * Error kódy jsou určeny pro funkce, co budou jakkoliv komunikovat s Homerem.
 * Ostatní třídy a funkce si svoje rozhraní budou specifikovat lokálně.
 *
 * Error kódy mají následující pravidla pro vytváření:
 * - vždy je to 32 bitové číslo.
 * - makro má vždy prefix ERROR_CODE_
 * - je to cislo maximalne 8 cifer dlouhe
 * - horni ctyri cifry jsou stejné pro jednu skupinu chybových kódů (group id).
 * - spodni ctyri cifry se iterují podle konkrétní chyby.
 * - pokud nenastala chyba, funkce/třída/metoda vrací ERROR_CODE_OK = 0
 * - pokud nastala univerzální neidentifikovatelná chyba, vrací se ERROR_CODE_GENERAL_ERROR = 99999999
 *
 *********************************************/

/*********************************************
 *        UNIVESAL ERROR CODES
 *        (group id 0)
 *        (group id 9999)
 *********************************************/
#ifndef ERROR_CODE_OK
#define ERROR_CODE_OK								(uint32_t) 0
#endif

#ifndef ERROR_CODE_UNKNOWN_TOPIC
#define ERROR_CODE_UNKNOWN_TOPIC					(uint32_t) 1
#endif

#ifndef ERROR_CODE_MISSING_LABEL
#define ERROR_CODE_MISSING_LABEL					(uint32_t) 2
#endif

#ifndef ERROR_CODE_UNKNOWN_LABEL
#define ERROR_CODE_UNKNOWN_LABEL					(uint32_t) 3
#endif

#ifndef ERROR_CODE_MALLOC_ERROR
#define ERROR_CODE_MALLOC_ERROR						(uint32_t) 4
#endif

#ifndef ERROR_CODE_LOCK_ERROR
#define ERROR_CODE_LOCK_ERROR						(uint32_t) 5
#endif

#ifndef ERROR_CODE_GENERAL_ERROR
#define ERROR_CODE_GENERAL_ERROR					(uint32_t) 99999999
#endif

/*********************************************
 *        ERROR CODES FROM JSON VALIDATOR
 *        (group id 1)
 *********************************************/
#ifndef ERROR_CODE_JSON_INVALID
#define ERROR_CODE_JSON_INVALID						(uint32_t) 10000
#endif

#ifndef ERROR_CODE_JSON_MSGID_MISSING
#define ERROR_CODE_JSON_MSGID_MISSING				(uint32_t) 10001
#endif

#ifndef ERROR_CODE_JSON_NOT_OBJECT
#define ERROR_CODE_JSON_NOT_OBJECT					(uint32_t) 10002
#endif

/*********************************************
 *        ERROR CODES TOPIC INFO
 *        (group id 2)
 *********************************************/

// unknown errors so far

/*********************************************
 *        ERROR CODES TOPIC COMMAND
 *        (group id 3)
 *********************************************/

#ifndef ERROR_CODE_COMMAND_UNKNOWN_COMPONENT
#define ERROR_CODE_COMMAND_UNKNOWN_COMPONENT	   	(uint32_t) 30000
#endif

#ifndef ERROR_CODE_COMMAND_UNKNOWN_FULLID
#define ERROR_CODE_COMMAND_UNKNOWN_FULLID			(uint32_t) 30001
#endif

#ifndef ERROR_CODE_COMMAND_UNKNOWN_SHORTID
#define ERROR_CODE_COMMAND_UNKNOWN_SHORTID			(uint32_t) 30002
#endif

#ifndef ERROR_CODE_COMMAND_UNEXPECTED_PART
#define ERROR_CODE_COMMAND_UNEXPECTED_PART		   	(uint32_t) 30003
#endif

#ifndef ERROR_CODE_COMMAND_CRC_ERROR
#define ERROR_CODE_COMMAND_CRC_ERROR			   	(uint32_t) 30004
#endif

#ifndef ERROR_CODE_COMMAND_ADD_FAILED
#define ERROR_CODE_COMMAND_ADD_FAILED				(uint32_t) 30005
#endif

#ifndef ERROR_CODE_COMMAND_REMOVE_FAILED
#define ERROR_CODE_COMMAND_REMOVE_FAILED			(uint32_t) 30006
#endif

#ifndef ERROR_CODE_COMMAND_DEVICE_BACKUP_FAILED
#define ERROR_CODE_COMMAND_DEVICE_BACKUP_FAILED			(uint32_t) 30007
#endif

#ifndef ERROR_CODE_COMMAND_INVALID_DEVICE_STATE
#define ERROR_CODE_COMMAND_INVALID_DEVICE_STATE			(uint32_t) 30008
#endif

#ifndef ERROR_CODE_COMMAND_BIN_DUPLICITY
#define ERROR_CODE_COMMAND_BIN_DUPLICITY				(uint32_t) 30009
#endif

#ifndef ERROR_CODE_COMMAND_ERASE_IN_PROGRESS
#define ERROR_CODE_COMMAND_ERASE_IN_PROGRESS			(uint32_t) 30010
#endif

#ifndef ERROR_CODE_COMMAND_AUTOBACKUP_ENABLED
#define ERROR_CODE_COMMAND_AUTOBACKUP_ENABLED			(uint32_t) 30011
#endif

#ifndef ERROR_CODE_COMMAND_BIN_MISSING
#define ERROR_CODE_COMMAND_BIN_MISSING					(uint32_t) 30012
#endif

#ifndef ERROR_CODE_COMMAND_BIN_INVALID
#define ERROR_CODE_COMMAND_BIN_INVALID					(uint32_t) 30013
#endif

/*********************************************
 *        ERROR CODES TOPIC SETTINGS
 *        (group id 4)
 *********************************************/

#ifndef ERROR_CODE_SETTINGS_HOSTNAME_ERROR
#define ERROR_CODE_SETTINGS_HOSTNAME_ERROR			(uint32_t) 40000
#endif

#ifndef ERROR_CODE_SETTINGS_TIME_REFUSED
#define ERROR_CODE_SETTINGS_TIME_REFUSED			(uint32_t) 40001
#endif

/*********************************************
 *        ERROR CODES FROM SENDER
 *        (group id 5)
 *********************************************/


/*********************************************
 *        ERROR CODES FOR EXTMEM
 *        (group id 6)
 *********************************************/
/*
 * Extmem general read error
 */
#ifndef ERROR_CODE_EXTMEM_READ_ERROR
#define ERROR_CODE_EXTMEM_READ_ERROR				(uint32_t) 60000
#endif

/*
 * Extmem general write error
 */
#ifndef ERROR_CODE_EXTMEM_WRITE_ERROR
#define ERROR_CODE_EXTMEM_WRITE_ERROR				(uint32_t) 60001
#endif

/*
 * Extmem general write error
 */
#ifndef ERROR_CODE_EXTMEM_ERASE_ERROR
#define ERROR_CODE_EXTMEM_ERASE_ERROR				(uint32_t) 60002
#endif

/*********************************************
 *        ERROR CODES FOR INTMEM
 *        (group id 7)
 *********************************************/
/*
 * Extmem general read error
 */
#ifndef ERROR_CODE_INTMEM_READ_ERROR
#define ERROR_CODE_INTMEM_READ_ERROR				(uint32_t) 70000
#endif

/*
 * Extmem general write error
 */
#ifndef ERROR_CODE_INTMEM_WRITE_ERROR
#define ERROR_CODE_INTMEM_WRITE_ERROR				(uint32_t) 70001
#endif

/*
 * Extmem general write error
 */
#ifndef ERROR_CODE_INTMEM_ERASE_ERROR
#define ERROR_CODE_INTMEM_ERASE_ERROR				(uint32_t) 70002
#endif

#endif /* _BYZANCE_ERROR_CODES_H */
