#include "ByzanceClient.h"

#include "nat64_dri.h"

REGISTER_DEBUG(Client);

mqtt_connection				ByzanceClient::_connection;
mqtt_credentials			ByzanceClient::_credentials;

// pole pointeru na topicy, do kterych se budu subscribovat
char*						ByzanceClient::_topics[10];

ExtMem*						ByzanceClient::_extmem;

bool 						ByzanceClient::_client_connected;
bool 						ByzanceClient::_client_subscribed;

/*
 * Pointery na fyzickou vrstvu wifiny a ethernetu a prepinac, kterym reknu, co se ma pouzit
 */
NetworkInterface*			ByzanceClient::_itf;

Netsource_t			 		ByzanceClient::_netsource;
char*						ByzanceClient::_client_id;
MQTTSocket*					ByzanceClient::_mqttsocket;

Connection_t 		 		ByzanceClient::_client_connection;
uint32_t			 		ByzanceClient::_connection_attempts;

MQTT::Client*				ByzanceClient::_client;

/*
 * Threads
 */
Thread*						ByzanceClient::_thread_connection;


/*
 * Callbacks
 */
Callback<void(uint32_t)>	ByzanceClient::_msg_thrown_callback;
Callback<void(void)>		ByzanceClient::_msg_rcvd_callback;
Callback<void(bool)>		ByzanceClient::_connection_change_callback;


Mail<message_struct, BYZANCE_CLIENT_SEND_MAIL_SIZE> 	ByzanceClient::_send_mail;
Semaphore 					ByzanceClient::_send_sem(BYZANCE_CLIENT_SEND_MAIL_SIZE);
Mail<message_struct, BYZANCE_CLIENT_RECV_MAIL_SIZE> ByzanceClient::_recv_mail;
//Mutex						ByzanceClient::_lock;

void ByzanceClient::init(ExtMem* extmem, char* full_id) {

	ByzanceClient::_extmem		= extmem;

	// nalepi do _client_id fullid, napriklad 004B00393533510B34353732
	_client_id = (char*)malloc(strlen(CLIENT_PREFIX) + strlen(full_id) + 1);
	sprintf(_client_id, "%s%s", CLIENT_PREFIX, full_id);

	client_fill_topics(_client_id, _topics);

	_client_connected = false;
	_client_subscribed = false;

	_client_connection = CLIENT_CONNECTION_NORMAL;
	__INFO("Loading NORMAL connection informations\n");

	_connection_attempts = 0;
	__DEBUG("number of connection attempts = %d\n", _connection_attempts);
  
	// vlákno zatím vždy využilo max 1416 bytů
	// dostane přiděleny 2 KiB
	// todo: pořešit priority vláken
	ByzanceClient::_thread_connection	= new Thread(osPriorityBelowNormal1, 2048,NULL,"ByzanceClient");

	ByzanceClient::_mqttsocket->attach_changed(&socket_changed);

}

void ByzanceClient::init_itf(NetworkInterface* itf){
	ByzanceClient::_itf 		= itf;
}
void ByzanceClient::set_netsource(Netsource_t source){
	ByzanceClient::_netsource	= source;

	switch(ByzanceClient::_netsource){
		case NETSOURCE_ETHERNET:
			__INFO("bude se pouzivat ETHERNET\n");
			break;
		case NETSOURCE_WIFI:
			__INFO("bude se pouzivat WIFI\n");
			break;
		default:
			__INFO("nezname rozhrani pro Internet\n");
	}
}

void ByzanceClient::connect(mqtt_connection *connection, mqtt_credentials *credentials) {

	memcpy(&_connection,	connection, sizeof(mqtt_connection));
	memcpy(&_credentials,	credentials, sizeof(mqtt_credentials));

	ByzanceClient::_thread_connection->start(ByzanceClient::thread_connection);
}

void ByzanceClient::disconnect() {
	destroy_client();
}

void ByzanceClient::stop() {
	ByzanceClient::_thread_connection->terminate();
	destroy_client();
}

#define THREAD_SIGNAL_SOCKET_CHANGED	0x01
#define THREAD_SIGNAL_INCOMING_MSG		0x02

