#include "MQTTClient.h"

REGISTER_DEBUG(paho);

MQTT::Client::Client(MQTTSocket& network, unsigned int command_timeout_ms)  : ipstack(network), packetid() {

	__TRACE("zacatek client konstruktoru\n");

    ping_waiting = false;
    for (int i = 0; i < MAX_MESSAGE_HANDLERS; ++i){
        messageHandlers[i].topicFilter = 0;
    }
    this->command_timeout_ms = command_timeout_ms;
    isconnected = false;

    __TRACE("konec client konstruktoru\n");

}



int MQTT::Client::sendPacket(int length, Countdown& countdown) {

	__TRACE("zacatek sendPacket\n");

	BYZANCE_UNUSED_ARG(countdown);
    int rc = FAILURE;
	int sent = 0;
        
    while ((sent < length) && (!countdown.expired())) {
        rc = ipstack.write(&sendbuf[sent], length - sent, countdown.left_ms());

        // there was an error writing the data
        if (rc < 0)  {
            break;
        }
        sent += rc;
    }

    if (sent == length)  {
        if (this->keepAliveInterval > 0) {
            last_sent.countdown(this->keepAliveInterval); // record the fact that we have successfully sent the packet
        }
        rc = SUCCESS;
    }  else {
        rc = FAILURE;
    }
        
    __TRACE("konec sendPacket, rc = %d\n", rc);

    return rc;
}

int MQTT::Client::decodePacket(int* value, int timeout){

	__TRACE("zacatek decodePacket\n");

    unsigned char c;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do {
    	int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            __ERROR("bad data\n");
            goto exit;
        }

        rc = ipstack.read(&c, 1, 100);
        if (rc != 1){
        	__ERROR("decode failed\n");
            goto exit;
        }

        *value += (c & 127) * multiplier;
        multiplier *= 128;

    } while ((c & 128) != 0);
exit:

	__TRACE("konec decodePacket, vraci se %d\n", len);

    return len;
}


/**
 * If any read fails in this method, then we should disconnect from the network, as on reconnect
 * the packets can be retried.
 * @param timeout the max time to wait for the packet read to complete, in milliseconds
 * @return the MQTT packet type, or -1 if none
 */
