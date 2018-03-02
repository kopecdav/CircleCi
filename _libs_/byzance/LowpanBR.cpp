#include "LowpanBR.h"

static mem_stat_t heap_info;

#include "eventOS_event_timer.h"
#include "nat64_dri.h"
#include "tun_dri.h"
#include "NanostackRfPhyAtmel.h"
#include "nsdynmemLIB.h"
#include "ns_event_loop.h"

REGISTER_DEBUG(LowpanBR);

border_router_setup_s br_setup = {
	    0xaabb,     /**< Link layer PAN-ID, accepts only < 0xfffe.  */
	    0xface,     /**< Defines 802.15.4 short address. If the value is <0xfffe it indicates that GP16 is activated. */
	    4,     		/**< ZigBeeIP uses always 2. */
	    "aaaabbbbccccddd",         /**< Network ID 16-bytes, will be used at beacon payload. */
	    {0xfd,0x00,0x0d,0xB8,0x0f,0xf1,0x00,0x00},    /**< Define ND default prefix, ABRO, DODAG ID, GP address. */
	    180,        /**< Define ND router lifetime in seconds, recommend value 180+. */
	    0      		/**< ND ABRO version number (0 when starting a new ND setup). */
};

dodag_config_t dodag_config = {
	    1,      /**< Path Control Size limits number of DAO parents. Possible values are 0-7, meaning 1-8 parents.(This also has an Authentication flag 0x10 – the “SEC”, but we don’t support that, and 4 reserved flags, so it is just the 3-bit PCS field). */
	    12,  	/**< RPL Trickle DIOIntervalDoublings, recommended value 12. */
	    9,    	/**< RPL Trickle DIOIntervalMin, recommended value 9. */
	    3,      /**< RPL Trickle DIORedundancyConstant, recommended value 3. */
	    2048,  	/**< RPL MaxRankIncrease, recommended value 2048. */
	    128,  	/**< RPL MinHopRankIncrease, recommended value 128. */
	    1,      /**< Objective code point, recommended value 1. */
	    64,    	/**< Default lifetime for RPL routes, expressed in lifetime units. */
	    60    	/**< Defines the unit used to express route lifetimes, in seconds. */
};

const uint8_t pana_key[16]={0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8, 0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0};
const net_link_layer_psk_security_info_s psk_key_info ={
		0x01,
		{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf}
};

const uint8_t gp16_address_suffix[6] = {0x00, 0x00, 0x00, 0xff, 0xfe, 0x00};

const channel_list_s chan_list = {
		CHANNEL_PAGE_2,
		1<<1
};

const uint8_t nat64_prefix[] = NAT_PREFIX;

/*	heap musí být dostatečně velký
 *  v heapu se uchovává:
 *  	- vlastní TCP buffer border routeru
 *  	- odesílací fronta do lowpan sítě (může způsobovat ztrátu paketů, pokud jsou posílány v úzkém časovém sledu)
 *  	- routovací tabulka (závislost na velikosti sítě)
 *  	- heap statistiku je možné vypisovat pomocí metody ns_dyn_mem_get_mem_stat()
 */

#define APP_DEFINED_HEAP_SIZE 18432

uint8_t dodag_id[16];
int8_t LowpanBR::_lowpanif;
BR_state LowpanBR::_state=BR_NOT_INITIASED;
int8_t LowpanBR::_br_tasklet_id=-1;
uint8_t* LowpanBR::_heap;
/*Callback<void(const message_struct *)> LowpanBR::_rx_callback;
Callback<void(const message_struct *)> 		LowpanBR::_byzance_callback_rx_finished;
Callback<void(const message_struct *)> 		LowpanBR::_byzance_callback_tx_finished;
Callback<void(uint8_t)> 					LowpanBR::_byzance_callback_overflow;
Callback<void(TxFailedCodes, message_struct *)> 	LowpanBR::_byzance_callback_tx_failed;*/
rtos::Thread *LowpanBR::_thread;
uint8_t *LowpanBR::_dyn_mem;
uint8_t LowpanBR::_send_buffer[SEND_BUF_SIZE];
int8_t LowpanBR::_data_socket;
int8_t LowpanBR::_backhaulif;
bool LowpanBR::_nwk_busy=false;
uint16_t LowpanBR::_len;
ns_address LowpanBR::_rem_addr;
uint8_t LowpanBR::_tries=0;
NetworkInterface **LowpanBR::_eth;
nat_nwk_statistic_t LowpanBR::_nwk_stat;
RFWrapper *LowpanBR::_rf_phy;