// protected:
void ByzanceClient::thread_connection() {

	int rc = 0;

	__TRACE("pustilo se vlakno\n");

    /*
     * MQTT THREAD WHILE
     */
    while(true) {

    	/*
    	 * CREATE SOCKET
    	 */
    	if(ByzanceClient::_mqttsocket==NULL){

    		__TRACE("vytvarim mqtt socket\n");
			ByzanceClient::_mqttsocket = new MQTTSocket();

			ByzanceClient::_mqttsocket->init(ByzanceClient::_itf);

    	}

    	if(ByzanceClient::_mqttsocket->is_connected()){

	    	/*
	    	 * CREATE CLIENT
	    	 */
			if(_client==NULL){
				__TRACE("creating\n");
				_client = new MQTT::Client(*ByzanceClient::_mqttsocket, 5000);
				__TRACE("created\n");
			}

			/*
			 * CLIENT IS CONNECTED
			 */
			if (_client->isConnected()) {

				_client_connected = true;

				// wait for signal
				Thread::signal_wait(0, 1000);
				osEvent evt = _send_mail.get(0);		//get mail without waiting
				while(evt.status == osEventMail) {	//while mail is present
					__TRACE("incoming mail\n");
					message_struct *ms = (message_struct*)evt.value.p;
					//publish message
					rc = _client->publish(ms->topic.c_str(), (char*)ms->payload.c_str(), ms->payload.length());
					ms->~message_struct();	//call destructor
					_send_mail.free(ms);		//free mailbox
					_send_sem.release();	//release semaphore
					__TRACE("publish returned %d\n", rc);
					if(rc){
						__ERROR("publish error %d\n", rc);
					}
					evt = _send_mail.get(0);		//fill event to check if other mail is available
				}

				/*
				 * CLIENT YIELD, kde si MQTT klient přijme a vyblije svoje věci
				 * experimentalne prodlouzen timeout, jestli to nezpusobuje Tomovi pomale odezvy
				 */
				rc = _client->yield(1000);
				if(rc){
					__INFO(">>>>>>>>>  YIELD vratil rc = %d\n", rc);
				}

				//Thread::wait(1);

			} else {

				__WARNING("client is disconnected\n");

				_client_connected = false;
				_client_subscribed = false;

				// MQTT Connect
				MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
				connect_data.MQTTVersion = 3;
				connect_data.clientID.cstring = _client_id;
				connect_data.username.cstring = _credentials.username;
				connect_data.password.cstring = _credentials.password;
				connect_data.keepAliveInterval = 15;

				__INFO("connecting to broker\n");

				rc = _client->connect(connect_data);
				if(rc){
					__ERROR("connect to broker failed (err=%d)\n", rc);
					destroy_client();
					__TRACE("client is destroyed\n");
					Thread::wait(500);
				} else {
					__INFO("connected to %s:%d\n", ByzanceClient::_connection.hostname, ByzanceClient::_connection.port);

					_client_connected = true;

					/*
					 * MQTT SUBSCRIBE
					 */
					rc = ByzanceClient::subscribe_all(CLIENT_NUMBER_OF_SEND_ATTEMPTS, 1);
					if(rc<0){
						__ERROR("subscribe failed (err=%d)\n", rc);
						destroy_client();
					} else {
						__INFO("subscribed\n");

						_client_subscribed = true;

						if(_connection_change_callback){
							_connection_change_callback(true);
						}

					}
				}

				Thread::wait(1);

			}

		// socket disconnected
    	} else {

    		__WARNING("socket is disconnected\n");

    		_client_connected = false;
    		_client_subscribed = false;

    		switch(_netsource){

    		case NETSOURCE_DISABLED:
    			// TODO: stop thread
    			break;

#if BYZANCE_SUPPORTS_ETHERNET
    		case NETSOURCE_ETHERNET:
    		{
    			/*
    			 * TRY TO CONNECT SOCKET VIA ETH HOSTNAME
    			 */
    			__DEBUG("socket connecting to %s:%d, timeout=%d\n", _connection.hostname, _connection.port, 5000);
    			rc = ByzanceClient::_mqttsocket->connect((char*)_connection.hostname, _connection.port, 5000);
    			if(rc){
    				__WARNING("socket connect failed (error=%d)\n", rc);
    				destroy_socket();
    			} else {
    				__INFO("socket connected to %s:%d\n", ByzanceClient::_connection.hostname, ByzanceClient::_connection.port);
    			}
    			break;
    		}
#endif

#if BYZANCE_SUPPORTS_6LOWPAN
    		case NETSOURCE_6LOWPAN:
    		{
    			SocketAddress address;
    			ip_addr_t homer_ipv4;
    			if(ipaddr_aton(_connection.hostname,&homer_ipv4) == 1){		//if string was converted to  IPv4
    				char ip[16]=NAT_PREFIX;									//fill prefix
    				memcpy(&ip[12],&homer_ipv4.addr,4);						//append IPv4 to prefix
    				address.set_ip_bytes(ip, NSAPI_IPv6);					//set bytes and addr type
    				address.set_port(_connection.port);						//source port
    				ByzanceClient::_mqttsocket->bind(58543);				//fill source port
    				rc = ByzanceClient::_mqttsocket->connect(address, 5000);

    			}else{
    				//ipv4 conversion failed - try to use it as hostname
    				ByzanceClient::_mqttsocket->bind(58543);				//fill source port
    				rc = ByzanceClient::_mqttsocket->connect(_connection.hostname,_connection.port, 5000);
    			}

				if(rc){
					__WARNING("socket connect failed (error=%d)\n", rc);
					destroy_socket();
				} else {
					__INFO("socket connected to %s:%d\n", address.get_addr().bytes, address.get_port());
				}
        		break;
    		} // wrapper for compiler
#endif

#if BYZANCE_SUPPORTS_WIFI
    		case NETSOURCE_WIFI:
    			break;
#endif


    		default:
    			//__INFO("unknown interface = %d\n", _netsource);
    			break;

    		}

    	}

    	//__ERROR("**** otocilo se ****\n");
        Thread::wait(1);
    }
}

