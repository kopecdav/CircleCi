#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <stdint.h>

/*
 * NUMBER OF SECTORS
 */
 
#if defined(TARGET_BYZANCE_IODAG3E)
  #define FLASH_SECTORS  24
#endif 

/*
 * MEMORY MAP
 */
#if defined(TARGET_BYZANCE_IODAG3E)
// bank 1
#define ADDR_FLASH_SECTOR_0   ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1   ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2   ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3   ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4   ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5   ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6   ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7   ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8   ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9   ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10	((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11	((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */

// bank 2
#define ADDR_FLASH_SECTOR_12	((uint32_t)0x08100000) /* Base @ of Sector 12, 16 Kbyte */
#define ADDR_FLASH_SECTOR_13	((uint32_t)0x08104000) /* Base @ of Sector 13, 16 Kbyte */
#define ADDR_FLASH_SECTOR_14	((uint32_t)0x08108000) /* Base @ of Sector 14, 16 Kbyte */
#define ADDR_FLASH_SECTOR_15	((uint32_t)0x0810C000) /* Base @ of Sector 15, 16 Kbyte */
#define ADDR_FLASH_SECTOR_16	((uint32_t)0x08110000) /* Base @ of Sector 16, 64 Kbyte */
#define ADDR_FLASH_SECTOR_17	((uint32_t)0x08120000) /* Base @ of Sector 17, 128 Kbyte */
#define ADDR_FLASH_SECTOR_18	((uint32_t)0x08140000) /* Base @ of Sector 18, 128 Kbyte */
#define ADDR_FLASH_SECTOR_19	((uint32_t)0x08160000) /* Base @ of Sector 19, 128 Kbyte */
#define ADDR_FLASH_SECTOR_20	((uint32_t)0x08180000) /* Base @ of Sector 20, 128 Kbyte */
#define ADDR_FLASH_SECTOR_21	((uint32_t)0x08A00000) /* Base @ of Sector 21, 128 Kbyte */
#define ADDR_FLASH_SECTOR_22	((uint32_t)0x08C00000) /* Base @ of Sector 22, 128 Kbyte */
#define ADDR_FLASH_SECTOR_23	((uint32_t)0x08E00000) /* Base @ of Sector 23, 128 Kbyte */
#endif

void get_map(uint32_t* map);

#endif /* MEMORY_MAP_H */
