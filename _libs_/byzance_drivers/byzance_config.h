#ifndef _BYZANCE_CONFIG_H
#define _BYZANCE_CONFIG_H

#include "mbed.h"

/*
 * MQTT paho knihovna QOS
 */
#define MQTTCLIENT_QOS1 0
#define MQTTCLIENT_QOS2 0

/*
 * nastavení MQTT packetu
 */
#define MQTT_MAX_TOPIC_SIZE			128
#define MQTT_MAX_PAYLOAD_SIZE		1536
#define MQTT_MAX_OVERHEAD_SIZE		10
#define	MQTT_MAX_SINGLE_BUFF		(MQTT_MAX_TOPIC_SIZE > MQTT_MAX_PAYLOAD_SIZE ? MQTT_MAX_TOPIC_SIZE: MQTT_MAX_PAYLOAD_SIZE)

#define MAX_MESSAGE_HANDLERS		10
#define MAX_MQTT_PACKET_SIZE		MQTT_MAX_TOPIC_SIZE + MQTT_MAX_PAYLOAD_SIZE + MQTT_MAX_OVERHEAD_SIZE

/*
 * pokud nastane nějaké katastrofické selhání, zařízení v něm zůstane (zamrzne a začne blikat),
 * nebo se pokusí o restart
 */

#define BYZANCE_RESET_AFTER_FAILURE			0


/*
 * kolikrát se klient pokusí odeslat zprávu nebo subscribovat do určitého topicu
 * pokud to do tohoto pokusu nevyjde, tak zruší aktuální socket a instanci klienta a naváže si novou
 * při zrušení připojení se i vymažou buffery se zprávama
 */
#define CLIENT_NUMBER_OF_SEND_ATTEMPTS		10

/*
 *	zap/vyp kompletniho logu webovych stranek co prijima nebo odesila webserver
 */
#define ENABLE_HTTP_THREAD_MESSAGES			0

#endif /* _BYZANCE_CONFIG_H */