void ByzanceClient::destroy_client(){

	// todo: tady by se mohl udělat unsubscribe, pokud je _client_subscribed

	__TRACE("destroy_client: start\n");
	delete(_client);
	_client = NULL;

	destroy_socket();

	_client_connected = false;
	_client_subscribed = false;

	__TRACE("destroy_client: client destroyed\n");

	if(_connection_change_callback){
		_connection_change_callback.call(false);
	}

	//ByzanceClient::disconnect();

}

void ByzanceClient::destroy_socket(){
	__TRACE("destroy_socket: start\n");

	delete(ByzanceClient::_mqttsocket);
	ByzanceClient::_mqttsocket = NULL;

	__TRACE("destroy_socket: socket destroyed\n");

	Thread::wait(500);
}

// retry - kolikrat se klient pokusi o subscribe
// timeout - jaky casovy rozestup si dava mezi dalsima pokusama o subscribe
// return 0 - ok,
// return negative - error
// return positive - number of subscribed topics
int ByzanceClient::subscribe_all(uint32_t retry, uint32_t timeout){

	int cnt = 0;
	int rc = 0;
	uint32_t send_attempt = 0;

	while(_topics[cnt]!=NULL){
		__DEBUG("subscribe to topic %s\n", _topics[cnt]);
		do{

			rc = _client->subscribe(_topics[cnt], MQTT::QOS0, &ByzanceClient::rcvd_packet_callback);

			if(rc){
				__ERROR("subscribe to topic %s failed (err=%d), attempt #%d/%d\n", _topics[cnt], rc, send_attempt, CLIENT_NUMBER_OF_SEND_ATTEMPTS);

				send_attempt++;

				if(send_attempt>=retry){
					__TRACE("subscribe failed (%s)\n", _topics[cnt]);

					// vrati se zaporna hodnota pokusu, na kterem to ztroskotalo
					// kladne hodnoty jsou "ok", zaporne jsou chybove
					return -cnt;
				}

				Thread::wait(timeout);

			} else {
				send_attempt = 0;
			}

			Thread::wait(1);

		} while(rc);

		cnt++;

	}

	return cnt;

}

/*
 * Do tohoto callbacku skáče program automaticky z PAHO klientu, pokud něco přijde do jednoho z topiců viz níže
 *
 * - command_in
 * - settings_in
 * - info_in
 *
 * - a_in
 * - d_in
 * - m_in
 *
 * - debug_in
 *
 * Cut off prefix full id AAAAAAAABBBBBBBBCCCCCCCC/ (24 bytes)
 * Funkce přidá příchozí zprávu do mailu mezi příchozí zprávy.
 * Processing zprávy se pak dělá jindy
 */
void ByzanceClient::rcvd_packet_callback(MQTT::MessageData& md) {
	if(!_client_subscribed || !_client_connected){
		__WARNING("message thrown - client not connected or subscribed\n");
		if(_msg_thrown_callback){
			_msg_thrown_callback.call(CLIENT_ERROR_UNSUBSRIBED);
		}
		return;
	}

	std::size_t pos;
	string topic_tmp;

	__INFO("message arrived\n");

	topic_tmp.assign((char*)md.topicName.lenstring.data, md.topicName.lenstring.len);

	// topic looks like AAAAAAAABBBBBBBBCCCCCCCC/command_in/foo

	// find first slash
	pos=topic_tmp.find("/");

	// remove prefix AAAAAAAABBBBBBBBCCCCCCCC/ and save message
	if (pos!=std::string::npos){
		void *mem = _recv_mail.alloc();
		if(mem == NULL){
			__ERROR("message thrown - mail is full\n");
			if(_msg_thrown_callback){
				_msg_thrown_callback.call(CLIENT_ERROR_BUFFER_FULL);
			}
			return;
		}
		message_struct* msg = new(mem) message_struct;	//call constructor

		msg->topic.assign(topic_tmp.substr(pos+1));
		msg->payload.assign((char*)md.message.payload, md.message.payloadlen);

		__TRACE("storing msg to mail\n");
		_recv_mail.put(msg);	//put to mail
	}

	//_client->yield(1000);

	if(_msg_rcvd_callback){
		_msg_rcvd_callback.call();
	}
}

