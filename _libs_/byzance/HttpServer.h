#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "mbed.h"
#include "HTMLParser.h"
#include "IODA_general.h"
#include "byzance_debug.h"
#include "MQTTClient.h"
#include "extmem_config.h"
#include "byzance_config.h"
#include "ByzanceClient.h"
#include "struct_binary.h"
#include "struct_mqtt.h"
#include "struct_wifi_credentials.h"
#include "enum_http.h"

typedef enum : unsigned char {
	HTTP_METHOD_UNKNOWN,
	HTTP_METHOD_GET,
	HTTP_METHOD_POST
} HttpMethod_t;

typedef enum : unsigned char {
	HTTP_TYPE_UNKNOWN,
	HTTP_TYPE_APPLICATION_X_WWW_FORM_URLENCODED,
} HttpType_t;

/**
* \struct struct_http_request
* \brief brief description TO DO 
*
* Description TO DO 
*/
struct struct_http_request {
	HttpMethod_t	method;
	char 			url[32];
	size_t			length;
	HttpType_t		type;
};

/**
* \class HttpServer
* \brief brief description TO DO 
*
* Description TO DO 
*/
class HttpServer {

public:

    /** init
     *
     * TODO add long description
     *
     * \param extmem TODO doplnit
     */
	static void init(ExtMem* extmem);

	 /** init_eth
      *
      * TODO add long description
      *
      * \param eth TODO doplnit
      */
	static void init_itf(NetworkInterface* itf);

	 /** If requested page match pattern, callback is called
     *
     * TODO add long description
     *
     * \param TODO doplnit
     */
	static void on(const char* url, void (*function)(void));

	 /** TODO
	 *
	 * TODO add long description
	 *
	 * \param TODO doplnit
	 */
	static void on_not_found(const char* url, void (*function)(void));

    /** run
     *
     * TODO add long description
     *
     */
	static void run();

	 /** stop
      *
      * TODO add long description
      *
      */
	static void stop();


    /** get_stack_size
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static uint32_t get_stack_size(void);

	/** get_free_stack
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static uint32_t get_free_stack(void);

	/** get_used_stack
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static uint32_t get_used_stack(void);

	/** get_max_stack
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static uint32_t get_max_stack(void);

	/** get_state
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static Thread::State get_state(void);

	/** get_priority
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static osPriority get_priority(void);
   
protected:

	/**
	 * Pointers
	 */
	static NetworkInterface*	_itf;

	static TCPSocket*			_http_socket;
	static TCPServer*			_http_server;

	static ExtMem*				_extmem;

	static char*				_buffer;

	/*
	 * Thread pointers
	 */
	static Thread*				_thread_http;

	/** _thread_http_function
	 * 
	 */
	static void _thread_http_function();

	/** _find_label
	 * 
	 * \param
	 * \param
	 * \param
	 */
	static bool _find_label(string& request, const char* label, string& result);

	/** _create_header
	 * 
	 * \param
	 * \param
	 * \param
	 */
	static void _create_header(HttpError_t code, string& header, size_t len);

	/** _send_response
	 * 
	 * \param
	 */
	static void _send_response(string& data);

	/** _print_request
	 * 
	 * \param
	 */
	static void _print_request(struct_http_request* r);

	/** _state_changed
	 * 
	 */
	static void _state_changed(void);

	//static void _state_changed_sock(void);

private:
    
    HttpServer() {};
    ~HttpServer() {};
};

#endif  /* HTTP_SERVER_H */
