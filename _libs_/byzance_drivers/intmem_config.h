#ifndef INTMEM_CONFIG_H
#define INTMEM_CONFIG_H

#include <stdint.h>

/*
 * Memory sectors size
 */
#if defined(TARGET_BYZANCE_IODAG3E)
	#define INTMEM_SIZEOF             	(uint32_t)0x00200000 //    2 MiB
	#define INTMEM_SIZEOF_BOOTLOADER	(uint32_t)0x00010000 //   64 KiB
	#define INTMEM_SIZEOF_FIRMWARE		(uint32_t)0x001F0000 // 1984 KiB
#else
  #error Intmem not configured
#endif

/*
 * Some useful macros
 */
#define INTMEM_BEGIN              	(uint32_t)0x08000000
#define INTMEM_END                  INTMEM_BEGIN + INTMEM_SIZEOF

/*
 * Memory sectors adresses
 */
typedef enum
{
#if defined(TARGET_BYZANCE_IODAG3E)
	INTPART_BOOTLOADER   = (uint32_t)INTMEM_BEGIN,
	INTPART_FIRMWARE     = (uint32_t)INTMEM_BEGIN + INTMEM_SIZEOF_BOOTLOADER
#else 
#error "Target not defined in intmem_config.h"
#endif
} Intpart_t;

/***************************************************************
 ***************************************************************
 * 						CONFIGURATION
 ***************************************************************
 **************************************************************/

#define INTMEM_OFFSET_STRUCT_CONFIGURATION		(uint32_t)0

/***************************************************************
 ***************************************************************
 * 						BINARIES
 ***************************************************************
 **************************************************************/

#define INTMEM_OFFSET_STRUCT_BOOTLOADER			(uint32_t)1024

// 48 bytes reserved for future use

#define INTMEM_OFFSET_STRUCT_FIRMWARE			(uint32_t)1280

// 48 bytes reserved for future use

#define INTMEM_OFFSET_STRUCT_BACKUP				(uint32_t)1536

// 48 bytes reserved for future use

#endif /* INTMEM_CONFIG_H */