/*
 * vymaže zprávu z přijímacího mailu
 */
void ByzanceClient::delete_incomming_msg(message_struct* ms){
	__TRACE("deleting msg from mail\n");
	ms->~message_struct();
	_recv_mail.free(ms);
}

/*
 * Vrací 0, pokud se podařilo přidat zprávu k odeslání
 * jinak vrací nějaké číslo chyby
 *
 * prefix AAAAAAAABBBBBBBBCCCCCCCC/ is added automatically
 */
int ByzanceClient::send_message(message_struct* ms, uint32_t timeout){

	if(!(_client_connected)||!(_client_subscribed)){
		__WARNING("not connected or subscribed yet\n");
		return -1;
	}

	__TRACE("waiting on semaphore\n");
	_send_sem.wait(timeout);		//wait on semaphore
	__TRACE("semaphore waiting done\n");
	void *mem = _send_mail.alloc();	//allocate memory
	if(mem==NULL){
		__ERROR("failed to allocate msg in buffer\n");
		return -2;
	}

	message_struct* msg = new(mem) message_struct;	//call constructor to init strings
	msg->payload = ms->payload;		//fill mail
	msg->topic = _client_id;
	msg->topic += "/";
	msg->topic+=ms->topic;
	_send_mail.put(msg);				//send mail
	_thread_connection->signal_set(THREAD_SIGNAL_INCOMING_MSG);	//set signal
	__TRACE("signal set\n");
	return 0;
/*
	if(_lock.lock(timeout)!=osOK){
		__ERROR("mutex was not locked\n");
		return -2;
	}

	if(!(_client_connected)||!(_client_subscribed)){
		__TRACE("send_message: ERROR: not connected or subscribed\n");
		_lock.unlock();
		return -1;
	}

	__DEBUG("send_message continue\n");

	// client_id + topic
	string topic;
	topic.assign(_client_id);
	topic += "/";
	topic+=ms->topic;

	int rc = 0;
	ms->topic.assign(topic);

	rc = _client->publish(ms->topic.c_str(), (char*)ms->payload.c_str(), ms->payload.length());
	__TRACE("publish returned %d\n", rc);
	if(rc){
		__ERROR("publish error %d\n", rc);
	}

	// for some reason it must be here
	// refer to test_msgout
	// when enormous shitload of messages is sent while ping is performed
	// lwpip is disconnected

	_lock.unlock();

	Thread::wait(1);

	return rc;
	*/
}

/*
 * Vrací pointer, pokud je nějaká zpráva k přečtení
 * jinak vrací NULL
 */
message_struct* ByzanceClient::get_message(uint32_t timeout){
	__TRACE("picking from mail\n");
	osEvent evt = _recv_mail.get(timeout);
	if(evt.status == osEventMail){
		return (message_struct*) evt.value.p;
	}
	return NULL;
}

bool ByzanceClient::is_connected(){
	if((_client_connected)&&(_client_subscribed)){
		//__DEBUG("is_connected: returning true\n");
		return 1;
	}

	//__DEBUG("is_connected: returning false\n");
	return 0;
}

/*
 * Function is called when client throws some message away for some reason
 */
void ByzanceClient::attach_msg_thrown(void (*function)(uint32_t)) {
	_msg_thrown_callback = function;
}

void ByzanceClient::attach_msg_rcvd(void (*function)(void)) {
	  _msg_rcvd_callback = function;
}

void ByzanceClient::attach_connection_change(void (*function)(bool)) {
	_connection_change_callback = function;
}

uint32_t ByzanceClient::get_stack_size(void) {
	return _thread_connection->stack_size();
}

uint32_t ByzanceClient::get_free_stack(void) {
	return _thread_connection->free_stack();
}

uint32_t ByzanceClient::get_used_stack(void) {
	return _thread_connection->used_stack();
}

uint32_t ByzanceClient::get_max_stack(void) {
	return _thread_connection->max_stack();
}

Thread::State ByzanceClient::get_state(void){
	return _thread_connection->get_state();
}

osPriority ByzanceClient::get_priority(void){
	return _thread_connection->get_priority();
}

void ByzanceClient::socket_changed(void){
	//__ERROR("CLIENT_SOCKET_CHANGED\n");

	_thread_connection->signal_set(THREAD_SIGNAL_SOCKET_CHANGED);
}
