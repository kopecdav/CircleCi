/*
 * NAT64Dri.cpp
 *
 *  Created on: 8. 6. 2017
 *      Author: martin
 */

#include "nat64_dri.h"
#include "tun_dri.h"

#include "pbuf.h"
#include "inet_chksum.h"
#include "ip.h"
#include "lwip/prot/ip6.h"
#include "lwip/prot/ip4.h"
#include "lwip/prot/tcp.h"
#include "lwip/prot/udp.h"

static uint8_t send_buffer[1500];
const uint8_t nat64_prefix[] = NAT_PREFIX;

NAT64row nat_list[NAT64_LIST_SIZE];
NAT64row nat_udp_list[NAT64_LIST_SIZE];
NAT64row nat_tcp_list[NAT64_LIST_SIZE];
uint16_t nat64_index=0;

//NAT64row nat_tcp_list[NAT64_LIST_SIZE];

void nat64_init_tcp_table(){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        nat_tcp_list[i].pseudoport=NAT64_TCP_PORT_START+i;
    }
}

void nat64_tcp_del(uint16_t index){
    if(index >= NAT64_LIST_SIZE)
        return;
    nat_tcp_list[index].in_use = 0;
}

void nat64_feed(uint32_t feed){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        if(nat_tcp_list[i].in_use){
            nat_tcp_list[i].timeout+=feed;
            if(nat_tcp_list[i].timeout>NAT64_TIMEOUT){
                NAT64_DEBUG("nat64_feed: tcp timeout exceeded for row %d\n",i);
                nat64_tcp_del(i);
            }
        }
        if(nat_udp_list[i].in_use){
			nat_udp_list[i].timeout+=feed;
			if(nat_udp_list[i].timeout>NAT64_TIMEOUT){
				NAT64_DEBUG("nat64_feed: udp timeout exceeded for row %d\n",i);
				nat64_udp_del(i);
			}
		}
    }
}

uint16_t nat64_tcp_add(uint32_t ipv4,uint16_t port4,uint16_t port6, uint8_t *ipv6){
    uint16_t free_index=0;
    while(nat_tcp_list[free_index].in_use == 1 && free_index < NAT64_LIST_SIZE){
        free_index++;
    }
    if(free_index >= NAT64_LIST_SIZE){
    	NAT64_DEBUG("nat64_tcp_add: tcp nat table is full\n");
        return 0;
    }
    nat_tcp_list[free_index].ipv4=ipv4;
    nat_tcp_list[free_index].port4=port4;
    nat_tcp_list[free_index].port6=port6;
    memcpy(nat_tcp_list[free_index].ipv6,ipv6,16);
    nat_tcp_list[free_index].in_use = 1;
    nat_tcp_list[free_index].timeout = 0;
    return nat_tcp_list[free_index].pseudoport;
}

uint16_t nat64_tcp_find_or_add(uint32_t ipv4,uint16_t port4,uint16_t port6, uint8_t *ipv6){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        if(nat_tcp_list[i].in_use)
            if(nat_tcp_list[i].port6 == port6)
                if(nat_tcp_list[i].port4 == port4)
                    if(nat_tcp_list[i].ipv4 == ipv4)
                        if(memcmp(ipv6,nat_tcp_list[i].ipv6,16)==0){
                        	ipv6,nat_tcp_list[i].timeout=0;
                            return nat_tcp_list[i].pseudoport;
                        }


    }
    return nat64_tcp_add(ipv4,port4,port6,ipv6);
}

NAT64row* nat64_tcp_find(uint32_t ipv4,uint16_t port4,uint16_t pseudoport){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        if(nat_tcp_list[i].in_use)
            if(nat_tcp_list[i].pseudoport == pseudoport)
                if(nat_tcp_list[i].port4 == port4)
                    if(nat_tcp_list[i].ipv4 == ipv4){
                    	nat_tcp_list[i].timeout=0;
                    	return &nat_tcp_list[i];
                    }


    }
    return NULL;
}


void nat64_print_tcp_table(){
	printf("tcp nat table\n");
    uint16_t j=0;
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++ ){
        if(nat_tcp_list[i].in_use){
            printf("row %d pseudoport %d port4 %d port6 %d timeout %d\n",j,nat_tcp_list[i].pseudoport,nat_tcp_list[i].port4,nat_tcp_list[i].port6,nat_tcp_list[i].timeout);
            j++;
        }
    }
    return;
}


NAT64row *nat64_udp_find(uint32_t ipv4,uint16_t port4,uint16_t pseudoport){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        if(nat_udp_list[i].in_use)
            if(nat_udp_list[i].pseudoport == pseudoport)
                if(nat_udp_list[i].port4 == port4)
                    if(nat_udp_list[i].ipv4 == ipv4)
                    	return &nat_udp_list[i];
    }

    return NULL;
}

