#ifndef BYZANCE_CLIENT_H_
#define BYZANCE_CLIENT_H_

#include "mbed.h"

#include "byzance_debug.h"
#include "support_functions.h"

#include "ExtMem.h"
#include "extmem_config.h"

#include "byzance_config.h"

#include "MQTTSocket.h"
#include "MQTTClient.h"

#include "MqttBuffer.h"
#include "picojson.h"
#include "struct_mqtt.h"
#include "struct_wifi_credentials.h"

#include "ByzanceClientRoutines.h"
#include "Countdown.h"
#include "enum_netsource.h"
#include "MsgIdBuffer.h"

#define BYZANCE_CLIENT_SEND_MAIL_SIZE 5
#define BYZANCE_CLIENT_RECV_MAIL_SIZE 5
#define CLIENT_TIMEOUT 1000

/**
 * CONNECTION TYPE
 */
typedef enum
{
	CLIENT_CONNECTION_NORMAL	= 0x00,
	CLIENT_CONNECTION_BACKUP	= 0x01
} Connection_t;

/**
 * SOURCE TYPE
 */
typedef enum
{
	DEVTYPE_IODA		= 0x00,
	DEVTYPE_DEVICE		= 0x01
} DevType_t;

/**
 * Reasons why client failed
 */
typedef enum
{
	CLIENT_ERROR_OK,
	CLIENT_ERROR_UNSUBSRIBED,
	CLIENT_ERROR_TIMEOUT,
	CLIENT_ERROR_LOCKED,
	CLIENT_ERROR_DUPLICITY,
	CLIENT_ERROR_INVALID,
	CLIENT_ERROR_BUFFER_FULL
} ClientError_t;

/**
* \class ByzanceClient
* \brief TO DO.
*
* description TO DO.
*/
class ByzanceClient {

	public:

		
		 /**
	     * Initialize static class
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void init(ExtMem* extmem, char* client_id);
		
		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void init_itf(NetworkInterface* itf);

		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void set_netsource(Netsource_t source);

		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void connect(mqtt_connection *connection, mqtt_credentials *credentials);
		
		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void disconnect();
		
		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void stop();

		/**
	     * Determine if the client is connected
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static bool is_connected();

		/**
	     * Send message to homer 
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static int send_message(message_struct* ms, uint32_t timeout = CLIENT_TIMEOUT);

		/**
	     * Recieve message from homer
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static message_struct* get_message(uint32_t timeout=0);

	    /**
	     * Buffer overflow callback
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void attach_msg_thrown(void (*function)(uint32_t));
	    template<typename T>
		
		/**
	     * Buffer overflow callback
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void attach_msg_thrown(T *object, void (T::*member)(uint32_t)) {
	  	  _msg_thrown_callback.attach(object, member);
	    }

	    /**
	     * New message callback (IODA)
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void attach_msg_rcvd(void (*function)(void));
	    template<typename T>

		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void attach_msg_rcvd(T *object, void (T::*member)(void)) {
	  	  _msg_rcvd_callback.attach(object, member);
	    }

	    /**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void attach_connection_change(void (*function)(bool));
	    template<typename T>

		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void attach_connection_change(T *object, void (T::*member)(bool)) {
	  	  _connection_change_callback.attach(object, member);
	    }

	    /**
	     * Stack size getters
	     */
	    static uint32_t get_stack_size(void);
	    static uint32_t get_free_stack(void);
	    static uint32_t get_used_stack(void);
	    static uint32_t get_max_stack(void);
	    static Thread::State get_state(void);
	    static osPriority get_priority(void);
	    static void delete_incomming_msg(message_struct* ms);

	protected:

	    /**
	     * pointer na fyzickou vrstvu ETH nebo WIFI
	     */
	    static NetworkInterface*	_itf;

		static Netsource_t			 _netsource;
		static Connection_t 		 _client_connection;
		static uint32_t				 _connection_attempts;

	    /**
	     * pointer na socket vytvoreny nad ETH nebo WIFI
	     */
	    static MQTTSocket*			_mqttsocket;

	    /**
	     * samotny MQTT klient
	     */
	    static MQTT::Client*		_client;

	    /**
	     * Thread pointers
	     */
	    static Thread*				_thread_connection;

	    /**
	     * Broker informations
	     */
	    static mqtt_connection		_connection;
	    static mqtt_credentials		_credentials;

		static ExtMem*		_extmem;
		static bool			_client_connected;
		static bool			_client_subscribed;
		static char*		_client_id;

	    static char*		_topics[10];

	    //static Mutex 		_lock;

		static int subscribe_all(uint32_t retry, uint32_t timeout);

		/**
	     * Thread function
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void thread_connection();

		/**
	     * TO DO 
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static int send(MqttBuffer *buffer, uint32_t retry, uint32_t timeout);

		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void destroy_client();
		
		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		static void destroy_socket();

		 /**
	     * PAHO callbacks registered in subscribe
		 * 
		 * \param  
		 * \param
	     * \return
		 */
	    static void rcvd_packet_callback(MQTT::MessageData& md);

		
		
		static Callback<void(uint32_t)>	_msg_thrown_callback;
		static Callback<void(void)>		_msg_rcvd_callback;
	    static Callback<void(bool)>		_connection_change_callback;

	    static void socket_changed(void);

	private:

	    /*
	     * mail_box can hold up to BYZANCE_CLIENT_SEND_MAIL_SIZE of message_structs
	     */
	    static Mail<message_struct, BYZANCE_CLIENT_SEND_MAIL_SIZE> _send_mail;

	    /*
	     * _send_sem avoids allocation faults in mail_box
	     * this is workaround beacuse mail_box.alloc(timeout) is not implemented in mbed yet
	     * todo: use mail_box.alloc(osWaitForever) and get rid of _send_sem
	     */
	    static Semaphore _send_sem;

	    static Mail<message_struct, BYZANCE_CLIENT_RECV_MAIL_SIZE> _recv_mail;

		/**
	     * TO DO
		 * 
		 * \param  
		 * \param
	     * \return
		 */
		ByzanceClient() {};
		~ByzanceClient() {};
};

#endif /* BYZANCE_CLIENT_H_ */