int MQTT::Client::readPacket(Countdown& countdown) {

	int ret = 0;

	__TRACE("zacatek readPacket, timeout = %d\n", countdown.left_ms());

    int rc = FAILURE;
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;
	//int read = 0;

    //TODO: promyslet, jestli timer.left_ms nedat natvrdo na 1500,
    // stejne jako je to u sendPacket

    /* 1. read the header byte.  This has the packet type in it */

    // Third parameter is wait time. Its not necessary to wait here if there is no new packet to read.
    rc = ipstack.read(readbuf, 1, 1);
    if (rc != 1){

    	// pokud prostě jenom nejsou data, vrací se NSAPI_ERROR_WOULD_BLOCK
    	// to není úplně chyba -> nevypisuju
    	if(rc!=NSAPI_ERROR_WOULD_BLOCK){
    		__ERROR("read failed, rc=%d\n", rc);
        	//isconnected = false;
    	}

        goto exit;
    }

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(&rem_len, countdown.left_ms());
    len += MQTTPacket_encode(readbuf + 1, rem_len); /* put the original remaining length into the buffer */

    __TRACE("readPacket: MAX_MQTT_PACKET_SIZE = %d, rem_len = %d, len = %d\n", MAX_MQTT_PACKET_SIZE, rem_len, len);

    /*
     * Reamaining packet size to read is bigger than MAX_MQTT_PACKET_SIZE -> exit
     */
	if (rem_len > (MAX_MQTT_PACKET_SIZE - len))	{

		__WARNING("readPacket: buffer overflow\n");

		uint32_t read_size = 0;

		// flush all the remaining data
		do{

			// calculate maximal allowed size to flush
			if(rem_len>MAX_MQTT_PACKET_SIZE){
				read_size = MAX_MQTT_PACKET_SIZE;
			} else {
				read_size = rem_len;
			}

			__TRACE("rem_len = %d, read_size = %d\n", rem_len, read_size);

			rc = ipstack.read(readbuf, read_size, countdown.left_ms());
			if(rc < 0){	//in case of error
				if (rc != NSAPI_ERROR_WOULD_BLOCK)
				{	//error - other error
					__ERROR("socket read error - other error %d\n",rc);
					return 0;
				}else{	//error - would block
					__ERROR("reading would block, waiting\n");
					Thread::wait(50);	//wait and try it once again
				}
			}else{	//data were read
				__TRACE("read %d bytes; remaining time %d\n", rc, countdown.left_ms());
				rem_len -= rc;
			}

		} while(rem_len);

		__WARNING("msg thrown away\n");

		rc = BUFFER_OVERFLOW;
		goto exit;
	}

	if(rem_len){
		__TRACE("trying to read data from socket\n");
		rc = ipstack.read(readbuf + len, rem_len, countdown.left_ms());
		if(rc != rem_len){
			__ERROR("error reading from socket - countdown expired or socket error rc = %d\n",rc);
			goto exit;
		}
	}

	// dokud je něco třeba ještě číst, tak to snažím číst;
	// když vyprší timeout na čtení, vyskočím na konec s chybou
	/*
	while(rem_len){

		// return positive - number of received bytes
		// negative - error code
		ret = ipstack.read(readbuf + len, rem_len, 10);
		//__ERROR("readPacket pred: timeout = %d, rem_len = %d, read = %d, len = %d\n", countdown.left_ms(), rem_len, ret, len);

		if(ret>0){
			len+=ret;
			rem_len-=ret;
		} else {
            __ERROR("read failed, waitign, rem_len = %d, read = %d, len = %d\n", rem_len, ret, len);
			Thread::wait(70);
			//isconnected = false;
			//goto exit;
		}

		//__ERROR("readPacket po: timeout = %d, rem_len = %d, read = %d, len = %d\n", countdown.left_ms(), rem_len, ret, len);

		if(countdown.left_ms()<=0){
			__ERROR("readPacket: vyprsel timeout pro cteni, t=%d, neprecetlo se %d\n", countdown.left_ms(), rem_len);
			//goto exit;
		}
	}*/

	__TRACE("readPacket: all data are read, t=%d\n", countdown.left_ms());

    header.byte = readbuf[0];
    rc = header.bits.type;
    if (this->keepAliveInterval > 0){
        last_received.countdown(this->keepAliveInterval); // record the fact that we have successfully received a packet
    }

    exit:
        
	__TRACE("konec readPacket, rc=%d\n", rc);

    return rc;
}


// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
bool MQTT::Client::isTopicMatched(char* topicFilter, MQTTString& topicName){

	__TRACE("zacatek isTopicMatched\n");

    char* curf = topicFilter;
    char* curn = topicName.lenstring.data;
    char* curn_end = curn + topicName.lenstring.len;

    while (*curf && curn < curn_end)  {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+') {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        } else if (*curf == '#'){
            curn = curn_end - 1;    // skip until end of string
        }
        curf++;
        curn++;
    }

    __TRACE("konec isTopicMatched\n");

    return (curn == curn_end) && (*curf == '\0');
}

/*
 * Projede seznam subscribovaných topiců
 * pokud se topicName shoduje s nějakým subscribovaným
 * zavolá callback o nové zprávě
 * @return - SUCCESS (0) - prijaty topic odpovida subscribovanemu a byl zavolan callback o prijeti
 * @return - FAILURE (-1) - prijaty topic neodpovida zadnemu subscribovanemu
 */
int MQTT::Client::deliverMessage(MQTTString& topicName, Message& message) {
	__TRACE("zacatek deliverMessge\n");

    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (int i = 0; i < MAX_MESSAGE_HANDLERS; ++i){

    	// topic is null -> go to next topic
    	if(messageHandlers[i].topicFilter == 0){
    		continue;
    	}

    	// equals = topicy jsou stejne
    	// isTopicMatched = topicy nejsou stejne, protoze je obsazeny wildchar, napr. #
    	// ale prijaty topic spada do patternu
    	if (MQTTPacket_equals(&topicName, (char*)messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)messageHandlers[i].topicFilter, topicName))
        {
            if (messageHandlers[i].fp.attached()) {
                MessageData md(topicName, message);
                messageHandlers[i].fp(md);

                __TRACE("deliver ok\n");

                rc = SUCCESS;
            }
        }
    }

    __TRACE("konec deliverMessage\n");

    return rc;
}

