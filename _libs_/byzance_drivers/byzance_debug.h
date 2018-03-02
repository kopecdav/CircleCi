#ifndef _BYZANCE_DEBUG_H
#define _BYZANCE_DEBUG_H

#include "ByzanceLogger.h"

/*
 * ZAP/VYP makra, nehrabat do toho
 */
#ifndef ON
#define ON	1
#endif

#ifndef OFF
#define OFF 0
#endif

/*********************************************************************************************
 *
 * DEBUG JEDNOTLIVÝCH KOMPONENT
 * KAŽDOU LZE ZAPNOUT NEBO VYPNOUT A NASTAVIT JEJÍ LEVEL
 *
 * ABY TO FUNGOVALO, JE TŘEBA MÍT V MAINU ZAPLÝ DEBUGGER A SPRÁVNĚ NASTAVENÝ GLOBÁLNÍ LEVEL
 *
 *********************************************************************************************/

/*
 * the real main file (it runs init and loop functions)
 */
constexpr bool main_DEBUGGING = true;
constexpr DebugLevel_t main_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Countdown class
 */
constexpr bool countdown_DEBUGGING = true;
constexpr DebugLevel_t countdown_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Core class common for all targets
 */
constexpr bool Commander_DEBUGGING = true;
constexpr DebugLevel_t Commander_DEBUGGING_LEVEL = DEBUG_LEVEL_TRACE;

/*
 * Core class common for all targets
 */
constexpr bool core_DEBUGGING = true;
constexpr DebugLevel_t core_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Hlavní Byzance vlákno
 */
constexpr bool byzance_DEBUGGING = true;
constexpr DebugLevel_t byzance_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * MQTT Client
 */
constexpr bool Client_DEBUGGING = true;
constexpr DebugLevel_t Client_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * LowpanBR
 */
constexpr bool LowpanBR_DEBUGGING = true;
constexpr DebugLevel_t LowpanBR_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * DNS64
 */
constexpr bool DNS64_DEBUGGING = true;
constexpr DebugLevel_t DNS64_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * HTTP Server
 */
constexpr bool HttpServer_DEBUGGING = true;
constexpr DebugLevel_t HttpServer_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * HTML Page parser
 */
constexpr bool HTMLParser_DEBUGGING = true;
constexpr DebugLevel_t HTMLParser_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Byzance Parser (incomming digital/analog/meessage) packets
 */
constexpr bool ByzanceParser_DEBUGGING = true;
constexpr DebugLevel_t ByzanceParser_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Byzance Serializer (outcomming digital/analog/meessage) packets
 */
constexpr bool ByzanceSerializer_DEBUGGING = true;
constexpr DebugLevel_t ByzanceSerializer_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * PAHO knihovna
 */
constexpr bool paho_DEBUGGING = true;
constexpr DebugLevel_t paho_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * MQTTSocket - vec, co se predava do PAHO
 */
constexpr bool MQTTSocket_DEBUGGING = true;
constexpr DebugLevel_t MQTTSocket_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Watchdog
 */
constexpr bool Watchdog_DEBUGGING = true;
constexpr DebugLevel_t Watchdog_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * LED module component
 */
constexpr bool LedModule_DEBUGGING = true;
constexpr DebugLevel_t LedModule_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Connector - komponenta na posilani dat mezi dvema deskama
 */
constexpr bool connector_DEBUGGING = true;
constexpr DebugLevel_t connector_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Configurator - trida pro udrzovani configu
 */
constexpr bool Configurator_DEBUGGING = true;
constexpr DebugLevel_t Configurator_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Byzance Worker
 */
constexpr bool Worker_DEBUGGING = true;
constexpr DebugLevel_t Worker_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Všechno, co ve Workerovi spadne do topicu Commands
 */
constexpr bool MQTTCommands_DEBUGGING = true;
constexpr DebugLevel_t MQTTCommands_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Všechno, co ve Workerovi spadne do topicu Settings
 */
constexpr bool MQTTSettings_DEBUGGING = true;
constexpr DebugLevel_t MQTTSettings_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Všechno, co ve Workerovi spadne do topicu Info
 */
constexpr bool MQTTInfo_DEBUGGING = true;
constexpr DebugLevel_t MQTTInfo_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Modul externí flashky - low level
 */
constexpr bool SpiFlash_DEBUGGING = true;
constexpr DebugLevel_t SpiFlash_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Modul externí flashky - high level
 */
constexpr bool ExtMem_DEBUGGING = true;
constexpr DebugLevel_t ExtMem_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Modul interní flashky
 */
constexpr bool IntMem_DEBUGGING = true;
constexpr DebugLevel_t IntMem_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Flash API pro uživatele
 */
constexpr bool flash_DEBUGGING = true;
constexpr DebugLevel_t flash_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * MQTT buffer třída
 */
constexpr bool MqttBuffer_DEBUGGING = true;
constexpr DebugLevel_t MqttBuffer_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * JSON třída
 */
constexpr bool json_DEBUGGING = true;
constexpr DebugLevel_t json_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * Binary manager
 */
constexpr bool BinManager_DEBUGGING = true;
constexpr DebugLevel_t BinManager_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * MQTT manager
 */