void nat64_init_udp_table(){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        nat_udp_list[i].pseudoport=NAT64_UDP_PORT_START+i;
    }
}


void nat64_udp_del(uint16_t index){
    if(index >= NAT64_LIST_SIZE)
        return;
    nat_udp_list[index].in_use = 0;
}

uint16_t nat64_udp_add(uint32_t ipv4,uint16_t port4,uint16_t port6, uint8_t *ipv6){
    uint16_t free_index=0;
    while(nat_udp_list[free_index].in_use == 1 && free_index < NAT64_LIST_SIZE){
        free_index++;
    }
    if(free_index >= NAT64_LIST_SIZE){
        NAT64_DEBUG("nat64_udp_add: udp nat table is full\n");
        return 0;
    }
    nat_udp_list[free_index].ipv4=ipv4;
    nat_udp_list[free_index].port4=port4;
    nat_udp_list[free_index].port6=port6;
    memcpy(nat_udp_list[free_index].ipv6,ipv6,16);
    nat_udp_list[free_index].in_use = 1;
    nat_udp_list[free_index].timeout = 0;
    return nat_udp_list[free_index].pseudoport;
}

uint16_t nat64_udp_find_or_add(uint32_t ipv4,uint16_t port4,uint16_t port6, uint8_t *ipv6){
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++){
        if(nat_udp_list[i].in_use)
            if(nat_udp_list[i].port6 == port6)
                if(nat_udp_list[i].port4 == port4)
                    if(nat_udp_list[i].ipv4 == ipv4)
                        if(memcmp(ipv6,nat_udp_list[i].ipv6,16)==0)
                            return nat_udp_list[i].pseudoport;


    }

    return nat64_udp_add(ipv4,port4,port6,ipv6);
}

void nat64_print_udp_table(){
    uint16_t j=0;
    for(uint16_t i=0; i<NAT64_LIST_SIZE; i++ ){
        if(nat_udp_list[i].in_use){
            printf("row %d pseudoport %d port4 %d port6 %d\n",j,nat_udp_list[i].pseudoport,nat_udp_list[i].port4,nat_udp_list[i].port6);
            j++;
        }
    }
    return;
}

/*
 * uint8_t nat64_input(struct pbuf *p)
 *
 * p is pbuf - first byte of p->payload is first byte of IPv4 header
 *
 * returns 0 if packet is not to NAT, else returns 1
 *
 */