void LowpanBR::init(NetworkInterface **itf,RFWrapper *rf){
	_rf_phy=rf;
	_eth=itf;
	__TRACE("enter LowpanBR::init\n");
	int8_t ret;
	SPI m_spi(RF_SPI4_MOSI,RF_SPI4_MISO,RF_SPI4_SCK);
	DigitalOut nCS(RF_SPI4_NSS);
	DigitalOut res(RF_RST);
	res = 0;
	res = 1;
	nCS = 1;
	uint8_t pn;
	m_spi.format(8,0);
	nCS = 0;
	m_spi.write(0x80 | 0x1C);	//read PARTNUM register
	pn = m_spi.write(0);	//read with dummy byte
	nCS=1;
	if(pn==0xff){
		__ERROR("hardware not present\n",ret);
		return;
	}
	if(_thread == NULL){
		_thread = new rtos::Thread(osPriorityLow, 512,NULL,"LowpanBR");
		ret = _thread->start(&LowpanBR::_thread_handler);
		__INFO("thread started with status %d\n",ret);
	}
	else{
		__WARNING("LowpanBR already running\n");
	}
	__TRACE("exit LowpanBR::init\n");
}

void LowpanBR::_thread_handler(){
	__TRACE("enter LowpanBR::_thread_handler\n");
	int8_t ret=-1;

	ns_hal_init(NULL, APP_DEFINED_HEAP_SIZE, NULL, &heap_info);
	ret = net_init_core();
	if(ret != 0){
		__ERROR("net_init_core returned %d\n",ret);
	}


	_lowpanif = LowpanBR::_rf_interface_init();
	if(_lowpanif>=0){
		__INFO("rf interface successfully initiased with id %d\n",_lowpanif);
		_state = BR_WAITING_FOR_INIT;
		ret = eventOS_event_handler_create(LowpanBR::_init,ARM_LIB_TASKLET_INIT_EVENT);	//create init handler
		__INFO("event handler created with status %d, running it\n",ret);
		eventOS_scheduler_run();		//enter event loop
	}
	__TRACE("exit LowpanBR::_thread_handler\n");
	//todo free heap
	_thread->terminate();
}