int MQTT::Client::yield(unsigned long timeout_ms) {

	//__DEBUG("skocili jsme do yield\n");
    int rc = FAILURE;
    Countdown countdown;

    //ByzanceLogger::debug("Spoustime countdown\n");
    countdown.countdown_ms(timeout_ms);

	rc = keepalive();
	if(rc){
		isconnected = false;
		__ERROR("ODPOJIL SE KLIENT, rc = -2\n");
		return -2;
	}


	if (cycle(countdown) < 0) {
		//__DEBUG("vracime failure\n");
		// v cycle se nic nestalo

		return SUCCESS;
	}


    return rc;
}

int MQTT::Client::cycle(Countdown& countdown) {

    // read the socket, see what work is due
    int packet_type = readPacket(countdown);

    int len = 0;
    int rc = SUCCESS;

    switch (packet_type)
    {
		case FAILURE:
		case BUFFER_OVERFLOW:
			rc = packet_type;
			break;
        case CONNACK:
        case PUBACK:
        case SUBACK:
            break;
        case PUBLISH:
		{
            MQTTString topicName = MQTTString_initializer;
            Message msg;
            if (MQTTDeserialize_publish((unsigned char*)&msg.dup, (int*)&msg.qos, (unsigned char*)&msg.retained, (unsigned short*)&msg.id, &topicName,
                                 (unsigned char**)&msg.payload, (int*)&msg.payloadlen, readbuf, MAX_MQTT_PACKET_SIZE) != 1){
                	goto exit;
            	}

            	int rc2 = 0;
                rc2 = deliverMessage(topicName, msg);

            	__TRACE("deliver rc2 = %d\n", rc2);

                // TODO: break?!
                break;

		}

        case PINGRESP:
            _ping_timer.stop();
            __ERROR("ping took %d\n",_ping_timer.read_ms());
        	//ByzanceLogger::debug(">>>>>>>> Pin outstanding se VYPL\n");
            ping_waiting = false;
            break;
    }

exit:
    if (rc == SUCCESS){
        rc = packet_type;
    }

    return rc;
}

int MQTT::Client::keepalive() {
    int rc = SUCCESS;

    if (keepAliveInterval == 0){
        return SUCCESS;
    }

    //chci poslat ping, ale jeste neprisla odpoved na predchozi
    if(last_sent.expired() && ping_waiting){
    	__ERROR("ping resp wasnt recieved in time\n");
    	return FAILURE;
    }

	//pokud vyprsel aleson jeden z timeoutu a neceka se na odpoved od pingu
    if ((last_sent.expired() || last_received.expired()) && !ping_waiting){
    	__DEBUG("trying to send ping\n");

        Countdown countdown(1000);
        int len = MQTTSerialize_pingreq(sendbuf, MAX_MQTT_PACKET_SIZE);

        // ping packet se podarilo serializovat a odeslat
        if (len > 0 && (rc = sendPacket(len, countdown)) == SUCCESS) {
        	__TRACE(">>>> POSLAL JSEM PING, setting ping outstanding\n");
            _ping_timer.reset();
            _ping_timer.start();
            ping_waiting = true;
            return SUCCESS;

        // ping packet se nepodarilo serializovat, nebo odeslat
        } else {
        	__ERROR(">>>> PING PROBLEM -> bude se odpojovat\n");

        	return FAILURE;
        }
    }

    return rc;
}


// only used in single-threaded mode where one command at a time is in process
int MQTT::Client::waitfor(int packet_type, Countdown& countdown) {

	int rc = FAILURE;

    do {
        if (countdown.expired()){
            break; // we timed out
        }
    } while ((rc = cycle(countdown)) != packet_type);

    return rc;
}

int MQTT::Client::connect(MQTTPacket_connectData& options) {

	__TRACE("connect: zacatek funkce\n");

    Countdown countdown(command_timeout_ms);
    int rc = FAILURE;
    int len = 0;

    if (isconnected) // don't send connect packet again if we are already connected
        goto exit;

    this->keepAliveInterval = options.keepAliveInterval;
    this->cleansession = options.cleansession;

    if ((len = MQTTSerialize_connect(sendbuf, MAX_MQTT_PACKET_SIZE, &options)) <= 0){
        goto exit;
    }

    // send the connect packet
    if ((rc = sendPacket(len, countdown)) != SUCCESS) {
        goto exit; // there was a problem
    }

    __TRACE("connect: sendPacket prosel");

    if (this->keepAliveInterval > 0)
        last_received.countdown(this->keepAliveInterval);
    // this will be a blocking call, wait for the connack
    if (waitfor(CONNACK, countdown) == CONNACK) {
    	__TRACE("connect: cekam na connack\n");
        unsigned char connack_rc = 255;
        bool sessionPresent = false;
        if (MQTTDeserialize_connack((unsigned char*)&sessionPresent, &connack_rc, readbuf, MAX_MQTT_PACKET_SIZE) == 1){
            rc = connack_rc;
        } else {
            rc = FAILURE;
        }
    } else {
    	rc = FAILURE;
    	__TRACE("connect: connack neprisel\n");
    }

exit:
	__TRACE("connect: konec funkce (rc = %d)\n", rc);

    if (rc == SUCCESS){
        isconnected = true;
    }

    return rc;
}

