#include "ByzanceLogger.h"
#include "ByzanceCore.h"
#include "IODA_general.h"
#include "macros_byzance.h"

#include "extmem_config.h"
#include "intmem_config.h"
#include "ByzanceSpi.h"

/*
 * MQTT callback functions
 */
#include "MQTTCommands.h"
#include "MQTTSettings.h"
#include "MQTTInfo.h"
#include "HttpServer.h"
#include "MqttBuffer.h"
#include "ByzanceClient.h"
#include "ByzanceWorker.h"

#include "error_codes.h"

#include "byzance_extern_declaration.h"

#include "Flash.h"
#include "ByzanceReporter.h"

#if BYZANCE_SUPPORTS_6LOWPAN
#include "LowpanBR.h"
#include "NanostackRfPhyAtmel.h"
#include "NanostackInterface.h"
#include "RFWrapper.h"
#endif

REGISTER_DEBUG(byzance);

Thread*					Byzance::_thread_byzance;

char*					Byzance::_macaddr;
ExtMem*					Byzance::_extmem;
IntMem*					Byzance::_intmem;
NetworkInterface*		Byzance::_itf;

char*					ByzanceCore::_fullID;
ByzanceWorker*			worker;

ByzanceSpi*				Byzance::_spi;

bool					Byzance::_byzance_disabled;

bool					Byzance::_firmware_changed;

// pokud zapnu link connection, tak se automaticky bude linková a clientová vrstva pokouset pripojit k internetu
bool					Byzance::_link_connection_enabled;
bool					Byzance::_link_connected;
bool					Byzance::_client_connected;

// zmena stavu pripojeni linkove vrstvy (ETH/WIFI)
Callback<void(bool)> 	Byzance::_link_connection_changed_callback;

// zmena stavu pripojeni MQTT vrstvy
Callback<void(bool)> 	Byzance::_client_connection_changed_callback;

// zmena verze knihovny
Callback<void(char*)> 	Byzance::_firmware_changed_callback;

Callback<void(void)>	Byzance::_restart_follows_callback;

Callback<void(bool)>	Byzance::_bin_busy_callback;

#if BYZANCE_SUPPORTS_6LOWPAN
RFWrapper *rf_phy;
LoWPANNDInterface mesh;
static mem_stat_t heap_info;
#endif

#define THREAD_SIGNAL_NEW_MSG	0x01

/**********************************************
 **********************************************
 **********************************************
 ***
 ***      INIT AND DEINIT
 ***
 ***********************************************
 ***********************************************
 **********************************************/