void LowpanBR::_init(arm_event_s *event){
	__TRACE("enter event handler\n");
	//this called in case of event
	arm_library_event_type_e event_type;
	event_type = (arm_library_event_type_e)event->event_type;

	switch(event_type){
	case ARM_LIB_NWK_INTERFACE_EVENT:		//rf interface event
		_parse_nwk_event(event);			//parsed by this function
		break;

	case APPLICATION_EVENT:		//own events
		__TRACE("event handler app event\n");
		switch(event->event_data){
		case 0x100:{

		}
			break;
		default:
			//printf("info:default app event\n");
			break;
		}
		break;


	case ARM_LIB_TASKLET_INIT_EVENT:		//first event called
		__TRACE("event handler init event\n");
		_br_tasklet_id = event->receiver;	//get taskled id
		int8_t ret;
		ret = LowpanBR::_backhaul_init();
		if(ret==0){
			__INFO("backhaul interface successfully initiased and requested to bring up, waiting for boostrap ready event\n");
		}
		else{
			__ERROR("failed to initiase backhaul interface with status %d\n",ret);
		}
		ret=LowpanBR::_lowpan_init();			//init lowpan nwk (rf already initiased)
		if(ret==0){
			__INFO("lowpan interface successfully initiased and requested to bring up, waiting for boostrap ready event\n");
		}
		else{
			__ERROR("failed to initiase lowpan interface with status %d\n",ret);
		}
		ret = arm_net_route_add(nat64_prefix, sizeof(nat64_prefix)*8,NULL, 0xffffffff, 128, _backhaulif);
		if(ret<0){
			__ERROR("failed to add route between lowpan and backhaul interface with status %d\n",ret);
		}
		break;
	case ARM_LIB_SYSTEM_TIMER_EVENT:		//custom timers
		nat64_feed(1000);
		//nat64_print_tcp_table();
		//nat64_print_udp_table();
		eventOS_event_timer_request(9,ARM_LIB_SYSTEM_TIMER_EVENT,_br_tasklet_id,1000);
		__TRACE("event handler timer event\n");
		//printf("trace: timer event id %d\n",event->event_id);
		//arm_print_routing_table2((void (*)(const char*, ...))&//printf);
	/*	if (event->event_id >= 10 && event->event_id < 15) {	//timers for repeating sending
		/extended_msg *m;
			LowpanBuffer::get_by_timer(event->event_id,&m);		//get msg by timer id
			eventOS_event_timer_cancel(event->event_id, ARM_LIB_SYSTEM_TIMER_EVENT);	//cancel timer
			if(m->tries<3){		//how many times was msg already sent?
				socket_sendto(_data_socket,&m->addr,m->msg,m->len);		//try to send msg again
				m->tries++;
				//printf("warn: UDP datagram ack not recieved in time, trying again %d/3",m->tries);
				eventOS_event_timer_request(event->event_id, ARM_LIB_SYSTEM_TIMER_EVENT, _br_tasklet_id, 100);	//request timer again
			}
			else{
				//printf("error: datagram could not be sent to device\n");
				m->inuse=false;
				//callback tx failed
			}
		}

		if (event->event_id == 9) {		//timer for checking device stats
		/*	char *fullid;
			StateEnum_TypeDef state;
			uint8_t timeout;
			if(DevList::lock() != DEVLIST_OK){		//lock DevList
				//printf("warn: devlist not locked, try it again in 100ms\n");
				eventOS_event_timer_request(9, ARM_LIB_SYSTEM_TIMER_EVENT, _br_tasklet_id, 100);
				return;
			}
			//increment timeout for all enumerated and timeouted devices
			for(uint32_t i=0;i<DevList::get_list_length();i++){
				DevList::get_state_at_index(&state,i);
				if(state==DEV_STATE_ENUMERATED || state==DEV_STATE_TIMEOUT){
					DevList::get_full_id_at_index(&fullid,i);
					DevList::get_timeout_cnt(&timeout,fullid);
					//if timeout is too large, set as timeouted or dead
					if(timeout==3){
						//printf("info: device id:%s in state timeout\n",fullid);
						DevList::set_state_at_index(i,DEV_STATE_TIMEOUT);
					}
					if(timeout==6){
						//printf("info: device id:%s in state dead\n",fullid);
						DevList::set_state_at_index(i,DEV_STATE_DEAD);
					}
					DevList::increase_timeout_cnt(fullid,1);
				}
			}
			DevList::unlock();
			eventOS_event_timer_request(9, ARM_LIB_SYSTEM_TIMER_EVENT, _br_tasklet_id, 10000);	//request timer again
		}*/
		break;
	}
}

