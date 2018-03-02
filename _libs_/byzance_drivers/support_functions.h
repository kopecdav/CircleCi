#ifndef SUPPORT_FUNCTIONS_H
#define SUPPORT_FUNCTIONS_H

#include "mbed.h"
#include <string>
#include <vector>

#include "byzance_config.h"
#include "macros_bootloader.h"

#define STRINGIFY_TARGET(x) #x
#define TOSTRING_TARGET(x) STRINGIFY_TARGET(x)

/*
 * Used for determining FULL ID
 */

#define STM32_UUID ((uint32_t *)0x1FFF7A10)		// STM32F4 - RM0090.pdf
#define FULL_ID_LENGTH				((uint8_t)24)	// Delka stringu s Full ID

void get_mac_address(char *addr);
void get_mac_address(uint64_t * mac_int);
void mac_to_bytes(const char* mac, char* bytes);

int get_subpart(string base, string& sub, uint8_t level, char c);
vector<int> find_location(string sample, char c);

void get_fullid(char * pcID);

uint32_t get_revision(void);

uint32_t get_chip_seed(void);
uint32_t get_chip_hash(void);

bool timeout_expired(int32_t time_now, int32_t time_old, uint32_t time_out);

void convert_mqtt_to_protocol(uint8_t * result_data, uint16_t * result_data_len, const char * input_subtopic_name, uint16_t input_subtopic_len, const char * input_payload, uint16_t input_payload_len);
void convert_protocol_to_mqtt(char * result_topic_name, uint16_t *result_topic_len, char * result_payload, uint16_t * result_payload_len, uint8_t * input_data, char * input_pre_topic_name);

#if !defined(BYZANCE_BOOTLOADER)
void get_name_state(Thread::State state, char* name);
void get_name_priority(osPriority priority, char* name);
#endif

uint8_t calc_crc(const char* buffer);
uint8_t trim_crlf(char* data);

#endif /* SUPPORT_FUNCTIONS_h */