Byzance_Err_TypeDef Byzance::init(){

	uint32_t tmp32 = 0;

	int rc = 0;

	_link_connection_enabled	= false;
	_link_connected				= false;
	_client_connected			= false;

	ByzanceCore::hook_attach();
	ByzanceCore::ticker_init();

	ByzanceCore::attach_restart_follows(&_restart_follows);

	/*
 	 * INPUTS REGISTRATION
 	 */
	if (ByzanceCore::_analogInputs == NULL) {
		ByzanceCore::_analogInputs = new vector<ByzanceAnalogInputRegistration>;
	}
	if (ByzanceCore::_digitalInputs == NULL) {
		ByzanceCore::_digitalInputs = new vector<ByzanceDigitalInputRegistration>;
	}
	if (ByzanceCore::_messageInputs == NULL) {
		ByzanceCore::_messageInputs = new vector<ByzanceMessageInputRegistration>;
	}

	Thread::wait(500);

	// if module is inited for the fist time, return true
	// in that case start it and set some colour
	// in some special cases it could be inited from pre_init -> do nothing
    if(LedModule::init(LED_RED, LED_GREEN, LED_BLUE, 1)){
    	__WARNING("led inited for the first time in Byzance class\n");
    } else {
    	__WARNING("led inited earlier, not controlled by Byzance\n");
    }

	// Init spolecneho SPI.
	Byzance::_spi = new ByzanceSpi(MEM_MOSI, MEM_MISO, MEM_CLK);

	/*
	 * EXTMEM INIT
	 */
	external_spiflash spi_settings;

	spi_settings.speed	 		= 42000000;
	spi_settings.manID	 		= 0x01;
	spi_settings.memType		= 0x40;

	spi_settings.memCap 		= 0x17;
	spi_settings.size 			= SPIFLASH_SIZE;
	spi_settings.sectors 		= SPIFLASH_SECTORS;
	spi_settings.sector_size 	= SPIFLASH_SECTOR_SIZE;
	spi_settings.subsectors 	= SPIFLASH_SUBSECTORS;
	spi_settings.subsector_size = SPIFLASH_SUBSECTOR_SIZE;
	spi_settings.pages 			= SPIFLASH_PAGES;
	spi_settings.page_size 		= SPIFLASH_PAGE_SIZE;
	spi_settings.otp_bytes 		= SPIFLASH_OTP_BYTES;
	spi_settings.spi_csn		= MEM_CS;

	rc = Byzance::_extmem->init(&spi_settings, Byzance::_spi);
	if(rc==0){
		__TRACE("init: extmem init OK\n");
	} else {
		__ERROR("init: extmem init ERROR\n");
		ByzanceCore::catastropic_failure();
	}

	if(Byzance::_extmem==NULL){
		__TRACE("init: JE TO NULL\n");
	}

	/*
	 * extmem class for user
	 */
	Flash::init(_extmem);

	/*
	 * Init interni flash pameti
	 */
	Byzance::_intmem->init();

	/*
	 * Init configurator
	 */
	rc = Configurator::init(_extmem);
	if(rc){
		__ERROR("init: configurator init failed (code=%d)\n", rc);
		ByzanceCore::catastropic_failure();
	}

	/*
	 * Init mqtt manager
	 */
	rc = MqttManager::init(_extmem);
	if(rc){
		__ERROR("init: mqtt manager init failed (code=%d)\n", rc);
		ByzanceCore::catastropic_failure();
	}

	/*
	 * Init binary manager
	 */
	rc = BinManager::init(_extmem, _intmem);
	if(rc){
		__ERROR("init: binary manager init failed (code=%d)\n", rc);
		ByzanceCore::catastropic_failure();
	}

	BinManager::attach_state_changed(&_callback_binmgr_state_changed);

	/*
	 * FIX BUILD ID
	 */
	rc = BinManager::fix_build_id(get_byzance_build_id());
	if(rc>0) {

		__WARNING("init: Firmware has changed (PROBABLY PROGRAMMED VIA ST-LINK)\n");

	} else if (rc==0) {
		__WARNING("init: build id check OK\n");
	} else {
		ByzanceCore::catastropic_failure();
	}

#if BYZANCE_SUPPORTS_6LOWPAN
	/*
	 * LowpanBR - only if netsource is not lopwan and lowpanbr flag is set
	 */
	rc = Configurator::get_lowpanbr(&tmp32);
	if(rc){
		ByzanceCore::catastropic_failure();
	}

	if(tmp32){
		rc = Configurator::get_netsource(&tmp32);
		if(rc){
			ByzanceCore::catastropic_failure();
		}

		if(tmp32 != NETSOURCE_6LOWPAN){
			rf_phy = new RFWrapper();
			LowpanBR::init(&_itf,rf_phy);
		} else {
			__ERROR("invalid configuration - netsource is lowpan while lowpanbr flag set\n");
		}

	}

#endif

	/*
	 * GET FULL ID
	 */
	static char full_id[32];
	get_fullid(full_id);
	ByzanceCore::_fullID = full_id;

	/*
	 * LAUNCH flag reset and inform user about firmware change
	 */
    rc = Configurator::get_launched(&tmp32);
    if(rc){
    	ByzanceCore::catastropic_failure();
    }
	if(tmp32){
		rc = Configurator::set_launched(0);
		if(rc){
			ByzanceCore::catastropic_failure();
		}

		__WARNING("launch flag reset\n");

		_firmware_changed = true;

	} else {
		__WARNING("launch flag missing\n");
	}


	// byzance thread se musi pustit az jsou vsechny operace s extmem dokoncene, neznamo proc
	__TRACE("pousti se thread_byzance\n");

	// thread zatím sežral maximálně 5440 bytu, 5.12.2016
	// todo: pořešit priority vláken
	Byzance::_thread_byzance = new Thread(osPriorityNormal2, 8192,NULL,"Byzance");
	Byzance::_thread_byzance->start(&Byzance::_thread_byzance_function);

	/*
	 * tunnel ByzanceCore callback to Byzance class
	 */
	ByzanceCore::attach_bin_busy(&_bin_busy);

    return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::deinit(){

	// not implemented yet

	return BYZANCE_GENERAL_ERROR;
}

bool Byzance::set_disabled(){

	if(_byzance_disabled){
		return false;
	}

	_byzance_disabled = true;
	return true;
}

bool Byzance::is_disabled(){
	return _byzance_disabled;
}

Byzance_Err_TypeDef Byzance::get_version(char *val){

	char ver[32];
	BinManager::get_version(BIN_COMPONENT_FIRMWARE, ver);

	strcpy(val, ver);

	return BYZANCE_OK;

}

/**********************************************
 **********************************************
 **********************************************
 ***
 ***      CONFIGURATOR METHODS RELATED TO BOOTLOADER
 ***
 ***********************************************
 ***********************************************
 **********************************************/


Byzance_Err_TypeDef Byzance::get_flashflag(uint32_t *val){

	Configurator::get_flashflag(val);

	return BYZANCE_OK;

}

Byzance_Err_TypeDef Byzance::get_autobackup(uint32_t *val){

	Configurator::get_autobackup(val);

	return BYZANCE_OK;

}

Byzance_Err_TypeDef Byzance::get_blreport(uint32_t *val){

	Configurator::get_blreport(val);

	return BYZANCE_OK;

}

Byzance_Err_TypeDef Byzance::get_wdenable(uint32_t *val){

	Configurator::get_wdenable(val);

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_wdtime(uint32_t *val){

	Configurator::get_wdtime(val);

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_netsource(uint32_t *val){

	Configurator::get_netsource(val);

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_alias(char* val){

	Configurator::get_alias(val);

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_webview(uint32_t *val){

	Configurator::get_webview(val);

	return BYZANCE_OK;
}


Byzance_Err_TypeDef Byzance::get_webport(uint32_t *val){

	Configurator::get_webport(val);

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_timeoffset(int* val){

	Configurator::get_timeoffset(val);

	return 	BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_timesync(uint32_t *val){

	Configurator::get_timesync(val);

	return 	BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_lowpanbr(uint32_t *val){

	Configurator::get_lowpanbr(val);

	return 	BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_restartbl(uint32_t *val){

	Configurator::get_restartbl(val);

	return 	BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::set_restartbl(uint32_t val){

	Configurator::set_restartbl(val);

	return 	BYZANCE_OK;
}

/**********************************************
 **********************************************
 **********************************************
 ***
 ***      MISCELLANIOUS METHODS
 ***
 ***********************************************
 ***********************************************
 **********************************************/

Byzance_Err_TypeDef Byzance::get_revision(uint32_t* val){

	//get_revision();
	// todo implement

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_uptime(uint32_t* val){

	*val = ByzanceCore::uptime;

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_connected_time(uint32_t* val){

	*val = ByzanceCore::connected_time();

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_full_id(char* val){
	//return ByzanceCore::_fullID;

	return BYZANCE_OK;
}

Byzance_Err_TypeDef Byzance::get_supply_voltage(float* val){

	#if defined(TARGET_BYZANCE_IODAG3E)
		// PWR_MEAS
		AnalogIn	ain(PF_4);
		*val =  ain.read()*3.240*19;
	#else
		*val = 0;
	#endif

	return BYZANCE_OK;

}

Byzance_Err_TypeDef Byzance::get_vref_int(float* val){

	#if defined(TARGET_BYZANCE_IODAG3E)
		AnalogIn   vrefint(ADC_VREF);
		*val = 3.240 * vrefint.read();
	#else
		*val = 0;
	#endif

	return BYZANCE_OK;

}

Byzance_Err_TypeDef Byzance::get_core_temp(float* val){

	#if defined(TARGET_BYZANCE_IODAG3E)
		AnalogIn   tempint(ADC_TEMP);
		*val = (3.240 * tempint.read() - 0.76f) / 0.0025f + 25.0f;
	#else
		*val = 0;
	#endif

	return BYZANCE_OK;

}

void Byzance::led_module(bool state){

	// function can be called in pre_init
	// led module could be unitialised
	if(!LedModule::initialised()){
		LedModule::init(LED_RED, LED_GRN, LED_BLUE, 1);
	}

	ByzanceCore::led_module(state);

}

/**********************************************
 **********************************************
 **********************************************
 ***
 ***      BYZANCE IO METHODS
 ***
 ***********************************************
 ***********************************************
 **********************************************/

// implemented in core

/**********************************************
 **********************************************
 **********************************************
 ***
 ***      CONNECTION RELATED METHODS
 ***
 ***********************************************
 ***********************************************
 **********************************************/

/*
 * Byzance knihovna se připojí k Brokeru a pustí HTTP server
 */
Byzance_Err_TypeDef Byzance::connect() {

	uint32_t tmp32 = 0;
	int rc = 0;

	rc = Configurator::get_netsource(&tmp32);
	if(rc){
		ByzanceCore::catastropic_failure();
	}

	switch(tmp32){

		case NETSOURCE_DISABLED:
			__WARNING("network interface disabled\n");
			return BYZANCE_INVALID_INTERFACE;

#if BYZANCE_SUPPORTS_ETHERNET
		case NETSOURCE_ETHERNET:
			/*
			 * ETHERNET - CREATE INSTANCE
			 */
			if(_itf == NULL){
				_itf = new EthernetInterface();
				__DEBUG("ethernet interface created\n");
			} else {
				__DEBUG("ethernet interface already exists\n");
			}
			break;
#endif

#if BYZANCE_SUPPORTS_6LOWPAN
		case NETSOURCE_6LOWPAN:{
			//ns_hal_init(NULL, 15920,  NULL,&heap_info);	//inicializace RAM
			rf_phy = new RFWrapper();
			int16_t rc = mesh.initialize((NanostackRfPhyAtmel*)rf_phy);			//inicializace mesh api
			if(rc < 0){
				switch(rc){
				case NSAPI_ERROR_DEVICE_ERROR:
					__ERROR("lowpan mesh api error - probably hardware problem (WEXP not connected)\n");
					break;
				default:
					__ERROR("lowpan mesh api error - other error %d\n",rc);
				}
			}
			/*
			 * 6LOWPAN - CREATE MESH INSTANCE
			 */
			if(_itf == NULL){
				_itf = (NetworkInterface*)&mesh;
				__DEBUG("mesh interface created\n");
			} else {
				__DEBUG("mesh interface already exists\n");
			}
		}
			break;
#endif

#if BYZANCE_SUPPORTS_WIFI
		case NETSOURCE_WIFI:
			__ERROR("wifi not supported yet\n");
#endif

		default:

			__ERROR("unknown connection interface = %d\n", tmp32);

			return BYZANCE_LINKED_DOWN;
	}

	if(tmp32){
		// inicilizace clienta
		ByzanceClient::init(Byzance::_extmem, ByzanceCore::_fullID);
		ByzanceClient::set_netsource((Netsource_t)tmp32);

		// pokud client zahodí nějakou zprávu, vyvolá se tento callback
		ByzanceClient::attach_msg_thrown(&_client_msg_thrown);
		// nova zprava v clientovi
		ByzanceClient::attach_msg_rcvd(&_client_msg_rcvd);
		// pokud se klient odpoji nebo pripoji, vyvola se tento callback
		ByzanceClient::attach_connection_change(&_client_connection_change);

		// zapne pripojovaci flag
		// samotne pripojeni probehne v Byzance threadu
		_link_connection_enabled = 1;
	}

	rc = Configurator::get_webview(&tmp32);
	if(rc){
		ByzanceCore::catastropic_failure();
	}
	if(tmp32){
		__LOG("webview enabled, launching now\n");
		// inicializace HTTP serveru
		HttpServer::init(Byzance::_extmem);
	} else {
		__LOG("webview disabled\n");
	}

    return BYZANCE_OK;
}

/*
 * Byzance knihovna se odpojí od Brokeru a zastaví HTTP server
 */
Byzance_Err_TypeDef Byzance::disconnect() {
	ByzanceClient::disconnect();
	HttpServer::stop();
    return BYZANCE_OK;
}

NetworkInterface** Byzance::get_itf(){

	return &_itf;

}

const char* Byzance::get_ip_address(){
	if(_link_connected){
		return _itf->get_ip_address();
	}
	return "0.0.0.0";
}

const char* Byzance::get_mac_address(){
	if(_link_connected){
		return _itf->get_mac_address();
	}
	return "00:00:00:00:00:00";
}

void Byzance::attach_link_connection_changed(void (*function)(bool)) {
	Byzance::_link_connection_changed_callback = callback(function);
}

void Byzance::attach_client_connection_changed(void (*function)(bool)) {
	Byzance::_client_connection_changed_callback = callback(function);
}

void Byzance::attach_firmware_changed(void (*function)(char*)) {
	Byzance::_firmware_changed_callback = callback(function);
}

/**********************************************
 **********************************************
 **********************************************
 ***
 ***      RESTART RELATED METHODS
 ***
 ***********************************************
 ***********************************************
 **********************************************/

void Byzance::restart(time_t sec){

}

void Byzance::restart_postpone(time_t sec) {
	__DEBUG("request to postpone restart for %d sec\n", sec);
	ByzanceCore::restart(sec);
}

time_t Byzance::restart_pending() {
	__DEBUG("restart_pending returns %d sec\n", ByzanceCore::restart_pending);
	return ByzanceCore::restart_pending;
}

void Byzance::attach_restart_follows(void (*function)(void)) {
	_restart_follows_callback = callback(function);
}

void Byzance::attach_bin_busy(void (*function)(bool)) {
	_bin_busy_callback = callback(function);
}

void Byzance::_thread_byzance_function() {

	static uint32_t			buffer_filled;
	static message_struct	*msg;
	static uint8_t			cntr;
	int rc = 0;
	uint32_t tmp32 = 0;

	__DEBUG("thread has been launched\n");

	/*
	 * Call firmware changed user callback
	 */
	if(_firmware_changed){

		__WARNING("firmware changed callback\n");

		char ver[16];

		rc = Byzance::get_version(ver);
		if(!rc){

			__WARNING("calling firmware changed callback\n");

			if(_firmware_changed_callback){
				_firmware_changed_callback.call(ver);
			}
		}

		_firmware_changed = 0;
	}

	while(true) {

		/*
		 * Handle some processes like
		 * - automatic backup create
		 */

		BinManager::yield();

		// when autobackup is disabled during backup process,
		// backup process is stopped
		rc = Configurator::get_autobackup(&tmp32);
		if(rc){
			ByzanceCore::catastropic_failure();
		}

		if((!tmp32) && BinManager::is_busy(BIN_COMPONENT_BACKUP)){
			BinManager::backup_stop();
			__WARNING("backup process has been stopped because autobackup has been disabled\n");
		}


		/*
		 * Connect procedura
		 */
		if(_link_connection_enabled){
			if(!_link_connected){

				__WARNING("Ioda is disconnected. Connecting...\n");
				Watchdog::instance()->service();		// Feed the dog.

				/**************************************
				 *
				 * CONNECT INTERFACE
				 *
				 **************************************/

				__INFO("using interface: ");

				rc = Configurator::get_netsource(&tmp32);
				if(rc){
					ByzanceCore::catastropic_failure();
				}

				switch(tmp32){
				case NETSOURCE_ETHERNET:
					__INFO("ethernet\n");
					break;
				case NETSOURCE_WIFI:
					__INFO("wifi\n");
					break;
				case NETSOURCE_6LOWPAN:
					__INFO("6lowpan\n");
					break;
				default:
					__ERROR("invalid netsource\n");
				}

				/*
				 * ETHERNET - CONNECT
				 */
				__DEBUG("interface connecting\n");
				rc = _itf->connect();
				__DEBUG("connect passed\n");
				//wait_ms(100);
				if(!rc){
					__WARNING("interface is connected\n");
					_link_connected = 1;
					__LOG("interface ip = %s\n", _itf->get_ip_address());
					__INFO("interface mac = %s\n", _itf->get_mac_address());

#if BYZANCE_SUPPORTS_6LOWPAN
					//setting up dns client for lowpan
					if(tmp32 == NETSOURCE_6LOWPAN){
						//get ipv6 border router address
						char d[64];
						if(mesh.getRouterIpAddress(d,sizeof(d))){
							SocketAddress dns;
							dns.set_ip_address(d);
							_itf->add_dns_server(dns);	//add it as DNS server
						}
					}
#endif

					// pointery na ethernet se predaji poddruzenym sluzbam
					// sluzby se v dalsim kroku nastartuji
					ByzanceClient::init_itf(Byzance::_itf);
					rc = Configurator::get_webview(&tmp32);
					if(rc){
						ByzanceCore::catastropic_failure();
					}
					if(tmp32){
						// inicializace HTTP serveru
						HttpServer::init_itf(Byzance::_itf);
					}

				} else {
					__WARNING("interface connect failed (err=%d)\n", rc);
				}

				/*
				 * pripojilo se to, inicializujeme zbytek
				 */
				if(_link_connected){
					worker = ByzanceWorker::init();
					ByzanceWorker::init_itf(_itf);

					Watchdog::instance()->service();		// Feed the dog.

					__TRACE("loading mqtt connection informations\n");

					mqtt_connection		connection;
					mqtt_credentials	credentials;
					MqttManager::get_connection(MQTT_NORMAL, &connection);
					MqttManager::get_credentials(&credentials);

					__TRACE("connecting to NORMAL mqtt broker\n");
					ByzanceClient::connect(&connection, &credentials);
					rc = Configurator::get_webview(&tmp32);
					if(rc){
						ByzanceCore::catastropic_failure();
					}
					if(tmp32){
						// inicializace HTTP serveru
						HttpServer::run();
					}

					Watchdog::instance()->service();		// Feed the dog.

					/*
					 * CALLBACK FOR USER
					 */
					if(_link_connection_changed_callback){
						_link_connection_changed_callback.call(true);
					}

				// link not connected
				} else {

				}
			}
		}

		// Feed the dog.
		Watchdog::instance()->service();

		/*******
		 *
		 *
		 *
		 * sem se program dostane pouze pokud je linková vrstva spojená
		 *
		 *
		 *
		 */
		if(!_link_connected){
			__TRACE("link is not connected yet\n");
			Thread::wait(100);
			continue;
		}

		/*
		 * Předávání zpráv mezi Senderem a Clientem
		 */

		if(ByzanceClient::is_connected()){

			/*
			0 -> wait for any signal
			1 -> wait 1ms
			*/
			osEvent evt = Thread::signal_wait(0, 20);
			if ((evt.value.signals & THREAD_SIGNAL_NEW_MSG) && evt.status == osEventSignal){

				__TRACE("-signal rcvd\n");

				rc = 0;

				do{
					msg = ByzanceClient::get_message();
					if(msg != NULL){
						/*
						 * Tady se zpracovává zpráva určená Yodovi
						 * prefix topic fullid je useknuty
						 */

						// fixme bez wait + debug hlasky to chcipa pri prenosu binark z hpomera na yodu
						__TRACE("pred workerem\n");
						//BYZANCE_TRACE("topic=%s\n", msg.topic.c_str());

						worker->process(msg);
						ByzanceClient::delete_incomming_msg(msg);
						__TRACE("po workeru\n");
					}
				} while(msg != NULL);

			}

		} else {
			// disconnected -> nothing to do
			Thread::wait(100);
		}

	}
}

/*
 * Klient z nějakého důvodu zahodil zprávu
 */
void Byzance::_client_msg_thrown(uint32_t code){
	__ERROR("client thrown away incomming message for some reason (code=%d)\n", code);
}

/*
 * Klient přijal novou zprávu
 */
void Byzance::_client_msg_rcvd(void){
	__DEBUG("callback: client new msg\n");
	_thread_byzance->signal_set(THREAD_SIGNAL_NEW_MSG);
}

/*
 * Zmena pripojeni v MQTT clientu
 */
void Byzance::_client_connection_change(bool connected){

	if(connected){

		/*
		 * REPORT TO HOMER THAT CLIENT IS READY
		 */
		__WARNING("client connection change - NOW CONNECTED\n");

		ByzanceReporter::report(REPORT_EVENT_WELCOME);

		ByzanceCore::connected_start();

	} else {
		__WARNING("client connection change - NOW DISCONNECTED\n");

		// no need to report disconnect event because there is no possibility to deliver it

		ByzanceCore::connected_stop();

		if(BinManager::upload_running()){
			__WARNING("upload was running -> stopped\n");
			BinManager::upload_stop();
		}
	}

	/*
	 * CALLBACK FOR USER
	 */
	if(_client_connection_changed_callback){
		_client_connection_changed_callback.call(connected);
	}

	// change internal variable
	_client_connected = connected;

}

void Byzance::_callback_binmgr_state_changed(bin_component_t comp, bin_state_t state){

	__DEBUG("binmgr state changed callback - comp=%d, state=%d\n", comp, state);

	ByzanceReporter::report_state(comp, state);

}

uint32_t Byzance::get_stack_size(void) {
	return _thread_byzance->stack_size();
}

uint32_t Byzance::get_free_stack(void) {
	return _thread_byzance->free_stack();
}

uint32_t Byzance::get_used_stack(void) {
	return _thread_byzance->used_stack();
}

uint32_t Byzance::get_max_stack(void) {
	return _thread_byzance->max_stack();
}

Thread::State Byzance::get_state(void){
	return _thread_byzance->get_state();
}

osPriority Byzance::get_priority(void){
	return _thread_byzance->get_priority();
}

bool Byzance::get_connected_link(void){
	return _link_connected;
}

// internal function for this class which calls user callback for higher layer
void Byzance::_restart_follows(void) {

	// if there is some Byzance procedure in progress, postpone restart
	if(BinManager::is_busy()){

		__INFO("_restart_follows: some byzance procedure in progress, postpone restart\n");
		ByzanceCore::restart(1);

	// byzance is ready to restart, notify user
	} else {
		__INFO("_restart_follows: calling user callback\n");
		if(_restart_follows_callback){
			_restart_follows_callback.call();
		}
	}
}

void Byzance::_bin_busy(bool busy){

	__DEBUG("****** BIN_BUSY CALLED *********\n");

	if(_bin_busy_callback){
		_bin_busy_callback.call(busy);
	}
}