int8_t LowpanBR::_backhaul_init(){
	__TRACE("enter LowpanBR::_backhaul_init\n");
	////printf("init backhaul");
	//todo generate mac from full-id
	uint8_t mac[8]={0xaa, 0xde, 0xfc,0xaa, 0xde, 0xfc};
	mbed_mac_address((char *)mac);
	//fixme data type
	int8_t driver_id = arm_tun_phy_device_register(mac, &_backhaul_phy_status_cb);
	if(driver_id < 0){
		__ERROR("arm_tun_phy_device_register failed\n");
		return -1;
	}
	eth_mac_api_t * eth_mac_api = ethernet_mac_create(driver_id);
	if(eth_mac_api == NULL){
		__ERROR("ethernet_mac_create failed to create fake tun api\n");
		return -2;
	}
	_backhaulif = arm_nwk_interface_ethernet_init(eth_mac_api, "bh0");
	if(_backhaulif < 0){
		__ERROR("arm_nwk_interface_ethernet_init failed to init fake tun api\n");
		return -3;
	}
	int8_t ret = arm_nwk_interface_up(_backhaulif);
	if(ret < 0){
		__ERROR("arm_nwk_interface_up failed to bring backhaul interface up with code %d\n",ret);
		return -4;
	}
	return 0;
}

void LowpanBR::_backhaul_phy_status_cb(uint8_t status, int8_t driver_id)
{
	switch(status){
	case NAT64_SEND_ETH_OK:
		_nwk_stat.eth_ok++;
		__INFO("NAT64 sending frame to Ethernet OK\n");
		break;
	case NAT64_SEND_ETH_FAIL:
		_nwk_stat.eth_fail++;
		__ERROR("NAT64 sending frame to Ethernet failed\n");
		break;
	case NAT64_FULL:
		__WARNING("NAT64 table is full\n");
		break;
	case NAT64_SEND_LOWPAN_OK:
		_nwk_stat.lowpan_ok++;
		__INFO("NAT64 sending frame to lowpan OK\n");
		break;
	case NAT64_SEND_LOWPAN_FAIL:
		_nwk_stat.lowpan_fail++;
		__ERROR("NAT64 sending frame to lowpan failed\n");
		break;
	default:
		__WARNING("NAT64 status %d not handled\n",status);
	}
}

int8_t LowpanBR::_rf_interface_init(void)
{
	__TRACE("enter LowpanBR::_rf_interface_init\n");
    char phy_name[] = "mesh0";
    int8_t rfid = -1;
    uint8_t my_mac[8];
	mbed_mac_address((char *)my_mac);
	_rf_phy->set_mac_address(my_mac);
    int8_t rf_phy_device_register_id = _rf_phy->rf_register();
    if(rf_phy_device_register_id < 0){
    	//rf_device_unregister();
    	__ERROR("rf_device_register failed with status %d, probably hardware problem\n",rf_phy_device_register_id);
    	return -1;
    }
	mac_description_storage_size_t storage_sizes;
	storage_sizes.device_decription_table_size = 32;
	storage_sizes.key_description_table_size = 3;
	storage_sizes.key_lookup_size = 1;
	storage_sizes.key_usage_size = 3;
	mac_api_t *api;
	api = ns_sw_mac_create(rf_phy_device_register_id, &storage_sizes);
	if(api == NULL){
    	__ERROR("ns_sw_mac_create api creation failed\n");
		return -2;
	}
	rfid = arm_nwk_interface_lowpan_init(api, phy_name);
	if(rfid<0){
    	__ERROR("arm_nwk_interface_lowpan_init failed with return code %d probably no memory\n");
	}
	__TRACE("exit LowpanBR::_rf_interface_init\n");
    return rfid;
}