int MQTT::Client::connect() {
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    return connect(default_options);
}

int MQTT::Client::subscribe(const char* topicFilter, enum QoS qos, messageHandler messageHandler) {

	__TRACE("subscribe: zacatek subscribe\n");

    int rc = FAILURE;
    Countdown countdown(command_timeout_ms);
    int len = 0;
    MQTTString topic = {(char*)topicFilter, 0, 0};

    if (!isconnected){
        goto exit;
    }

    len = MQTTSerialize_subscribe(sendbuf, MAX_MQTT_PACKET_SIZE, 0, packetid.getNext(), 1, &topic, (int*)&qos);

    if (len <= 0){
        goto exit;
    }

    // send the subscribe packet
    if ((rc = sendPacket(len, countdown)) != SUCCESS){
        goto exit;             // there was a problem
    }

    __TRACE("subscribe: sendPacket vratil %d\n", rc);

    // wait for suback
    if (waitfor(SUBACK, countdown) == SUBACK) {

    	int count = 0;
        int grantedQoS = -1;
        unsigned short mypacketid;

        if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, readbuf, MAX_MQTT_PACKET_SIZE) == 1){
            rc = grantedQoS; // 0, 1, 2 or 0x80
        }

        if (rc != 0x80) {
            for (int i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
                if (messageHandlers[i].topicFilter == 0) {
                    messageHandlers[i].topicFilter = topicFilter;
                    messageHandlers[i].fp.attach(messageHandler);
                    rc = 0;
                    break;
                }
            }
        }
    } else {
        rc = FAILURE;
    }

exit:

	__TRACE("subscribe: konec subscribe, rc = %d\n", rc);

    if (rc != SUCCESS){
        isconnected = false;
    }

    return rc;
}

int MQTT::Client::unsubscribe(const char* topicFilter) {

	__TRACE("unsubscribe: zacatek unsubscribe\n");

    int rc = FAILURE;
    Countdown countdown(command_timeout_ms);
    MQTTString topic = {(char*)topicFilter, 0, 0};
    int len = 0;

    if (!isconnected)
        goto exit;

    if ((len = MQTTSerialize_unsubscribe(sendbuf, MAX_MQTT_PACKET_SIZE, 0, packetid.getNext(), 1, &topic)) <= 0){
        goto exit;
    }

    // send the unsubscribe packet
    if ((rc = sendPacket(len, countdown)) != SUCCESS){
        goto exit; // there was a problem
    }

    if (waitfor(UNSUBACK, countdown) == UNSUBACK)  {
        unsigned short mypacketid;  // should be the same as the packetid above
        if (MQTTDeserialize_unsuback(&mypacketid, readbuf, MAX_MQTT_PACKET_SIZE) == 1){
            rc = 0;
        }
    }  else {
        rc = FAILURE;
    }

exit:
	__TRACE("unsubscribe: konec unsubscribe, rc = %d", rc);

    if (rc != SUCCESS){
        isconnected = false;
    }

    return rc;
}

int MQTT::Client::publish(const char* topicName, void* payload, size_t payloadlen, int timeout){

	__TRACE("zacatek");

    int rc = FAILURE;

    Countdown countdown(timeout);

    MQTTString topicString = MQTTString_initializer;

    int len = 0;

    topicString.cstring = (char*)topicName;

    len = MQTTSerialize_publish(sendbuf, MAX_MQTT_PACKET_SIZE, 0, MQTT::QOS0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
    if (len <= 0){
    	__ERROR("serialize error\n");
    	return FAILURE;
    }

    rc = sendPacket(len, countdown);
    if (rc != SUCCESS){
    	isconnected = false;
    	__ERROR("not published\n");
        return FAILURE;
    }

    return SUCCESS;
}

