/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
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
 *******************************************************************************/

#if !defined(MQTTETHSOCKET_H)
#define MQTTETHSOCKET_H

#include "TCPSocket.h"
#include "EthernetInterface.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

class MQTTSocket
{
public:    

	bool connected = 0;

	MQTTSocket();

	~MQTTSocket();

	void init(NetworkInterface *itf);

	// 0 on success, negative if error
    int connect(char* hostname, int port, int timeout=1000);

    // 0 on success, negative if error
	int connect(const SocketAddress &address, int timeout=1000);

    int read(unsigned char* buffer, int len, int timeout);
    
    int write(unsigned char* buffer, int len, int timeout);

    bool is_connected();
    
    int disconnect();
    
    int bind(uint16_t port);

    static void attach_changed(void (*function)(void));

    /** TODO add brief
     *
     * TODO add long description
     *
     * @param bool
     *
     */
    template<typename T>
    static void attach_changed(T *object, void (T::*member)(void)) {
    	_socket_changed_callback.attach(object, member);
    }

private:

    // pouze pro ethernet
    TCPSocket *mysock = NULL;

    static Callback<void(void)>	_socket_changed_callback;

    static void change(void);

};

#endif /* MQTTETHSOCKET_H */