inline int8_t LowpanBR::_lowpan_init(){
	__TRACE("enter LowpanBR::_lowpan_init\n");
	int8_t ret;
	ret = arm_nwk_interface_configure_6lowpan_bootstrap_set(_lowpanif,NET_6LOWPAN_BORDER_ROUTER,NET_6LOWPAN_ND_WITH_MLE);
	if(ret<0){
    	__ERROR("arm_nwk_interface_configure_6lowpan_bootstrap_set failed with code %d\n",ret);
	}
	ret = arm_nwk_link_layer_security_mode(_lowpanif, NET_SEC_MODE_PSK_LINK_SECURITY, 5, &psk_key_info);
	if(ret<0){
    	__ERROR("arm_nwk_link_layer_security_mode failed with code %d\n",ret);
	}
	link_layer_address_s mac_par;
	arm_nwk_mac_address_read(_lowpanif, &mac_par);
	br_setup.mac_panid=mac_par.mac_long[6]<<8|mac_par.mac_long[7];
	br_setup.mac_short_adr=mac_par.mac_long[6]<<8|mac_par.mac_long[7];
	ret = arm_nwk_6lowpan_border_router_init(_lowpanif,&br_setup);
	if(ret<0){
    	__ERROR("arm_nwk_6lowpan_border_router_init failed with code %d\n",ret);
	}
	memcpy(dodag_id, br_setup.lowpan_nd_prefix, 8);
	memcpy(&dodag_id[8], gp16_address_suffix, 6);
	dodag_id[14] = mac_par.mac_long[6];//(mac_par.mac_short >> 8);
	dodag_id[15] = mac_par.mac_long[7];
	ret = arm_nwk_6lowpan_border_router_context_update(_lowpanif, ((1 << 4) | 0x03), 128, 0xffff, dodag_id);
	if(ret<0){
    	__ERROR("arm_nwk_6lowpan_border_router_init failed with code %d\n",ret);
	}
	ret = arm_nwk_6lowpan_rpl_dodag_init(_lowpanif,dodag_id,&dodag_config,1,RPL_GROUNDED | BR_DODAG_MOP_NON_STORING | RPL_DODAG_PREF(0));
	if(ret<0){
    	__ERROR("arm_nwk_6lowpan_rpl_dodag_init failed with code %d\n",ret);
	}
	ret = arm_nwk_6lowpan_rpl_dodag_prefix_update(_lowpanif, dodag_id,64, RPL_PREFIX_ROUTER_ADDRESS_FLAG, 0xffffffff);
	if(ret<0){
    	__ERROR("arm_nwk_6lowpan_rpl_dodag_prefix_update failed with code %d\n",ret);
	}
	ret = arm_nwk_6lowpan_rpl_dodag_route_update(_lowpanif, dodag_id,0, 0, 0xffffffff);
	if(ret<0){
    	__ERROR("arm_nwk_6lowpan_rpl_dodag_route_update failed with code %d\n",ret);
	}
	//ret = arm_tls_add_psk_key(pana_key,1);
	//ret = arm_pana_server_library_init(_lowpanif, NET_TLS_PSK_CIPHER, pana_key, 120);
	ret = arm_nwk_set_channel_list(_lowpanif, &chan_list);
	if(ret<0){
    	__ERROR("arm_nwk_set_channel_list failed with code %d\n",ret);
	}
	/*ret = arm_nwk_interface_configure_ipv6_bootstrap_set(_lowpanif, NET_IPV6_BOOTSTRAP_STATIC, br_setup.lowpan_nd_prefix);
	if(ret<0){
	    __ERROR("arm_nwk_interface_configure_ipv6_bootstrap_set failed with code %d\n",ret);
	}*/
	ret = arm_nwk_interface_up(_lowpanif);
	if(ret<0){
		__ERROR("arm_nwk_interface_up failed with code %d\n",ret);
	}
	return ret;
}

void LowpanBR::_parse_nwk_event(arm_event_s *event){		//in case of network event
	__TRACE("network event\n");
	arm_nwk_interface_status_type_e nwk_stat=(arm_nwk_interface_status_type_e)event->event_data;
	int8_t ret;
	switch(nwk_stat){
	case ARM_NWK_BOOTSTRAP_READY:
		ret = arm_nwk_6lowpan_rpl_dodag_start(_lowpanif);
		if(ret==0){
			uint8_t ipv6[16];
			arm_net_address_get(_lowpanif, ADDR_IPV6_GP, ipv6);
			__INFO("bootstrap ready - both interfaces are up\n");
			_data_socket = socket_open(SOCKET_TCP,UDP_DATA_PORT,LowpanBR::_data_io_cb_tcp);
			socket_listen(_data_socket,2);
			//socket_open(SOCKET_UDP,UDP_SERVICE_PORT,LowpanBR::_service_io_cb);
			eventOS_event_timer_request(9,ARM_LIB_SYSTEM_TIMER_EVENT,_br_tasklet_id,1000);
			if(_eth == NULL){
				printf("eth is null\n");
			}
			DNS64::init(_eth);
			_state = BR_READY;
		}
		else{
			__WARNING("dodag not initiased, probably both interfaces are not up yet\n");
		}
		break;
	default:
		__INFO("other network event %d\n",nwk_stat);
		_state = BR_BOOTSTRAP_FAILED;
	}
}



