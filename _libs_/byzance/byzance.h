#ifndef BYZANCE_H
#define BYZANCE_H


#define STRINGIFY_BUILD_ID(x) #x
#define TOSTRING_BUILD_ID(x) STRINGIFY_BUILD_ID(x)

#include "mbed.h"
#include "rtos.h"

#include "ByzanceLogger.h"
#include "Watchdog.h"
#include "Flash.h"

#include "ByzanceCore.h"
#include "macros_byzance.h"

/*
 * Supported targets:
 * TARGET_BYZANCE_IODAG3E
 */

#include "IODA_general.h"

#include "build_datetime.h"

#include "LedModule.h"

#include "byzance_main.h"

/*******************************************************************************
 *
 * 				POZOR
 *
 * Funkce get_byzance_build_id musí zůstat v .h souboru
 * jinak nebude správně fungovat detekce build_id při update binárek
 *
 ******************************************************************************/

const char * get_byzance_build_id(void){
	return TOSTRING_BUILD_ID(__BUILD_ID__);
}

#endif /* BYZANCE_H */
