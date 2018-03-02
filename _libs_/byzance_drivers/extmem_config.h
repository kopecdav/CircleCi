#ifndef EXTMEM_CONFIG_H
#define EXTMEM_CONFIG_H

/*
 * Memory sectors size
 */
#define EXTMEM_BEGIN              		(uint32_t)0x00000000 // 0 MiB
#define EXTMEM_SIZEOF             		(uint32_t)0x00800000 // 8 MiB

#define EXTMEM_SIZEOF_BUFFER			(uint32_t)0x00200000 // 2 MiB
#define EXTMEM_SIZEOF_BACKUP			(uint32_t)0x00200000 // 2 MiB
#define EXTMEM_SIZEOF_USER				(uint32_t)0x00200000 // 2 MiB
#define EXTMEM_SIZEOF_CONFIG			(uint32_t)0x001FD000 // 2 MiB -
#define EXTMEM_SIZEOF_SECURE_INDEX		(uint32_t)0x00001000 // 4096 bytes
#define EXTMEM_SIZEOF_SECURE_JOURNAL	(uint32_t)0x00002000 // 8192 bytes

/*
 * Memory sectors adresses
 */
typedef enum {
	EXTPART_BUFFER			= (uint32_t)EXTMEM_BEGIN,																		// 0 MiB -> 2 MiB; buffer sector, used for storing binary from Homer
	EXTPART_BACKUP			= (uint32_t)EXTMEM_BEGIN +\
										EXTMEM_SIZEOF_BUFFER, // 2 MiB -> 4 MiB; backup binary
	EXTPART_USER			= (uint32_t)EXTMEM_BEGIN + \
										EXTMEM_SIZEOF_BUFFER + \
										EXTMEM_SIZEOF_BACKUP, // 4 MiB -> 6 MiB; user
	EXTPART_CONFIG			= (uint32_t)EXTMEM_BEGIN + \
										EXTMEM_SIZEOF_BUFFER + \
										EXTMEM_SIZEOF_BACKUP + \
										EXTMEM_SIZEOF_USER,	// 6 MiB -> XX MiB; configuration
	EXTPART_SECURE_INDEX	= (uint32_t)EXTMEM_BEGIN + \
										EXTMEM_SIZEOF_BUFFER + \
										EXTMEM_SIZEOF_BACKUP + \
										EXTMEM_SIZEOF_USER + \
										EXTMEM_SIZEOF_CONFIG,	// XX MiB -> 8 MiB; secure
	EXTPART_SECURE_JOURNAL	= (uint32_t)EXTMEM_BEGIN + \
										EXTMEM_SIZEOF_BUFFER + \
										EXTMEM_SIZEOF_BACKUP + \
										EXTMEM_SIZEOF_USER + \
										EXTMEM_SIZEOF_CONFIG + \
										EXTMEM_SIZEOF_SECURE_INDEX,
} Extpart_t;

/************************************
 *
 * MEMORY MAP
 *
 ************************************
 *
 *   ============
 *   | 0x00000000 |
 *   |    ...     | (Buffer)
 *   | 0x001FFFFF |
 *   |============|
 *   | 0x08020000 |
 *   |    ...     | (Backup)
 *   | 0x003FFFFF |
 *   |============|
 *   | 0x08400000 |
 *   |    ...     | (User)
 *   | 0x085FFFFF |
 *   |============|
 *   | 0x08600000 |
 *   |    ...     | (Config)
 *   | 0x087FCFFF |
 *   |============|
 *   | 0x087FD000 |
 *   |    ...     | (Journal index)
 *   | 0x087FDFFF |
 *   |============|
 *   | 0x087FE000 |
 *   |    ...     | (Journal data)
 *   | 0x087FFFFF |
 *   |============|
 *
 *
 */

                                                      
/***************************************************************
 ***************************************************************
 * 						CONFIGURATION
 ***************************************************************
 **************************************************************/

#define EXTMEM_OFFSET_STRUCT_CONFIGURATION		(uint32_t)0

/***************************************************************
 ***************************************************************
 * 						MQTT
 ***************************************************************
 **************************************************************/

#define EXTMEM_OFFSET_STRUCT_MQTT_NORMAL		(uint32_t)256

// XX bytes reserved for future use

#define EXTMEM_OFFSET_STRUCT_MQTT_BACKUP		(uint32_t)512

// XX bytes reserved for future use

#define EXTMEM_OFFSET_STRUCT_MQTT_CREDENTIALS	(uint32_t)768

// XX bytes reserved for future use

/***************************************************************
 ***************************************************************
 * 						BINARIES
 ***************************************************************
 **************************************************************/

#define EXTMEM_OFFSET_STRUCT_BOOTLOADER			(uint32_t)1024

// 48 bytes reserved for future use

#define EXTMEM_OFFSET_STRUCT_FIRMWARE			(uint32_t)1280

// 48 bytes reserved for future use

#define EXTMEM_OFFSET_STRUCT_BACKUP				(uint32_t)1536

// 48 bytes reserved for future use

#define EXTMEM_OFFSET_STRUCT_BUFFER				(uint32_t)1792

// 48 bytes reserved for future use

/***************************************************************
 ***************************************************************
 * 						DEVLIST
 ***************************************************************
 **************************************************************/

/*
 Offset a velikost (v bytech) device listu; samotny device list je alokovany od adresy 2048
 */
#define EXTMEM_OFFSET_DEVLIST_COUNTER		(uint32_t)2048
#define EXTMEM_SIZEOF_DEVLIST_COUNTER		(uint32_t)4

/*
 Offset a maximalni velikost (v bytech) device listu
 */
#define EXTMEM_OFFSET_DEVLIST_PLACE		(uint32_t)4096
#define EXTMEM_SIZEOF_DEVLIST_PLACE		(uint32_t)4096

#endif /* EXTMEM_CONFIG_H */