void LowpanBR::_service_io_cb(void *ev){	//callback on service UDP port
	__TRACE("enter LowpanBR::_service_io_cb\n");
	socket_callback_t *sc = (socket_callback_t*)ev;
	switch(sc->event_type){
	case SOCKET_TX_DONE:
		//printf("info: transmit service done\n");
		break;
	case SOCKET_DATA:{
		uint8_t buf[512];
		ns_address add;
		uint16_t ret = socket_read(sc->socket_id,&add,buf,sizeof(buf));
		printf("socket data len %d socket says%d\n",sc->d_len,ret);
		dns_header_t *p;
		p=(dns_header_t *)buf;
		uint16_t l = strlen((char *)buf+sizeof(dns_header_t)+2);
		printf("asking for %s\n",(char *)buf+sizeof(dns_header_t)+2);
		printf("trans id is %x flags %x query len %d\n",htons(p->tid),htons(p->flags),l);
		for(uint16_t i=0; i<ret;i++){
			printf("%02x",buf[i]);
			//if((i+1)%8==0)
				//printf("\n");
		}
		printf("\n");
	}

	/*	if(buf[0]==0x01){	//keep alive - check with DevList
			char *fullid =(char*)buf+1;
			if(DevList::lock() != DEVLIST_OK){		//lock DevList
				//printf("warn: devlist not locked in service io callback\n");
				break;
			}
			bool exists=false;
			DevList::exist(&exists,fullid);
			if(exists){	//if device exists
				// and if state is not enumerated, set it
				StateEnum_TypeDef state;
				DevList::get_state(&state,fullid);
				if(state != DEV_STATE_ENUMERATED){
					DevList::set_ip_addr(add.address,fullid);
					DevList::set_state(fullid,DEV_STATE_ENUMERATED);
					//printf("info: set state enumerated for device %s\n",fullid);
				}
				//set timeout to zero for device
				//printf("info: timeout set to zero for device %s\n",fullid);
				DevList::set_timeout_cnt(fullid,0);
				DevList::unlock();
				//response
				buf[0]=0x02; //keepalive ack
				socket_sendto(sc->socket_id,&add,buf,1);
			}
			else{
				DevList::unlock();
				//odpovime
				buf[0]=0x03; //not in devlist
				socket_sendto(sc->socket_id,&add,buf,1);
				//printf("warn: service packet received from device %s that is not in list\n",fullid);
			}

		}
		if(buf[0]==0x04){	//recieved ack of data msg
			//printf("info: ack recieved %d\n",buf[1]);
			extended_msg *msg;
			int8_t ret=LowpanBuffer::get_by_id(buf[1],&msg);	//second byte in ack msg is id of acked msg
			if(ret==0){	//if id of acked msg exists
				//printf("canceling timer %d by id %d\n",msg->timer,msg->msg_id);
				eventOS_event_timer_cancel(msg->timer, _br_tasklet_id);	//cancel timer
				msg->inuse=false;	//and set as unused
			}
			else{
				//printf("error: msg id not found in buffer\n");	//this should not happen
			}

		}*/

		break;
	default:
		//printf("info: other service io event %d\n",sc->event_type);
		break;
	}
}



