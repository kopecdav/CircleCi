/*
 * DNS64.cpp
 *
 *  Created on: 17. 7. 2017
 *      Author: martin
 */

#include "DNS64.h"
#include "nat64_dri.h"
NetworkInterface** DNS64::_eth;

REGISTER_DEBUG(DNS64);

/*
 * DNS64::init(NetworkInterface **itf, uint16_t port)
 *
 * function should open UDP socket for specified port and add callback to it
 *
 * itf - interface to resolve hostname to IPv4 from
 * port - port to open socket on (optional, default 53)
 *
 */
int8_t DNS64::init(NetworkInterface **itf, uint16_t port){
	__TRACE("init called\n");
	_eth=itf;
	int8_t ret;
	ret = socket_open(SOCKET_UDP,port,DNS64::_socket_cb);
	if(ret<0){
		__ERROR("failed to open socket %d\n",ret);
		return ret;
	}
	__INFO("successfully initiated\n");
	return 0;

}

void DNS64::_socket_cb(void *ev){
	__TRACE("socket event\n");
	socket_callback_t *sc = (socket_callback_t*)ev;
	switch(sc->event_type){
	case SOCKET_DATA:{
		__TRACE("received data len %d\n",sc->d_len);
		uint8_t buf[512];		//RFC says this is maximum size
		ns_address add;
		int16_t ret = socket_read(sc->socket_id,&add,buf,sizeof(buf));
		if(ret < 0){
			__ERROR("error reading from socket %d\n",ret);
			return;
		}
		__TRACE("read %d bytes from socket\n",ret);
		dns_header_t *p;
		p=(dns_header_t *)buf;
		if(p->flags == htons(0x0100)){
			__INFO("message is standart query\n");
		}else{
			__ERROR("invalid flags\n");
			return;
		}
		if(p->qs != htons(1)){
			__ERROR("number of queries is %d, maximum supported is 1\n",htons(p->qs));
			return;
		}
		uint16_t len=0;
		uint8_t iter=0;
		char name[255];
		while(buf[sizeof(dns_header_t)+len]){
			len+=buf[sizeof(dns_header_t)+len]+1;
			iter++;
			if(iter>16 || len>255){
				__ERROR("asked hostname is too long (>255) or contains too many subdomains (>16)\n");
				return;
			}
		}
		memcpy(name,buf+sizeof(dns_header_t)+1,len);
		len=0;
		while(buf[sizeof(dns_header_t)+len]){
			len+=buf[sizeof(dns_header_t)+len]+1;
			if(buf[sizeof(dns_header_t)+len] == 0)
				break;
			name[len-1]='.';
		};
		name[len] = 0;
		SocketAddress rem_addr;
		if(*_eth != NULL){
			ret = (*_eth)->gethostbyname(name,&rem_addr,NSAPI_IPv4);
			if(ret == 0){
				__INFO("solved ip for %s is %s\n",name,rem_addr.get_ip_address());
			}else{
				__ERROR("get by hostname returned %d\n",ret);
				return;
			}
		}else{
			__ERROR("backhaul interface is null - maybe not initiated yet\n");
			return;
		}
		//build response
		dns_AAAAresponse_t resp = {
				htons(0xc00c),	//name for first query offset
				htons(0x001c),	//AAAA record
				htons(0x0001),	//class
				htonl(0x15180),	//timeout
				htons(16),		//number of IPv6 address bytes
				NAT_PREFIX		//fill NAT prefix
		};
		p->flags=htons(0x8180);		//set flags response, etc.
		p->ans=htons(1);			//number of answers is one
		memcpy(&resp.ipv6[12],rem_addr.get_ip_bytes(),4);					//build IP in response - add last bytes to NAT_PREFIX
		memcpy((char *)buf+sizeof(dns_header_t)+len+4+1,&resp,sizeof(resp));
		//and send the buffer with response
		ret = socket_sendto(sc->socket_id,&add,buf,sizeof(dns_header_t)+len+5+sizeof(resp));
		if(ret != 0){
			__WARNING("sending to socket failed - remote address is maybe not registered yet\n");
			return;
		}

	}
		break;
	case SOCKET_TX_FAIL:
		__TRACE("transmit failed\n");
		break;
	case SOCKET_TX_DONE:
		__TRACE("transmit done\n");
		break;
	default:
		__TRACE("other event %d\n",sc->event_type);
	}
}
