//#include "TCPSocket.h"

#ifndef SOURCE_TLSSOCKET_H_
#define SOURCE_TLSSOCKET_H_

#include "mbed.h"
#include "mbedtls/ssl.h"
#include "entropy.h"
#include "ctr_drbg.h"
#include "error.h"
#include "base64.h"
#include "sha512.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"

/*
 * todo SSL_CA_PEM should be stored in flash
 *
 * we should create some KeyManager to manage keys, certificates etc.
 *
 * this is certificate for Lets encrypt authority, it wont work with other
 * authorities or if deprecated
 */
const char SSL_CA_PEM[] = "-----BEGIN CERTIFICATE-----\n\
MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n\
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n\
DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n\
SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n\
GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n\
AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n\
q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n\
SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n\
Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n\
a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n\
/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n\
AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n\
CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n\
bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n\
c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n\
VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n\
ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n\
MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n\
Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n\
AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n\
uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n\
wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n\
X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n\
PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n\
KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n\
-----END CERTIFICATE-----\n";



/**
* \class TLSSocket
* \brief brief TO DO 
*
* Long description TO DO 
*/
class TLSSocket{
public:
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	TLSSocket(NetworkInterface *itf);

	~TLSSocket();

	/**
	* 
	* \param
	* \param
	* \return
	*/
	nsapi_error_t connect(const char *host, uint16_t port);

	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	nsapi_error_t close();

	/**
	* 
	* \param
	* \param
	* \return
	*/
	nsapi_size_or_error_t send(const void *data, nsapi_size_t size);

	/**
	* 
	* \param
	* \param
	* \return
	*/
	nsapi_size_or_error_t recv(void *data, nsapi_size_t size);

	/**
	* 
	* \param
	* \param
	* \return
	*/
	void set_blocking(bool blocking);

	/**
	* 
	* \param
	* \param
	* \return
	*/
	void set_timeout(int timeout);

	/**
	* 
	* \param
	* \param
	* \return
	*/
	nsapi_error_t bind(const char *address, uint16_t port);

	/**
	* 
	* \param
	* \param
	* \return
	*/
	nsapi_error_t bind(const SocketAddress &address);

protected:
	TCPSocket *_tcpsocket;
    mbedtls_entropy_context _entropy;		//624 B
    mbedtls_ctr_drbg_context _ctr_drbg;		//320 B
    mbedtls_x509_crt _cacert;				//208 B
    mbedtls_ssl_context _ssl;				//264 B
    mbedtls_ssl_config _ssl_conf;			//180 B => sum 1696

    /**
     * Receive callback for mbed TLS
     *
     * raw encrypted data from TCPSocket to TLS module
     */
    static int _ssl_recv(void *ctx, unsigned char *buf, size_t len);

    /**
     * Send callback for mbed TLS
     *
     * this defines how to send raw encrypted data to TCPSocket
     */
    static int _ssl_send(void *ctx, const unsigned char *buf, size_t len);

	/**
	* 
	* \param
	* \param
	* \return
	*/
    static void print_mbedtls_error(const char *name, int err);
};

#endif /* SOURCE_TLSSOCKET_H_ */