void LowpanBR::_data_io_cb(void *ev){
	socket_callback_t *sc = (socket_callback_t*)ev;
	switch(sc->event_type){
	case SOCKET_TX_DONE:
		//printf("info: transmit data done\n");
		//TX done is not handled here
		//it is handled by receiving own ack packet in service io callback or timer overflow
		/*
		if(_byzance_callback_tx_finished){
			message_struct msg;
			_byzance_callback_tx_finished.call(&msg);
		}*/
		break;
	case SOCKET_DATA:	//in case we receive data on data port (MQTT msg from device)
		//printf("info: data packet received\n");
		/*if(_byzance_callback_rx_finished){
			//message_struct msg;
			uint8_t buf[SEND_BUF_SIZE];
			ns_address addr;
			socket_read(sc->socket_id,&addr,buf,SEND_BUF_SIZE);
		//	_data_to_msg(buf,&msg);
		//	//printf("info: calling RX callback\n");
	//		_byzance_callback_rx_finished.call(&msg);
		}*/
		break;
	default:
		//printf("info: other data io event %d\n",sc->event_type);
		break;
	}
}



void _recieve_cb(void *ev){
	socket_callback_t *sc = (socket_callback_t*)ev;
	uint8_t data[30];
	switch(sc->event_type){
	case SOCKET_DATA:
		socket_read(sc->socket_id,NULL,data,sizeof(data));
		//printf("data rec %s\n",data);
	}
}

void LowpanBR::_data_io_cb_tcp(void *ev){
	socket_callback_t *sc = (socket_callback_t*)ev;
	switch(sc->event_type){
	case SOCKET_INCOMING_CONNECTION:{
		ns_address_t add;
		int8_t rec_sc = socket_accept(sc->socket_id,&add,_recieve_cb);
		//printf("info: transmit data done\n");
		//TX done is not handled here
		//it is handled by receiving own ack packet in service io callback or timer overflow
		/*
		if(_byzance_callback_tx_finished){
			message_struct msg;
			_byzance_callback_tx_finished.call(&msg);
		}*/
	}
		break;
	case SOCKET_DATA:	//in case we receive data on data port (MQTT msg from device)
		//printf("info: data packet received\n");
		/*if(_byzance_callback_rx_finished){
			//message_struct msg;
			uint8_t buf[SEND_BUF_SIZE];
			ns_address addr;
			socket_read(sc->socket_id,&addr,buf,SEND_BUF_SIZE);
		//	_data_to_msg(buf,&msg);
		//	//printf("info: calling RX callback\n");
	//		_byzance_callback_rx_finished.call(&msg);
		}*/
		break;
	default:
		//printf("info: other data io event %d\n",sc->event_type);
		break;
	}
}

