/*
 * LowpanBR.h
 *
 *  Created on: 16. 5. 2017
 *      Author: martin
 */

#ifndef SOURCE_LOWPANBR_H_
#define SOURCE_LOWPANBR_H_

#include "mbed.h"
#include "eventOS_event.h"
#include "mac_api.h"
#include "sw_mac.h"
#include "net_rpl.h"
#include "RFWrapper.h"
#include "socket_api.h"
#include "eventOS_event_timer.h"
#include "eventOS_scheduler.h"
#include <string>
#include "mbed.h"
#include "ns_hal_init.h"
#include "support_functions.h"
#include "ethernet_mac_api.h"
#include "eth_arch.h"
#include "nat64_dri.h"
#include "net_interface.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "Thread.h"
#include "nsdynmemLIB.h"
#include "DNS64.h"

#define SEND_BUF_SIZE (MQTT_MAX_PAYLOAD_SIZE + MQTT_MAX_TOPIC_SIZE)
#define UDP_DATA_PORT 61112
#define UDP_SERVICE_PORT 53
#define TCP_DATA_PORT 61113
#define NR_BACKHAUL_INTERFACE_PHY_DRIVER_READY 2
#define NR_BACKHAUL_INTERFACE_PHY_DOWN  3

/** end of default configuration of Border Router */

/* State of BR*/

typedef enum BR_state{
	BR_NOT_INITIASED,
	BR_READY,
	BR_WAITING_FOR_INIT,
	BR_BOOTSTRAP_FAILED
};

/** 
* \struct nat_nwk_statistic
* \brief brief TO DO 
*
* Long description TO DO 
*/
typedef struct nat_nwk_statistic{
	uint64_t eth_ok;
	uint64_t eth_fail;
	uint64_t lowpan_ok;
	uint64_t lowpan_fail;
} nat_nwk_statistic_t;

/**
* \class LowpanBR
* \brief brief TO DO
*
* Long description TO DO 
*/
class LowpanBR{
public:

	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void init(NetworkInterface **itf,RFWrapper *rf);
	/*
	static void send_msg(message_struct *msg);
	static void set_rx_callback(void (*function)(const message_struct * msg));
	static void attach_byzance_callback_rx_finished(void (*function)(const message_struct * msg));
	static void attach_byzance_callback_tx_finished(void (*function)(const message_struct * msg));
	static void attach_byzance_callback_overflow(void (*function)(uint8_t));
	static void attach_byzance_callback_tx_failed(void (*function)(TxFailedCodes, message_struct *));
*/

	/** 
	*
	* \param
	* \param
	* \return
	*/
	static uint32_t get_stack_size();
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static uint32_t get_free_stack(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static uint32_t get_used_stack(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static uint32_t get_max_stack(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static rtos::Thread::State get_thread_state(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static osPriority get_priority(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static const mem_stat_t* get_heap_info(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void print_heap_info(void);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static const nat_nwk_statistic_t* get_nat_stat();

protected:
	static RFWrapper *_rf_phy;
	static nat_nwk_statistic_t _nwk_stat;
	static uint8_t *_dyn_mem;
	static int8_t _data_socket;
	static bool _nwk_busy;
	static rtos::Thread *_thread;
	static uint8_t* _heap;
	/*static Callback<void(const message_struct *)> 		_byzance_callback_rx_finished;
	static Callback<void(const message_struct *)> 		_byzance_callback_tx_finished;
	static Callback<void(uint8_t)> 					_byzance_callback_overflow;
	static Callback<void(TxFailedCodes, message_struct *)> 	_byzance_callback_tx_failed;*/
	//static void _data_to_msg(uint8_t * data, message_struct * msg);
	//static void _msg_to_data(message_struct * msg,uint8_t *data, uint16_t *len );
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _data_io_cb(void *ev);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _data_io_cb_tcp(void *ev);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _tcp_data_io_cb(void *ev);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _service_io_cb(void *ev);
	
	
	static BR_state _state;
	static int8_t _lowpanif;
	static int8_t _backhaulif;
	static int8_t _br_tasklet_id;
	static uint8_t _send_buffer[SEND_BUF_SIZE];
	static ns_address _rem_addr;
	static uint16_t _len;
	static uint8_t _tries;
	static NetworkInterface **_eth;
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _init(arm_event_s *event);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _parse_nwk_event(arm_event_s *event);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static int8_t _rf_interface_init();
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static int8_t _backhaul_init();
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _backhaul_phy_status_cb(uint8_t link_up, int8_t driver_id);
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static int8_t _lowpan_init();
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _thread_handler();
	
	/** 
	*
	* \param
	* \param
	* \return
	*/
	static void _free_send(uint16_t id);
	
	/** Constructor
	* 
	* \param
	* \param
	*/
	LowpanBR();

};

#endif /* SOURCE_LOWPANBR_H_ */
