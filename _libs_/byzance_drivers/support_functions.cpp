#include "support_functions.h"
#include "otp.h"

void get_mac_address(char* addr) {
    char mac[6];
    int startAddress = 0x1FFF7800;
    int macOffset =  0;
   /* while(macOffset < (32-6)) {
        if ((*(volatile char *)(startAddress + macOffset)) != 0x00) {
            break;
        }
        ++macOffset;
    }   */
    mac[0] = *(volatile char *)(startAddress + macOffset + 0);
    mac[1] = *(volatile char *)(startAddress + macOffset + 1);
    mac[2] = *(volatile char *)(startAddress + macOffset + 2);
    mac[3] = *(volatile char *)(startAddress + macOffset + 3);
    mac[4] = *(volatile char *)(startAddress + macOffset + 4);
    mac[5] = *(volatile char *)(startAddress + macOffset + 5);
    sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


void get_mac_address(uint64_t * mac_int){
    char mac[6];
    int startAddress = 0x1FFF7800;
    int macOffset =  0;
    while(macOffset < (32-6)) {
        if ((*(volatile char *)(startAddress + macOffset)) != 0x00) {
            break;
        }
        ++macOffset;
    }
    mac[0] = *(volatile char *)(startAddress + macOffset + 0);
    mac[1] = *(volatile char *)(startAddress + macOffset + 1);
    mac[2] = *(volatile char *)(startAddress + macOffset + 2);
    mac[3] = *(volatile char *)(startAddress + macOffset + 3);
    mac[4] = *(volatile char *)(startAddress + macOffset + 4);
    mac[5] = *(volatile char *)(startAddress + macOffset + 5);

    *mac_int  = ((uint64_t)mac[0]) << 40;
    *mac_int += ((uint64_t)mac[1]) << 32;
    *mac_int += ((uint64_t)mac[2]) << 24;
    *mac_int += ((uint64_t)mac[3]) << 16;
    *mac_int += ((uint64_t)mac[4]) <<  8;
    *mac_int += ((uint64_t)mac[5]) <<  0;
}

void mac_to_bytes(const char* mac, char* bytes){
	char buff[3];

	strncpy(buff, mac+0, 2);
	buff[2] = 0;
	bytes[0] = strtol(buff, NULL, 16);
	strncpy(buff, mac+3, 2);
	buff[2] = 0;
	bytes[1] = strtol(buff, NULL, 16);
	strncpy(buff, mac+6, 2);
	buff[2] = 0;
	bytes[2] = strtol(buff, NULL, 16);
	strncpy(buff, mac+9, 2);
	buff[2] = 0;
	bytes[3] = strtol(buff, NULL, 16);
	strncpy(buff, mac+12, 2);
	buff[2] = 0;
	bytes[4] = strtol(buff, NULL, 16);
	strncpy(buff, mac+15, 2);
	buff[2] = 0;
	bytes[5] = strtol(buff, NULL, 16);
	bytes[6] = 0x00;

}

/*
 * return 0 = OK
 * cokoliv jiného = error
 */
/*
 *
 */
int get_subpart(string base, string& sub, uint8_t level, char c){

	int start = 0;
	int len = 0;

	// naplní do vektoru pozice lomítka
    vector<int> results = find_location(base, c);

    // pokud není dostatek lomítek pro požadovaný level, vyhodí chybu
    if(results.size()<level) return 1;

    // tahám nultý level a za ním není lomítko
    if((level==0)&&(results.size()==0)) {
    	start = 0;
    	len = base.length();
    }

    // tahám nultý level a za je lomítko
    else if (level==0){
    	start = 0;
    	len = results.at(0);
    }

    // pokud chci vytáhnout poslední level, vytahuju subtopic od posledního lomítka do konce
    else if (level==results.size())
    {
    	start = results.at(level-1) + 1;
    	len = base.length() - results.at(level-1) - 1;
    }

    // vytahuju neco mezi tim
    else
    {
    	start = results.at(level-1) + 1;
    	len = results.at(level) - results.at(level-1) - 1;
    }

    sub = base.substr(start, len);

	return 0;
}


/*
 * najde všechny pozice konkrétního znaku a uloží do vektoru
 */
vector<int> find_location(string sample, char c)
{
	vector<int> characterLocations;
    for(uint32_t i =0; i < sample.size(); i++)
        if(sample[i] == c)
        	characterLocations.push_back(i);

    return characterLocations;
}

/**
 * Chip ID ma 96b, tj. 24 Bytu (MAC adresa je 48b a je to hexa string 12 Bytu).
 * U STM32F0 to zacina na adrese 0x1FFF F7AC.
 * https://techoverflow.net/blog/2015/02/03/reading-the-stm32-unique-device-id-in-c/
 *
 * Vstupem je pointer na char o delce min. 25 znaku.
 */
void get_fullid(char * pcID){
	char pcPart[10];

	if (pcID == NULL){
		return;
	}

	sprintf(pcPart, "%08X", (int)STM32_UUID[0]);
	strcpy(pcID, pcPart);

	sprintf(pcPart, "%08X", (int)STM32_UUID[1]);
	strcat(pcID, pcPart);

	sprintf(pcPart, "%08X", (int)STM32_UUID[2]);
	strcat(pcID, pcPart);
	pcID[FULL_ID_LENGTH] = '\0';
	return;
}

uint32_t get_revision(void){

	// read it little endian
	uint32_t tmp32 = 0;
	tmp32 |= OTP::read(1, 3)<<24;
	tmp32 |= OTP::read(1, 2)<<16;
	tmp32 |= OTP::read(1, 1)<<8;
	tmp32 |= OTP::read(1, 0)<<0;
	return tmp32;
}

uint32_t get_chip_seed(void){
	return (uint32_t)(STM32_UUID[2] + STM32_UUID[1] + STM32_UUID[0]);
}


uint32_t get_chip_hash(void){
	uint32_t pom = ((STM32_UUID[2] ^ STM32_UUID[1]) ^ STM32_UUID[0]);

	// Pokud by to nahodou byla nula, zmenim ji. V Devicech se pocita s tim, ze nula to byt nesmi.
	if(pom == 0){
		return 1;
	}
	return pom;
}




bool timeout_expired(int32_t time_now, int32_t time_old, uint32_t time_out){
	if(abs(time_now - time_old) > ((int32_t) time_out)){
		return true;
	}else{
		return false;
	}
}


/**
 * Funkce na konverzi MQTT dat na data pro prime odeslani protokolem na sbernice.
 *
 * 1. a 2. byte = delka topicu
 * 3. a 4. byte = delka payloadu
 * ostatní byty = za sebou topic name, ukoncovaci znak a pak payload
 *
 * @param result_data			Data, kam se vysledek konverze ulozi.
 * @param result_data_len		Delka dat po konverzi.
 * @param input_subtopic_name	Vstupni informace - nazev MQTT topicu, ale pouze cast relavantni pro device, tj. 2. a 3. uroven MQTT topicu.
 * @param input_subtopic_len	Delka vstupniho mqtt topicu
 * @param input_payload			Vstupni informace - payload topicu.
 * @param input_payload_len		Vstupni informace - delka pyloadu.
*/
void convert_mqtt_to_protocol(uint8_t * result_data, uint16_t * result_data_len, const char * input_subtopic_name, uint16_t input_subtopic_len, const char * input_payload, uint16_t input_payload_len){
	//uint16_t subtopic_name_len = strlen((const char *)input_subtopic_name);
	uint32_t result_data_length = 4 + input_subtopic_len + input_payload_len + 1;	// +1 za vlozeny '\0' mezi topicName a payload

	//ByzanceLogger::log("result_data_length = %d", result_data_length);

	// Pripravim si delky jednotlivych casti.
	result_data[0] = (uint8_t) ((input_subtopic_len >> 8) & 0xFF);   	// MSB
	result_data[1] = (uint8_t) ((input_subtopic_len >> 0) & 0xFF);   	// LSB
	result_data[2] = (uint8_t) ((input_payload_len >> 8) & 0xFF);   	// MSB
	result_data[3] = (uint8_t) ((input_payload_len >> 0) & 0xFF);   	// LSB

	// Prekopiruju topicName a pak payload do spolecne promenne.
	memcpy((uint8_t *)result_data + 4, (const char *)input_subtopic_name, input_subtopic_len);
	result_data[4 + input_subtopic_len] = '\0';	// Pridam si ukoncovaci znak.
	memcpy((uint8_t *)result_data + 4 + input_subtopic_len + 1, (const char *)input_payload, input_payload_len);	// +1 za vlozeny '\0' mezi topicName a payload

	// Predam info o celkove delce dat.
	*result_data_len = result_data_length;

}


/**
 * Funkce na konverzi dat z protokolu na sbernici na MQTT zpravu.
 *
 * 1. a 2. byte = delka topicu
 * 3. a 4. byte = delka payloadu
 * ostatní byty = za sebou topic name, ukoncovaci znak a pak payload
 *
 * @param result_topic_name		Vysledny MQTT topic name.
 * @param result_topic_len		Vysledna delka MQTT topic name
 * @param result_payload		Payload MQTT topicu.
 * @param result_payload_len	Delka payloadu MQTT.
 * @param input_data			Vstupni data z protokolu.
 * @param input_pre_topic_name	Textovy prefix, ktery se prida pred MQTT topic name ziskany z dat "input_data".
*/
/*
void convert_protocol_to_mqtt(char * result_topic_name, uint16_t * result_topic_len, char * result_payload, uint16_t * result_payload_len, uint8_t * input_data, char * input_pre_topic_name){
	uint16_t usTopicLen = (input_data[0] << 8) + input_data[1];
	uint16_t usPayloadLen = (input_data[2] << 8) + input_data[3];

	strcpy(result_topic_name, input_pre_topic_name);							// Prvni cast topicu (Full ID yody atp).
	strncat(result_topic_name, (const char *) &input_data[4], usTopicLen);		// Pripojim cast topicu z device.
	*result_topic_len = strlen(result_topic_name);								// Spocitam delku.
	*result_topic_len =  *result_topic_len > MQTT_MAX_TOPIC_SIZE ? MQTT_MAX_TOPIC_SIZE : *result_topic_len;	// Delku si overim na max. velikost.

	// Vytvorim payload.
	memcpy(result_payload, &input_data[4 + usTopicLen + 1], usPayloadLen);		// +1, ptz mezi topicName a payload je '\0'
	result_payload[usPayloadLen] = '\0';
	*result_payload_len = usPayloadLen;											// Predam delku dat v payloadu.
	*result_payload_len =  *result_payload_len > MQTT_MAX_PAYLOAD_SIZE ? MQTT_MAX_PAYLOAD_SIZE : *result_payload_len;	// Delku si overim na max. velikost.
}
*/

#if !defined(BYZANCE_BOOTLOADER)

void get_name_state(Thread::State state, char* name){
	switch (state){
	case Thread::Inactive:
		sprintf(name, "Inactive");
		break;
	case Thread::Ready:
		sprintf(name, "Ready");
		break;
	case Thread::Running:
		sprintf(name, "Running");
		break;
	case Thread::WaitingDelay:
		sprintf(name, "WaitingDelay");
		break;
	case Thread::WaitingJoin:
		sprintf(name, "WaitingJoin");
		break;
	case Thread::WaitingThreadFlag:
		sprintf(name, "WaitingThreadFlag");
		break;
	case Thread::WaitingEventFlag:
		sprintf(name, "WaitingEventFlag");
		break;
	case Thread::WaitingMutex:
		sprintf(name, "WaitingMutex");
		break;
	case Thread::WaitingSemaphore:
		sprintf(name, "WaitingSemaphore");
		break;
	case Thread::WaitingMemoryPool:
		sprintf(name, "WaitingMemoryPool");
		break;
	case Thread::WaitingMessageGet:
		sprintf(name, "WaitingMessageGet");
		break;
	case Thread::WaitingMessagePut:
		sprintf(name, "WaitingMessagePut");
		break;
	case Thread::WaitingInterval:
		sprintf(name, "WaitingInterval");
		break;
	case Thread::WaitingOr:
		sprintf(name, "WaitingOr");
		break;
	case Thread::WaitingAnd:
		sprintf(name, "WaitingAnd");
		break;
	case Thread::WaitingMailbox:
		sprintf(name, "WaitingMailbox");
		break;
	case Thread::Deleted:
		sprintf(name, "Deleted");
		break;
	default:
		sprintf(name, "=== error ===");
		break;
	}
}

void get_name_priority(osPriority priority, char* name){
	switch(priority){
	case osPriorityNone:
		sprintf(name, "osPriorityNone");
		break;
	case osPriorityIdle:
		sprintf(name, "osPriorityIdle");
		break;
	case osPriorityLow:
		sprintf(name, "osPriorityLow");
		break;
	case osPriorityLow1:
		sprintf(name, "osPriorityLow1");
		break;
	case osPriorityLow2:
		sprintf(name, "osPriorityLow2");
		break;
	case osPriorityLow3:
		sprintf(name, "osPriorityLow3");
		break;
	case osPriorityLow4:
		sprintf(name, "osPriorityLow4");
		break;
	case osPriorityLow5:
		sprintf(name, "osPriorityLow5");
		break;
	case osPriorityLow6:
		sprintf(name, "osPriorityLow6");
		break;
	case osPriorityLow7:
		sprintf(name, "osPriorityLow7");
		break;
	case osPriorityBelowNormal:
		sprintf(name, "osPriorityBelowNormal");
		break;
	case osPriorityBelowNormal1:
		sprintf(name, "osPriorityBelowNormal1");
		break;
	case osPriorityBelowNormal2:
		sprintf(name, "osPriorityBelowNormal2");
		break;
	case osPriorityBelowNormal3:
		sprintf(name, "osPriorityBelowNormal3");
		break;
	case osPriorityBelowNormal4:
		sprintf(name, "osPriorityBelowNormal4");
		break;
	case osPriorityBelowNormal5:
		sprintf(name, "osPriorityBelowNormal5");
		break;
	case osPriorityBelowNormal6:
		sprintf(name, "osPriorityBelowNormal6");
		break;
	case osPriorityBelowNormal7:
		sprintf(name, "osPriorityBelowNormal7");
		break;
	case osPriorityNormal:
		sprintf(name, "osPriorityNormal");
		break;
	case osPriorityNormal1:
		sprintf(name, "osPriorityNormal1");
		break;
	case osPriorityNormal2:
		sprintf(name, "osPriorityNormal2");
		break;
	case osPriorityNormal3:
		sprintf(name, "osPriorityNormal3");
		break;
	case osPriorityNormal4:
		sprintf(name, "osPriorityNormal4");
		break;
	case osPriorityNormal5:
		sprintf(name, "osPriorityNormal5");
		break;
	case osPriorityNormal6:
		sprintf(name, "osPriorityNormal6");
		break;
	case osPriorityNormal7:
		sprintf(name, "osPriorityNormal7");
		break;
	case osPriorityAboveNormal:
		sprintf(name, "osPriorityAboveNormal");
		break;
	case osPriorityAboveNormal1:
		sprintf(name, "osPriorityAboveNormal1");
		break;
	case osPriorityAboveNormal2:
		sprintf(name, "osPriorityAboveNormal2");
		break;
	case osPriorityAboveNormal3:
		sprintf(name, "osPriorityAboveNormal3");
		break;
	case osPriorityAboveNormal4:
		sprintf(name, "osPriorityAboveNormal4");
		break;
	case osPriorityAboveNormal5:
		sprintf(name, "osPriorityAboveNormal5");
		break;
	case osPriorityAboveNormal6:
		sprintf(name, "osPriorityAboveNormal6");
		break;
	case osPriorityAboveNormal7:
		sprintf(name, "osPriorityAboveNormal7");
		break;
	case osPriorityHigh:
		sprintf(name, "osPriorityHigh");
		break;
	case osPriorityHigh1:
		sprintf(name, "osPriorityHigh1");
		break;
	case osPriorityHigh2:
		sprintf(name, "osPriorityHigh2");
		break;
	case osPriorityHigh3:
		sprintf(name, "osPriorityHigh3");
		break;
	case osPriorityHigh4:
		sprintf(name, "osPriorityHigh4");
		break;
	case osPriorityHigh5:
		sprintf(name, "osPriorityHigh5");
		break;
	case osPriorityHigh6:
		sprintf(name, "osPriorityHigh6");
		break;
	case osPriorityHigh7:
		sprintf(name, "osPriorityHigh7");
		break;
	case osPriorityRealtime:
		sprintf(name, "osPriorityRealtime");
		break;
	case osPriorityRealtime1:
		sprintf(name, "osPriorityRealtime1");
		break;
	case osPriorityRealtime2:
		sprintf(name, "osPriorityRealtime2");
		break;
	case osPriorityRealtime3:
		sprintf(name, "osPriorityRealtime3");
		break;
	case osPriorityRealtime4:
		sprintf(name, "osPriorityRealtime4");
		break;
	case osPriorityRealtime5:
		sprintf(name, "osPriorityRealtime5");
		break;
	case osPriorityRealtime6:
		sprintf(name, "osPriorityRealtime6");
		break;
	case osPriorityRealtime7:
		sprintf(name, "osPriorityRealtime7");
		break;
	case osPriorityISR:
		sprintf(name, "osPriorityISR");
		break;
	case osPriorityError:
		sprintf(name, "osPriorityError");
		break;
	case osPriorityReserved:
		sprintf(name, "osPriorityReserved");
		break;
	default:
		sprintf(name, "=== errror ===");
		break;
	}
}

#endif /* BOOTLOADER */

uint8_t calc_crc(const char* buffer){

	uint8_t crc8 = 0;

	for(uint16_t i = 0; i < strlen(buffer); i++){
		crc8 ^= buffer[i];
	}

	return crc8;
}

uint8_t trim_crlf(char* data){

  uint8_t resp = 0;

	// trim all \r or \n characters (or all combinations) from the end of the string
	// if e.g. size == 10, indexes are 0..9 so last item has index == size-1
	for(int i = strlen(data)-1; i>=0; i--){
		if((data[i]=='\r')||(data[i]=='\n')){
			data[i]=0x00;
			resp = i;
		} else {
			return 0;
		}
	}
  
	return resp;
}
