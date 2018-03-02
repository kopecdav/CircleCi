/*
 * tun_dri.c
 *
 *  Created on: 9. 2. 2018
 *      Author: martin
 */
#include "tun_dri.h"
#include "ip.h"
#include "lwip/prot/ip6.h"
#include "lwip/prot/ip4.h"
#include "lwip/prot/tcp.h"
#include "lwip/prot/udp.h"
#include "nat64_dri.h"
#include "arm_hal_phy.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip_addr.h"
#include "lwip/def.h"

static const uint8_t nat64_prefix[] = NAT_PREFIX;
static phy_device_driver_s tun_device_driver;
static int8_t tun_interface_id = -1;
static void (*driver_readiness_status_callback)(uint8_t, int8_t) = 0;





/*
 * tun_phy_rx(uint8_t *data, uint16_t len)
 * send raw data to lowpan
 *
 */

int8_t tun_phy_rx(uint8_t *data, uint16_t len){
	TUN_DEBUG("tun_phy_rx: called, trying to send %d to lowpan\n",len);
	int8_t ret = tun_device_driver.phy_rx_cb(data, len, 0xff, 0, tun_interface_id);
	if(ret != 0){				//and call callbacks to the app about send status
		TUN_DEBUG("tun_phy_rx: sending to lowpan failed %d\n",ret);
		driver_readiness_status_callback(NAT64_SEND_LOWPAN_FAIL, tun_interface_id);
	} else {
		TUN_DEBUG("tun_phy_rx: sending to lowpan OK\n",ret);
		driver_readiness_status_callback(NAT64_SEND_LOWPAN_OK, tun_interface_id);
	}
}

/*
 * tun_phy_tx(uint8_t *data_ptr, uint16_t data_len, uint8_t tx_handle,data_protocol_e data_flow)
 * automatically called by driver when new data routed to tun interface
 *
 */
int8_t tun_phy_tx(uint8_t *data_ptr, uint16_t data_len, uint8_t tx_handle,data_protocol_e data_flow)
{
	/*
	 * this callback is called by backhaul api
	 * to send data from lowpan to ethernet
	 * if(data_len >= ENET_HDR_LEN){
        retval = tun_send(data_ptr, data_len);
    }
	 */
    int retval = -1;
    struct pbuf *mybuf;
    ip4_addr_t dest_ip;
    struct ip_hdr ip4hdr;
    //check data and covert them
    retval = nat_convert_6to4(data_ptr,data_len,&ip4hdr);
    if(retval < 0){	//probably not to NAT
    	TUN_DEBUG("tun_phy_tx: recved packet not to NAT\n");
    	return retval;
    }
    TUN_DEBUG("tun_phy_tx: packet from lowpan converted 6>4\n");
    mybuf = pbuf_alloc(PBUF_TRANSPORT,data_len-retval,PBUF_RAM);
    memcpy(mybuf->payload,data_ptr+retval,data_len-retval);
    mybuf->tot_len=data_len-retval;
    mybuf->len=data_len-retval;
    //send it to IPV4 output (finds rigt netif automatically)
    retval = ip4_output(mybuf, IP4_ADDR_ANY,(ip4_addr_t*) &ip4hdr.dest.addr, ip4hdr._ttl, ip4hdr._tos, ip4hdr._proto);
    TUN_DEBUG("tun_phy_tx: sending to IPv4 returned %d\n",retval);
    if(retval != ERR_OK){
		driver_readiness_status_callback(NAT64_SEND_ETH_FAIL, tun_interface_id);
	}else{
		driver_readiness_status_callback(NAT64_SEND_ETH_OK, tun_interface_id);
	}
	pbuf_free(mybuf);

    (void)data_flow;
    (void)tx_handle;

   return retval;
}

static int8_t tun_interface_state_control(phy_interface_state_e state, uint8_t not_required)
{
	/* we dont use state control
	 * tunneling driver has not any hardware feature to be controled
	 */
    switch(state){
        case PHY_INTERFACE_DOWN:
            break;
        case PHY_INTERFACE_UP:
            break;
        case PHY_INTERFACE_RESET:
            break;
        case PHY_INTERFACE_SNIFFER_STATE:
            /*TODO Allow promiscuous state here*/
            break;
        case PHY_INTERFACE_RX_ENERGY_STATE:
            /*Just to get rid of compiler warning*/
            break;

    }
    (void)not_required;

    return 0;
}

static int8_t tun_address_write(phy_address_type_e address_type, uint8_t *address_ptr)
{
	/*
	 * this is to set MAC address
	 * it actually does nothing - tunneling driver doesnt need MAC adress
	 * real eth driver uses its own address
	 */
    int8_t retval = 0;

    switch(address_type){
        case PHY_MAC_48BIT:
        	//stm32_set_mac_48bit(address_ptr); /* betzw - WAS: stm32_eth_set_address(address_ptr);
        	                                    /* But once initialized, we cannot change MAC address in the driver!?!
        	 	 	 	 	 	 	 	 	   */
            break;
        case PHY_MAC_64BIT:
        case PHY_MAC_16BIT:
        case PHY_MAC_PANID:
            retval=-1;
            break;
    }
    return retval;
}

int8_t arm_tun_phy_device_register(uint8_t *mac_ptr, void (*driver_status_cb)(uint8_t, int8_t))
{
    if (tun_interface_id < 0) {

        tun_device_driver.PHY_MAC = mac_ptr;
        tun_device_driver.address_write = &tun_address_write;

        tun_device_driver.driver_description = (const char*) "TUN";
        tun_device_driver.link_type = PHY_LINK_TUN;
        tun_device_driver.phy_MTU = ((uint32_t)1500U);
        tun_device_driver.phy_header_length = 0;
        tun_device_driver.phy_tail_length = 0;
        tun_device_driver.state_control = &tun_interface_state_control;
        tun_device_driver.tx = &tun_phy_tx;
        tun_device_driver.phy_rx_cb = NULL;
        tun_device_driver.phy_tx_done_cb = NULL;
        //init tables
        nat64_init_udp_table();
        nat64_init_tcp_table();
        tun_interface_id = arm_net_phy_register(&tun_device_driver);
        driver_readiness_status_callback = driver_status_cb;

        if (tun_interface_id < 0){
        	TUN_DEBUG("arm_tun_phy_device_register: device registration failed %d\n", tun_interface_id);
            driver_readiness_status_callback(0, tun_interface_id);
            return tun_interface_id;
        }
        TUN_DEBUG("arm_tun_phy_device_register: device registration OK %d\n", tun_interface_id);
    }
    //driver_readiness_status_callback(true, tun_interface_id);

    return tun_interface_id;
}