constexpr bool MqttManager_DEBUGGING = true;
constexpr DebugLevel_t MqttManager_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * I2C pin expander
 */
constexpr bool TCA6424A_DEBUGGING = true;
constexpr DebugLevel_t TCA6424A_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * LED driver
 */
constexpr bool TLC59116_DEBUGGING = true;
constexpr DebugLevel_t TLC59116_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * TK3G lib
 */
constexpr bool TK3G_DEBUGGING = true;
constexpr DebugLevel_t TK3G_DEBUGGING_LEVEL = DEBUG_LEVEL_TRACE;

/*
 * Seven segment driver on LED shield
 */
constexpr bool TripleSevenSeg_DEBUGGING = true;
constexpr DebugLevel_t TripleSevenSeg_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * TLSSocket
 */
constexpr bool TLSSocket_DEBUGGING = true;
constexpr DebugLevel_t TLSSocket_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * CLV motor driver
 */
constexpr bool RD_DEBUGGING = true;
constexpr DebugLevel_t RD_DEBUGGING_LEVEL = DEBUG_LEVEL_WARNING;

/*
 * DS1820 lib debug
 */
constexpr bool DS1820_DEBUGGING = true;
constexpr DebugLevel_t DS1820_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * MFRC522 - RFID
 */
constexpr bool MFRC522_DEBUGGING = true;
constexpr DebugLevel_t MFRC522_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

/*
 * SHT21 - humidity and temperature
 */
constexpr bool SHT21_DEBUGGING = true;
constexpr DebugLevel_t SHT21_DEBUGGING_LEVEL = DEBUG_LEVEL_WARNING;

/*
 * ms5637 - sensor
 */
constexpr bool MS5637_DEBUGGING = true;
constexpr DebugLevel_t MS5637_DEBUGGING_LEVEL = DEBUG_LEVEL_WARNING;

/*
 * PCA9536 - i2c expander
 */
constexpr bool PCA9536_DEBUGGING = true;
constexpr DebugLevel_t PCA9536_DEBUGGING_LEVEL = DEBUG_LEVEL_TRACE;

/*
 * I2CWrapper
 */
constexpr bool I2CWrapper_DEBUGGING = true;
constexpr DebugLevel_t I2CWrapper_DEBUGGING_LEVEL = DEBUG_LEVEL_ERROR;

#if BYZANCE_BOOTLOADER
	#define DEBUG_HEADER_ENABLED	0
#else
	#define DEBUG_HEADER_ENABLED	1
#endif

/*
 * DEBUG MACROS
 */

#if DEBUG_HEADER_ENABLED

	#define __LOG(...) __LOG_WRAP(__FUNCTION__, __LINE__, __VA_ARGS__)
	#define __ERROR(...) __ERROR_WRAP(__FUNCTION__, __LINE__, __VA_ARGS__)
	#define __WARNING(...) __WARNING_WRAP(__FUNCTION__, __LINE__, __VA_ARGS__)
	#define __INFO(...) __INFO_WRAP(__FUNCTION__, __LINE__, __VA_ARGS__)
	#define __DEBUG(...) __DEBUG_WRAP(__FUNCTION__, __LINE__, __VA_ARGS__)
	#define __TRACE(...) __TRACE_WRAP(__FUNCTION__, __LINE__, __VA_ARGS__)

	#define REGISTER_DEBUG_FUNC(component, level) \
	template <typename... Args> \
	inline void __##level##_WRAP(const char* func, int line, Args&& ... args) { \
		if ( component##_DEBUGGING && component##_DEBUGGING_LEVEL >= DEBUG_LEVEL_##level) \
			ByzanceLogger::new_log(DEBUG_LEVEL_##level, #component, func, line, std::forward<Args>(args)...); \
	}
#else

	#define __LOG(...) __LOG_WRAP(__VA_ARGS__)
	#define __ERROR(...) __ERROR_WRAP(__VA_ARGS__)
	#define __WARNING(...) __WARNING_WRAP(__VA_ARGS__)
	#define __INFO(...) __INFO_WRAP(__VA_ARGS__)
	#define __DEBUG(...) __DEBUG_WRAP(__VA_ARGS__)
	#define __TRACE(...) __TRACE_WRAP(__VA_ARGS__)

	#define REGISTER_DEBUG_FUNC(component, level) \
	template <typename... Args> \
	inline void __##level##_WRAP(Args&& ... args) { \
		if ( component##_DEBUGGING && component##_DEBUGGING_LEVEL >= DEBUG_LEVEL_##level) \
			ByzanceLogger::log(std::forward<Args>(args)...); \
		}
#endif

/*
 * Used in firmware
 */
#define REGISTER_DEBUG(component) \
    REGISTER_DEBUG_FUNC(component, LOG) \
    REGISTER_DEBUG_FUNC(component, ERROR) \
    REGISTER_DEBUG_FUNC(component, WARNING) \
    REGISTER_DEBUG_FUNC(component, INFO) \
    REGISTER_DEBUG_FUNC(component, DEBUG) \
    REGISTER_DEBUG_FUNC(component, TRACE)

#endif /* _BYZANCE_DEBUG_H */
