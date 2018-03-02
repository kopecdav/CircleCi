/*
 * DNS64.h
 *
 *  Created on: 17. 7. 2017
 *      Author: martin
 */

#ifndef BYZANCE_DNS64_H_
#define BYZANCE_DNS64_H_

#include "NetworkInterface.h"
#include "socket_api.h"
#include "net_interface.h"
#include "mbed.h"
#include "def.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

/*
 * DNS64
 *
 * this is simple DNS server for IPv6 on 6lowpan network
 *
 * - server functionality is on predefined UDP port no 53 (may be changed)
 * - it doesn't hold any static or dynamic records, server just uses NetworkInterface to resolve hostnames
 * flow is following:
 * - server receives IPv6 query (hostname)
 * - it asks NetworkInterface to get IPv4 by hostname (forwards query to real DNS servers)
 * - in case of response, it builds AAAA (IPv6) answer (IPv6 address = NAT_PREFIX + IPv4)
 *
 */

/**
 * dns_header
 * \brief defines fixed DNS header (transaction id, flags, number of queries, number of answers etc).
 */
typedef struct dns_header{
	uint16_t tid;
	uint16_t flags;
	uint16_t qs;
	uint16_t ans;
	uint16_t aurr;
	uint16_t adrr;
} dns_header_t;

/**
 * dns_AAAAresponse
 * \brief defines AAAA (IPv6) response, used pragma to get rid of 4 bytes paddings.
 * \param name - pointer to query
 * \param type (for AAAA 0x001c)
 * \param class (usually 0x0001)
 * \param ttl
 * \param data length (for IPv6 16)
 * \param data (IPv6 address)
 */
#pragma pack(push,1)
typedef struct dns_AAAAresponse{
	uint16_t name;
	uint16_t type;
	uint16_t cl;
	uint32_t ttl;
	uint16_t data_len;
	uint8_t ipv6[16];
}dns_AAAAresponse_t;
#pragma pack(pop)

/** 
* \class DNS64
* \brief brief TO DO 
*
* Description TO DO
*/
class DNS64 {
public:
	
	/** init
	* 
	* \param
	* \param
	*/
	static int8_t init(NetworkInterface **itf, uint16_t port=53);
	
protected:
	
	static NetworkInterface **_eth;
	
	/** _socket_cb
	*
	*
	* \param 
	*/
	static void _socket_cb(void *ev);
	DNS64();
};



#endif /* BYZANCE_DNS64_H_ */
