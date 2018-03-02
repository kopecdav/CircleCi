/*
 * TLSSocket.cpp
 *
 *  Created on: 24. 10. 2017
 *      Author: martin
 */
#include "TLSSocket.h"

REGISTER_DEBUG(TLSSocket);

TLSSocket::TLSSocket(NetworkInterface *itf){
	_tcpsocket = new TCPSocket(itf);
	mbedtls_entropy_init(&_entropy);
    mbedtls_ctr_drbg_init(&_ctr_drbg);
    mbedtls_x509_crt_init(&_cacert);
    mbedtls_ssl_init(&_ssl);
    mbedtls_ssl_config_init(&_ssl_conf);
}

TLSSocket::~TLSSocket(){
	mbedtls_entropy_free(&_entropy);
    mbedtls_ctr_drbg_free(&_ctr_drbg);
    mbedtls_x509_crt_free(&_cacert);
    mbedtls_ssl_free(&_ssl);
    mbedtls_ssl_config_free(&_ssl_conf);
    _tcpsocket->close();
    delete _tcpsocket;
}

nsapi_error_t TLSSocket::connect(const char *host, uint16_t port){
	__TRACE("connecting to %s:%d\n",host,port);
	int ret;
    if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                      NULL,
                      NULL)) != 0) {
        print_mbedtls_error("mbedtls_crt_drbg_init", ret);
        return ret;
    }
    if ((ret = mbedtls_x509_crt_parse(&_cacert, (const unsigned char *) SSL_CA_PEM,
                       sizeof (SSL_CA_PEM))) != 0) {
        print_mbedtls_error("mbedtls_x509_crt_parse", ret);
        return ret;
    }
    if ((ret = mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        print_mbedtls_error("mbedtls_ssl_config_defaults", ret);
        return ret;
    }
    mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);
    mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);

    /* It is possible to disable authentication by passing
     * MBEDTLS_SSL_VERIFY_NONE in the call to mbedtls_ssl_conf_authmode()
     */
    mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    if ((ret = mbedtls_ssl_setup(&_ssl, &_ssl_conf)) != 0) {
        print_mbedtls_error("mbedtls_ssl_setup", ret);
        return ret;
    }
    mbedtls_ssl_set_hostname(&_ssl, host);
    mbedtls_ssl_set_bio(&_ssl, static_cast<void *>(_tcpsocket),
                               _ssl_send, _ssl_recv, NULL );

    /* Connect to the server */
    __INFO("TCPSocket connecting with %s\r\n", host);
    ret = _tcpsocket->connect(host, port);
    if (ret != NSAPI_ERROR_OK) {
    	__ERROR("TCPSocket failed to connect%d\n",ret);
        _tcpsocket->close();
        return ret;
    }

   /* Start the handshake, the rest will be done in onReceive() */
    __INFO("starting the TLS handshake...\n");
    do {
        ret = mbedtls_ssl_handshake(&_ssl);
    } while (ret != 0 && (ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE));
    if (ret < 0) {
        print_mbedtls_error("mbedtls_ssl_handshake", ret);
        _tcpsocket->close();
        return ret;
    }
    /* It also means the handshake is done, time to print info */
    __INFO("TLS connection to %s established\n", host);

  /*
   * we dont want to print cert info by default
   * uncomment this to print cert info
   *
    char buf[1024];
    const uint32_t buf_size = sizeof(buf);

    mbedtls_x509_crt_info(buf, buf_size, "\r    ",
                    mbedtls_ssl_get_peer_cert(&_ssl));
    __INFO("server certificate:\n%s\n\n", buf);*/
    uint32_t flags = mbedtls_ssl_get_verify_result(&_ssl);
    if( flags != 0 )
    {
       // mbedtls_x509_crt_verify_info(buf, buf_size, "\r  ! ", flags);
        __ERROR("certificate verification failed\n");
    }
    else{
    	__INFO("certificate verification passed\n");
    }
    return NSAPI_ERROR_OK;
}

nsapi_error_t TLSSocket::close(){
	return _tcpsocket->close();
}

nsapi_size_or_error_t TLSSocket::send(const void *data, nsapi_size_t size){
        int ret = mbedtls_ssl_write(&_ssl,(const unsigned char *) data,size);
        if(ret == MBEDTLS_ERR_SSL_WANT_READ){
        	return NSAPI_ERROR_WOULD_BLOCK;
        }
        return ret;
}

nsapi_size_or_error_t TLSSocket::recv(void *data, nsapi_size_t size){
	int ret=mbedtls_ssl_read(&_ssl, (unsigned char *) data,size);
	if(ret==MBEDTLS_ERR_SSL_WANT_READ){
		return NSAPI_ERROR_WOULD_BLOCK;
	}
    return ret;
};

void TLSSocket::set_blocking(bool blocking){
	_tcpsocket->set_blocking(blocking);
}

void TLSSocket::set_timeout(int timeout){
	_tcpsocket->set_timeout(timeout);
}

nsapi_error_t TLSSocket::bind(const char *address, uint16_t port){
	return _tcpsocket->bind(address,port);
}

nsapi_error_t TLSSocket::bind(const SocketAddress &address){
	return _tcpsocket->bind(address);
}

int TLSSocket::_ssl_recv(void *ctx, unsigned char *buf, size_t len) {
    int recv = -1;
    TCPSocket *socket = static_cast<TCPSocket *>(ctx);
    recv = socket->recv(buf, len);
    if(NSAPI_ERROR_WOULD_BLOCK == recv){
        return MBEDTLS_ERR_SSL_WANT_READ;
    }else if(recv < 0){
    	__ERROR("TCPSocket recv error %d\r\n", recv);
        return -1;
    }else{
        return recv;
    }
}

/**
 * Send callback for mbed TLS
 *
 * this defines how to send raw encrypted data to TCPSocket
 */
int TLSSocket::_ssl_send(void *ctx, const unsigned char *buf, size_t len) {
   int size = -1;
    TCPSocket *socket = static_cast<TCPSocket *>(ctx);
    size = socket->send(buf, len);
    if(NSAPI_ERROR_WOULD_BLOCK == size){
        return MBEDTLS_ERR_SSL_WANT_WRITE;
    }else if(size < 0){
    	__ERROR("TCPSocket send error %d\r\n", size);
        return -1;
    }else{
        return size;
    }
}

void TLSSocket::print_mbedtls_error(const char *name, int err) {
    char buf[128];
    mbedtls_strerror(err, buf, sizeof (buf));
    __ERROR("%s() failed: -0x%04x (%d): %s\r\n", name, -err, err, buf);
}