uint8_t nat64_input(struct pbuf *p){
	//check minimal length with IPv4+UDP header and p->len
	struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
	uint8_t *po = (uint8_t*)p->payload;
	uint16_t dest_port=(po[sizeof(struct ip_hdr)]<<8)|po[sizeof(struct ip_hdr)+1];
	uint16_t source_port=(po[sizeof(struct ip_hdr)+2]<<8)|po[sizeof(struct ip_hdr)+3];

	NAT64row *row;
	switch(iphdr->_proto){
	case IP_PROTO_UDP:
		row = nat64_udp_find(iphdr->src.addr,dest_port,source_port);
		break;
	case IP_PROTO_TCP:
		row = nat64_tcp_find(iphdr->src.addr,dest_port,source_port);
		break;
	default:
		//protocol is not UDP or TCP - not to NAT
		return 0;
	}
	//check if we found row
	if(row==NULL || iphdr->_ttl == 0){
		//printf("row not matched %p\n",row);
		return 0;
	}
	send_buffer[2]=0x86; send_buffer[3]=0xdd;			//ipv6 identificator for tunnel driver
	//now we have row, lets create IPv6 header
	struct ip6_hdr *hdr6 = (struct ip6_hdr *)(send_buffer+TUN_DRIVER_OFFSET);
	hdr6->_hoplim = iphdr->_ttl - 1;	//decrement ttl and store it as hoplimit
	hdr6->_nexth = iphdr->_proto;		//next header is UDP or TCP
	IP6H_PLEN_SET(hdr6,lwip_ntohs(IPH_LEN(iphdr))-20);	//IPv6 payload len is total len in IPv4 without 20 B of IPV4 header
	IP6H_VTCFL_SET(hdr6,6,0,0);		//IPv6, no data labels etc.
	//fill dest IP - 16B
	memcpy(hdr6->dest.addr,row->ipv6,16);
	//fil src IP - NAT64 prefix 12B + IPv4 4B
	memcpy(hdr6->src.addr,nat64_prefix,12);	//NAT prefix
	memcpy((uint8_t*)(hdr6->src.addr)+sizeof(nat64_prefix),&iphdr->src.addr,4);	//IPv4 addr
	//copy remaining data
	memcpy(send_buffer+TUN_DRIVER_OFFSET+sizeof(struct ip6_hdr),po+sizeof(struct ip_hdr),IP6H_PLEN(hdr6));


	//calculate checksums

	//ipheader checksums
	uint32_t hsum=0;
	//addresses
	hsum += lwip_ntohs(LWIP_CHKSUM( (uint8_t *)&hdr6->src.addr, 32 ));
	//length
	hsum += IP6H_PLEN(hdr6);			//payload len
	//next header
	hsum += hdr6->_nexth;				//next protocol

	//calculate data checksum
	uint32_t dsum=0;
	if(iphdr->_proto == IP_PROTO_UDP){
		struct udp_hdr *udph = (struct udp_hdr *) (send_buffer+TUN_DRIVER_OFFSET+sizeof(struct ip6_hdr));
		udph->chksum = 0x00;	//set original checksum to zero
		udph->dest = lwip_htons(row->port6);	//fill port before calculating checksum
		//calculate checksum over all data
		dsum = lwip_ntohs(LWIP_CHKSUM(send_buffer+TUN_DRIVER_OFFSET+sizeof(struct ip6_hdr),IP6H_PLEN(hdr6)));
		//add IP header checksum and its data
		dsum += hsum;
		//accumulate higher bytes
		dsum = LWIP_CHKSUM(&dsum,4);
		//and fill it
		udph->chksum = (uint16_t)~dsum;
	}else if(iphdr->_proto == IP_PROTO_TCP){
		struct tcp_hdr *tcph = (struct tcp_hdr *)(send_buffer+TUN_DRIVER_OFFSET+sizeof(struct ip6_hdr));
		tcph->chksum = 0x00;	//set original checksum to zero
		tcph->dest = lwip_htons(row->port6);	//fill port before calculating checksum
		//calculate checksum over all data
		dsum = lwip_ntohs(LWIP_CHKSUM(send_buffer+TUN_DRIVER_OFFSET+sizeof(struct ip6_hdr),IP6H_PLEN(hdr6)));
		//add IP header checksum and its data
		dsum += hsum;
		//accumulate higher bytes
		dsum = LWIP_CHKSUM(&dsum,4);
		//and fill it
		tcph->chksum = lwip_ntohs((uint16_t)~dsum);
	}

	//send it to lowpan
	tun_phy_rx(send_buffer,IP6H_PLEN(hdr6)+sizeof(struct ip6_hdr)+TUN_DRIVER_OFFSET);
	return 1;

}

/*
 *
 */

int nat_convert_6to4(uint8_t *data_ptr,uint16_t data_len,struct ip_hdr *ip4hdr){
	if(data_len < 40+4+TUN_DRIVER_OFFSET){	//40 B of IPv6 header + at least 4B of UDP header + 4B padding
		return -1;	//packet too small
	}
	//check NAT64 prefix
	struct ip6_hdr *hdr6 = (struct ip6_hdr *)(data_ptr+TUN_DRIVER_OFFSET);
	if( memcmp(nat64_prefix,hdr6->dest.addr,sizeof(nat64_prefix)) ){
		return -2;		//packet doesnt fit to NAT64 prefix
	}
	//lets take ports from original packet
	uint16_t ports = (data_ptr[44]<<8)|data_ptr[45];
	uint16_t portd = (data_ptr[46]<<8)|data_ptr[47];
	uint16_t pseudoport=0;
	memcpy(&ip4hdr->dest.addr,(uint8_t *)(hdr6->dest.addr)+12,4);		//copy addr without NAT prefix

	//check protocol and get pseudoport
	switch(hdr6->_nexth){
	case IP_PROTO_UDP:
		pseudoport = nat64_udp_find_or_add(ip4hdr->dest.addr,portd,ports,(uint8_t *)hdr6->src.addr);
		break;
	case IP_PROTO_TCP:
		pseudoport = nat64_tcp_find_or_add(ip4hdr->dest.addr,portd,ports,(uint8_t *)hdr6->src.addr);
		break;
	default:
		NAT64_DEBUG("nat_convert_6to4: unsupported transport protocol\n");
		return -3;	//unsupported protocol
	}
	if(pseudoport == 0){
		NAT64_DEBUG("nat_convert_6to4: NAT table is full\n");
		return -4;	//NAT table is full
	}
	NAT64_DEBUG("nat_convert_6to4: using psudoport %d for packet len %d\n",pseudoport,data_len);
	//fill pseudoport
	data_ptr[44]=pseudoport>>8;
	data_ptr[45]=pseudoport&0x00ff;
	//fill TTL and ToS
	ip4hdr->_ttl = hdr6->_hoplim - 1;
	ip4hdr->_proto = hdr6->_nexth;
	return 40+4;	//offset of UDP or TCP header
}







