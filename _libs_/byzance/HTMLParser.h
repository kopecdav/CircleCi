#ifndef HTML_PARSER
#define HTML_PARSER

#include "mbed.h"
#include "IODA_general.h"
#include "HttpServer.h"
#include "enum_http.h"
#include "LowpanBR.h"
#include "MqttManager.h"
#include "build_datetime.h"

/*
 * Co všechno se bude zobrazovat v HTML stránce
 */
#define	HTML_SHOW_THREAD_BYZANCE	1
#define	HTML_SHOW_THREAD_CLIENT		1
#define	HTML_SHOW_THREAD_HTTP		1

#if BYZANCE_SUPPORTS_WIFI
#define	HTML_SHOW_THREAD_WIFI		0
#endif

#if BYZANCE_SUPPORTS_6LOWPAN
#define	HTML_SHOW_THREAD_LOWPANBR	1
#endif

/*
 * HANDLED HTML PAGES
 */
#define HTML_PAGE_HOME				(const char*)"/"
#define HTML_PAGE_BINMANAGER		(const char*)"/binmanager"
#define HTML_PAGE_THREADS			(const char*)"/threads"
#define HTML_PAGE_FAVICON			(const char*)"/favicon"
//#define HTML_PAGE_INFO				(const char*)"/info"
#define HTML_PAGE_WIFI				(const char*)"/wifi"
#define HTML_PAGE_DEVICES			(const char*)"/devices"
#define HTML_PAGE_INPUTS			(const char*)"/inputs"
#define HTML_PAGE_MQTT				(const char*)"/mqtt"
#define HTML_PAGE_CONFIGURATOR		(const char*)"/configurator"

/*
 * FORM handling
 */
#define HTML_FORM_NORMAL_BROKER		(const char*)"/settings-normal-broker"
#define HTML_FORM_BACKUP_BROKER		(const char*)"/settings-backup-broker"
#define HTML_FORM_WIFI				(const char*)"/settings-wifi"
#define HTML_FORM_OTHER				(const char*)"/settings-other"
#define HTML_FORM_RESTART			(const char*)"/settings-restart"


/**
* \class HTMLParser
* \brief brief TO DO
*
* Description
*/
class HTMLParser {

public:

    /** init
     *
     * TODO add long description
     *
     * \param extmem TODO doplnit
     */
	static void init(ExtMem* extmem);

	/** 
     *
     * TODO add long description
     *
     * \return TODO doplnit
     */
	static HttpError_t parse(char* request, string& response);

	static void show_binary(bin_component_t comp, string& buffer);
   
protected:

	static char*				_buffer;
	static ExtMem*				_extmem;

private:
    
    HTMLParser() {};
    ~HTMLParser() {};
};

#endif  /* HTML_PARSER */
