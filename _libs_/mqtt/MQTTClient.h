/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - fix for bug 458512 - QoS 2 messages
 *    Ian Craggs - fix for bug 460389 - send loop uses wrong length
 *******************************************************************************/

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "FP.h"
#include "MQTTPacket.h"
#include "stdio.h"
#include "macros_byzance.h"
#include "byzance_debug.h"
#include "Countdown.h"
#include "byzance_config.h"
#include "MQTTSocket.h"

namespace MQTT
{


enum QoS {QOS0};

// all failure return codes must be negative
enum returnCode { BUFFER_OVERFLOW = -2, FAILURE = -1, SUCCESS = 0 };


struct Message
{
    enum QoS qos;
    bool retained;
    bool dup;
    unsigned short id;
    void *payload;
    size_t payloadlen;
};


struct MessageData
{
    MessageData(MQTTString &aTopicName, struct Message &aMessage)  : message(aMessage), topicName(aTopicName)
    { }

    struct Message &message;
    MQTTString &topicName;
};


class PacketId
{
public:
    PacketId()
    {
        next = 0;
    }

    int getNext()
    {
        return next = (next == MAX_PACKET_ID) ? 1 : next+1;
    }

private:
    static const int MAX_PACKET_ID = 65535;
    int next;
};


/**
 * @class Client
 * @brief blocking, non-threaded MQTT client API
 *
 * This version of the API blocks on all method calls, until they are complete.  This means that only one
 * MQTT request can be in process at any one time.
 * @param Network a network class which supports send, receive
 * @param Timer a timer class with the methods:
 */
class Client
{

public:

    typedef void (*messageHandler)(MessageData&);

    /** Construct the client
     *  @param network - pointer to an instance of the Network class - must be connected to the endpoint
     *      before calling MQTT connect
     *  @param limits an instance of the Limit class - to alter limits as required
     */
    Client(MQTTSocket& network, unsigned int command_timeout_ms = 30000);

    /** MQTT Connect - send an MQTT connect packet down the network and wait for a Connack
     *  The nework object must be connected to the network endpoint before calling this
     *  Default connect options are used
     *  @return success code -
     */
    int connect();
    
        /** MQTT Connect - send an MQTT connect packet down the network and wait for a Connack
     *  The nework object must be connected to the network endpoint before calling this
     *  @param options - connect options
     *  @return success code -
     */
    int connect(MQTTPacket_connectData& options);
    
    /** MQTT Publish - send an MQTT publish packet and wait for all acks to complete for all QoSs
     *  @param topic - the topic to publish to
     *  @param payload - the data to send
     *  @param payloadlen - the length of the data
     *  @return success code -
     */
    int publish(const char* topicName, void* payload, size_t payloadlen, int timeout = 1000);

    /** MQTT Subscribe - send an MQTT subscribe packet and wait for the suback
     *  @param topicFilter - a topic pattern which can include wildcards
     *  @param qos - the MQTT QoS to subscribe at
     *  @param mh - the callback function to be invoked when a message is received for this subscription
     *  @return success code -
     */
    int subscribe(const char* topicFilter, enum QoS qos, messageHandler mh);

    /** MQTT Unsubscribe - send an MQTT unsubscribe packet and wait for the unsuback
     *  @param topicFilter - a topic pattern which can include wildcards
     *  @return success code -
     */
    int unsubscribe(const char* topicFilter);

    /** MQTT Disconnect - send an MQTT disconnect packet, and clean up any state
     *  @return success code -
     */
    int disconnect();

    /** A call to this API must be made within the keepAlive interval to keep the MQTT connection alive
     *  yield can be called if no other MQTT operation is needed.  This will also allow messages to be
     *  received.
     *  @param timeout_ms the time to wait, in milliseconds
     *  @return success code - on failure, this means the client has disconnected
     */
    int yield(unsigned long timeout_ms = 1000L);

    /** Is the client connected?
     *  @return flag - is the client connected or not?
     */
    bool isConnected()
    {
        return isconnected;
    }

private:

    int cycle(Countdown& countdown);
    int waitfor(int packet_type, Countdown& countdown);
    int keepalive();

    int decodePacket(int* value, int countdown);
    int readPacket(Countdown& countdown);
    int sendPacket(int length, Countdown& countdown);
    int deliverMessage(MQTTString& topicName, Message& message);
    bool isTopicMatched(char* topicFilter, MQTTString& topicName);

    MQTTSocket& ipstack;
    unsigned long command_timeout_ms;

    unsigned char sendbuf[MAX_MQTT_PACKET_SIZE];
    unsigned char readbuf[MAX_MQTT_PACKET_SIZE];

    Countdown last_sent, last_received;
    unsigned int keepAliveInterval;
    bool ping_waiting;
    bool cleansession;

    PacketId packetid;

    struct MessageHandlers
    {
        const char* topicFilter;
        FP<void, MessageData&> fp;
    } messageHandlers[MAX_MESSAGE_HANDLERS];      // Message handlers are indexed by subscription topic

    //FP<void, MessageData&> defaultMessageHandler;

    Timer _ping_timer;

    bool isconnected;

};

}

#endif