/*
void LowpanBR::_msg_to_data(message_struct *msg, uint8_t *data, uint16_t *len){
	convert_mqtt_to_protocol(data,len,msg->topic.data(),msg->topic.length(),msg->payload.data(),msg->payload.length());
}

void LowpanBR::_data_to_msg(uint8_t *data,message_struct *msg){
	static char  			topic_name[MQTT_MAX_TOPIC_SIZE];
	static uint16_t			topic_name_len;
	static char  			payload[MQTT_MAX_PAYLOAD_SIZE];
	static uint16_t			payload_len;
	convert_protocol_to_mqtt(topic_name, &topic_name_len, payload, &payload_len, data, NULL);
	msg->topic.assign(topic_name, topic_name_len);
	msg->payload.assign(payload, payload_len);
}

void LowpanBR::send_msg(message_struct *msg){
	if(_nwk_busy){
		//printf("warn: network busy\n");
		return;
	}
	//printf("here should be message sent\n");
	if(DevList::lock() != DEVLIST_OK){
		//callback tx failed
		//printf("error: devlist se nepodarilo zamknout\n");
		return;
	}
	string device_full_id = msg->topic.substr(0, FULL_ID_LENGTH);

	//ziskam IP; pokud neni zarizeni v seznamu, zprava se neodesle

	if(DevList::get_ip_addr(_rem_addr.address, device_full_id.c_str()) != DEVLIST_OK){
		DevList::unlock();
		//printf("error: device is not in list\n");
		if(_byzance_callback_tx_failed){
			_byzance_callback_tx_failed.call(TX_ERR_MSG_DEV_NOT_IN_LIST, msg);
		}
		return;
	}
	//pokud zarizeni neni enumerovano, zprava se neodešle
	StateEnum_TypeDef state;
	DevList::get_state(&state,device_full_id.data());
	if(state != DEV_STATE_ENUMERATED){
		DevList::unlock();
		//printf("error: device is in list but not enumerated yet\n");
		if(_byzance_callback_tx_failed){
			_byzance_callback_tx_failed.call(TX_ERR_MSG_NOT_ENUMERATED, msg);
		}
		return;
	}
	DevList::unlock();
	//_nwk_busy=true;_tries=0;
	//everything OK with DevList, lets try to send it
	_rem_addr.identifier = UDP_DATA_PORT;
	_rem_addr.type = ADDRESS_IPV6;
	_msg_to_data(msg,_send_buffer+2,&_len);

	//print ip address
	//printf("info: sending data msg to ");
	for(uint8_t i=0;i<16;i++)
		//printf("%02x",_rem_addr.address[i]);
	//printf("\n");

	//printf("before %d\n",_send_buffer[1]);
	int8_t ret = LowpanBuffer::add((uint8_t *)&_send_buffer,&_rem_addr,_len);		//todo: nevyplnovat msg id, ale vyplinit ve funkci add pres pointer na send buffer
	//printf("after %d\n",_send_buffer[1]);
	if(ret==-1){
		//printf("error: LowpanBuffer is full\n");
	}
	else{
		eventOS_event_timer_request(ret, ARM_LIB_SYSTEM_TIMER_EVENT, _br_tasklet_id, 100);
		ret = socket_sendto(_data_socket,&_rem_addr,_send_buffer,_len);
		if(ret<0){
			//nastala nejaka chyba pri odesilani
			//-4 obvykle znamena, ze adresa jeste není zcela registrovana
			_nwk_busy=false;
			//printf("error: sending returned %d\n",ret);
		}
	}
}


void LowpanBR::attach_byzance_callback_rx_finished(void (*function)(const message_struct * msg)){
	_byzance_callback_rx_finished = callback(function);
}
void LowpanBR::attach_byzance_callback_tx_finished(void (*function)(const message_struct * msg)){
	_byzance_callback_tx_finished = callback(function);
}
void LowpanBR::attach_byzance_callback_overflow(void (*function)(uint8_t)){
	_byzance_callback_overflow = callback(function);
}
void LowpanBR::attach_byzance_callback_tx_failed(void (*function)(TxFailedCodes, message_struct *)){
	_byzance_callback_tx_failed = callback(function);
}
*/

const nat_nwk_statistic_t* LowpanBR::get_nat_stat(){
	return &_nwk_stat;
}

uint32_t LowpanBR::get_stack_size(void) {
    return _thread->stack_size();
}

uint32_t LowpanBR::get_free_stack(void) {
    return _thread->free_stack();
}

uint32_t LowpanBR::get_used_stack(void) {
    return _thread->used_stack();
}

uint32_t LowpanBR::get_max_stack(void) {
    return _thread->max_stack();
}

rtos::Thread::State LowpanBR::get_thread_state(void){
    return _thread->get_state();
}

osPriority LowpanBR::get_priority(void){
    return _thread->get_priority();
}

const mem_stat_t* LowpanBR::get_heap_info(void) {
	const mem_stat_t *heap_info = ns_dyn_mem_get_mem_stat();
	return heap_info;
}

void LowpanBR::print_heap_info(void) {
	const mem_stat_t *heap_info = ns_dyn_mem_get_mem_stat();
	if (heap_info) {
		__LOG("heap size: %d reserved: %d reserved max: %d alloc fail: %d\n",heap_info->heap_sector_size,heap_info->heap_sector_allocated_bytes,heap_info->heap_sector_allocated_bytes_max,heap_info->heap_alloc_fail_cnt);
	}
	else{
		__LOG("heap info not available\n");
	}
}

