#include "mbed.h"
#include "ByzanceSpi.h"
#include "ExtMem.h"
#include "IntMem.h"
#include "extmem_config.h"
#include "intmem_config.h"
#include "build_datetime.h"
#include "support_functions.h"
#include "IODA_bootloader_version.h"
#include "Watchdog.h"
#include "struct_binary.h"
#include "struct_config_ioda.h"
#include "support_functions.h"

#include "ByzanceLogger.h"
#include "BootloaderCommander.h"
#include "macros_bootloader.h"

#include "Configurator.h"

// LED module with common cathode -> inverted
#define BOOTLOADER_LED_ON 	0
#define BOOTLOADER_LED_OFF 	1

#include "USBSerial.h"
USBSerial usb(0x1f01, 0x2013, 0x0002, false);

Serial *dbg;

uint32_t tmp32 = 0;
char buff[64];

typedef enum : unsigned char {
	BOOTLOADER_SOURCE_SERIAL,
	BOOTLOADER_SOURCE_USB
} BootloaderSource_t;

// standard things like bootloader header, informations etc; plaintext for humans
#define BOOTLOADER_BLREPORT(...);		bootloader_print(__VA_ARGS__);

/*
// responses to commands. Support of CRC; for machine processing or for humans
#define BOOTLOADER_RESPONSE(...);		bootloader_response(__VA_ARGS__);
*/

#define BOOTLOADER_MODE_COMMANDS_DELAY_MS	1000
#define BOOTLOADER_MODE_FACTORY_DELAY_MS	10000

#define USB_CONFIGURE_TIMEOUT	500 //ms

typedef enum {
	BOOTLOADER_MODE_JUMP,
	BOOTLOADER_MODE_FLASH,
	BOOTLOADER_MODE_RESTORE,
	BOOTLOADER_MODE_COMMANDS,
	BOOTLOADER_MODE_FACTORY,
	BOOTLOADER_MODE_UNKNOWN
} BootloaderMode_TypeDef;

/*
 * FUNCTION PROTOTYPES
 */
typedef  void (*pFunction)(void);
bool crc_ok();
void led_blink(void);
void bootloader_app();
void bootloader_end();
void command_mode();
void bootloader_print(const char* format, ...);
void catastrophic_failure();

// RGB module
// note: LEDS are inverted because of common cathode
DigitalOut ledRed(LED_RED);
DigitalOut ledGrn(LED_GREEN);
DigitalOut ledBlu(LED_BLUE);

DigitalIn usrBtn(USER_BUTTON);

// pointer na objekt externi pameti
ExtMem *extmem;
IntMem *intmem;

BootloaderMode_TypeDef mode;

#define MAXIMUM_COMMAND_SIZE 128

char cmd[MAXIMUM_COMMAND_SIZE];
uint8_t cmd_cnt;

volatile char				data_flag = 0;

BootloaderCommander commander;

bool crc_required = false;

Timer autojump;

char resp[1024];

bool blink_on;

/*
 * serial RX interrupt callback
 */
void received_char() {
	static char c = 0;
	c = dbg->getc();

	if(c=='\n'){

		// direct response msg to serial
		ByzanceLogger::init(dbg);

		// add termination character
		cmd[cmd_cnt] = 0x00;

		if(crc_ok()) {
			// notify about new message
			data_flag = 1;
		} else {
			// throw msg away if crc is not correct
			cmd[0] = 0x00;
		}

		// reset counter
		cmd_cnt = 0;

	} else if(c=='\r') {
		// ignoruje \r

	// buffer overflow -> clean buffer and begin again
	} else if(cmd_cnt>=MAXIMUM_COMMAND_SIZE) {
		cmd[0] = 0x00;
		cmd_cnt = 0;

	} else {
		cmd[cmd_cnt] = c;
		cmd_cnt++;
	}

}

void received_msg(){
	static char c = 0;
	while(usb.available()){
		c = usb.getc();

		if(c=='\n'){

			// direct response msg to usb
			ByzanceLogger::init(&usb);

			// add termination character
			cmd[cmd_cnt] = 0x00;

			if(crc_ok()) {
				// notify about new message
				data_flag = 1;
			} else {
				// throw msg away if crc is not correct
				cmd[0] = 0x00;
			}

			// reset counter
			cmd_cnt = 0;

		} else if(c=='\r') {

			// ignoruje \r

		// buffer overflow -> clean buffer and begin again
		} else if(cmd_cnt>=MAXIMUM_COMMAND_SIZE) {
			cmd[0] = 0x00;
			cmd_cnt = 0;

		} else {
			cmd[cmd_cnt] = c;
			cmd_cnt++;
		}
	}
}

bool crc_ok(){

	char* pch;

	// find last #
	pch=strrchr(cmd,'#');

	/*
	 * CRC is not required
	 */
	if(pch==NULL){

		// necessary for response
		crc_required = false;

		return true;

	}

	/*
	 * CRC is required
	 */
	// necessary for response
	crc_required = true;

	char crc[8];

	// copy from pos+1 (after hashtag) to the end
	// this is supposed to be a CRC
	strcpy(crc, pch+1);

	// cut off crc from the end of the message
	pch[0] = 0x00;

	uint8_t crc_calculated = 0;
	crc_calculated = calc_crc(cmd);

	uint8_t crc_received = 0;
	crc_received = strtoul(crc, NULL, 16);

	if(crc_calculated!=crc_received){
		return false;
	}

	return true;

}

bool welcome = false;

void settings(int baud, int bits, int parity, int stop){

	// wait to reconfigure with new settings or timeout
	Timer t;
	t.start();

	while(!usb.configured()){
		wait_ms(100);
		if(t.read_ms()>USB_CONFIGURE_TIMEOUT){
			//data_source = BOOTLOADER_SOURCE_SERIAL;
			break;
		}
	}

	//data_source = BOOTLOADER_SOURCE_USB;

	welcome = true;

}

int main() {

	ledRed	= BOOTLOADER_LED_ON;
	ledGrn	= BOOTLOADER_LED_ON;
	ledBlu	= BOOTLOADER_LED_OFF;

	Timer t;
	t.start();

	autojump.start();

	// wait for USB to configure
	while(!usb.configured()){
		wait_ms(100);
		if(t.read_ms()>USB_CONFIGURE_TIMEOUT) break;
	}

	usb.attach(&settings);

	// změnit pak na serialTX, RX
	dbg = new Serial(Y00, Y01);
	dbg->baud(115200);
	ByzanceLogger::init(&usb);
	ByzanceLogger::set_level(DEBUG_LEVEL_TRACE);
	ByzanceLogger::enable_prefix(false);

	char rc = 0;

	ByzanceSpi * _spi = new ByzanceSpi(MEM_MOSI, MEM_MISO, MEM_CLK);

	/*
	 * EXTMEM INIT
	 */
	external_spiflash spi_settings;
	/*
	SPI 1 - 42 MHz
	SPI 2 - 21 MHz -> extmem and NRF905 are connected to SPI2
	SPI 3 - 21 MHz

	Max frequency from datasheet
	extmem - 108 MHz
	nrf905 - 10 MHz

	For IODAG3E its limited to 42MHz
	*/
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


	rc = extmem->init(&spi_settings, _spi);
	if(rc){
		BOOTLOADER_BLREPORT("**** Extmem init FAIL *****\n");
		catastrophic_failure();
	}

	rc = extmem->check_ok();
	if(rc){
		BOOTLOADER_BLREPORT("**** Extmem check FAIL *****\n");
		catastrophic_failure();
	}

    /*
     * Init intmem
     */
	intmem->init();

    /*
     * Init configurator
     */
	Configurator::init(extmem);

    /*
     * Init binary manager
     * must be used after config is loaded because of printf blreport flag
     */
    // attach led blink must be used before init becuase there is set_defaults in init
    BinManager::attach_led_blink(&led_blink);
    BinManager::init(extmem, intmem);

    commander.attach_led_blink(&led_blink);
    commander.init(extmem, intmem);

    MqttManager::init(extmem);

    bootloader_app();

}

void bootloader_app(){

	int rc = 0;

	/*
	 * BOOTLOADER START
	 */
	BOOTLOADER_BLREPORT("\n");
	BOOTLOADER_BLREPORT("***************************************************************\n");
	BOOTLOADER_BLREPORT("*\n");
	BOOTLOADER_BLREPORT("* ########  ##    ## ########    ###    ##    ##  ######  ########\n");
	BOOTLOADER_BLREPORT("* ##     ##  ##  ##       ##    ## ##   ###   ## ##    ## ##      \n");
	BOOTLOADER_BLREPORT("* ##     ##   ####       ##    ##   ##  ####  ## ##       ##      \n");
	BOOTLOADER_BLREPORT("* ########     ##       ##    ##     ## ## ## ## ##       ######  \n");
	BOOTLOADER_BLREPORT("* ##     ##    ##      ##     ######### ##  #### ##       ##      \n");
	BOOTLOADER_BLREPORT("* ##     ##    ##     ##      ##     ## ##   ### ##    ## ##      \n");
	BOOTLOADER_BLREPORT("* ########     ##    ######## ##     ## ##    ##  ######  ########\n");
	BOOTLOADER_BLREPORT("* \n");

    /*
	 * CHECK IF BOOTLOADER HAS CHANGED
	 */
    rc = BinManager::fix_bootloader(BOOTLOADER_VERSION);
    if(rc==BINMGR_FIXED){
    	BOOTLOADER_BLREPORT("* Bootloader has changed (PROBABLY PROGRAMMED VIA ST-LINK)\n");
    }

	BOOTLOADER_BLREPORT("* Target:	%s\n", TOSTRING_TARGET(__BUILD_TARGET__));
	BOOTLOADER_BLREPORT("* Version: %s\n", BOOTLOADER_VERSION);
	BOOTLOADER_BLREPORT("* Compiled on %02d.%02d. %04d - %02d:%02d:%02d\n", __BUILD_DAY__, __BUILD_MONTH__, __BUILD_YEAR_LEN4__, __BUILD_HOUR__, __BUILD_MINUTE__, __BUILD_SECOND__);
	char full_id[32];
	get_fullid(full_id);
	BOOTLOADER_BLREPORT("* FULL ID is %s\n", full_id);
	char mac_addr[64];
	get_mac_address(mac_addr);
	BOOTLOADER_BLREPORT("* MAC address is %s\n", mac_addr);

	Configurator::get_alias(buff);
	BOOTLOADER_BLREPORT("* Alias is %s\n", buff);

	/*
	 * Measure how long is the user button pressed
	 */
	uint32_t presstime = 0;
	uint32_t start = HAL_GetTick();

	while(usrBtn){
		presstime = HAL_GetTick()-start;

		// FACTORY RESET MODE
		if(presstime>BOOTLOADER_MODE_FACTORY_DELAY_MS){

			// todo solve other leds

			ledRed = !ledRed;
			wait_ms(10);

			// overwrite previous mode if there is some
			mode = BOOTLOADER_MODE_FACTORY;

			break;

		// COMMANDS MODE
		} else if (presstime>BOOTLOADER_MODE_COMMANDS_DELAY_MS){

			ledRed	= BOOTLOADER_LED_OFF;
			ledGrn	= BOOTLOADER_LED_OFF;
			ledBlu 	= BOOTLOADER_LED_ON;

			if(!mode){
				mode = BOOTLOADER_MODE_COMMANDS;
			}

		}
	}

	/*
	 * AUTO BACKUP
	 */
	Configurator::get_autobackup(&tmp32);
	if(tmp32) {
		BOOTLOADER_BLREPORT("* Autobackup: \tON\n");
	} else {
		BOOTLOADER_BLREPORT("* Autobackup: \tOFF\n");
	}

	/*
	 * RESTARTBL
	 */
	Configurator::get_restartbl(&tmp32);
	if(tmp32) {
		BOOTLOADER_BLREPORT("* Restart BL: \tON\n");
		if(!mode){
			mode=BOOTLOADER_MODE_COMMANDS;
			rc = Configurator::set_restartbl(0);
			if(rc){
				BOOTLOADER_BLREPORT("* Ioda config save: \terror #%d\n", rc);
				while(1);
			}
		}
	} else {
		BOOTLOADER_BLREPORT("* Restart BL: \tOFF\n");
	}

	/*
	 * PREVIOUS LAUNCH CHECK
	 */
	Configurator::get_launched(&tmp32);
	if(tmp32) {
		BOOTLOADER_BLREPORT("* Prev. launch: \tERROR (%d)\n", tmp32);

		// if binary launch failed, attemp to to run restore in all cases
		// in restore process is checked validity of backup file
		if(!mode){
			mode=BOOTLOADER_MODE_RESTORE;
		}

	} else {
		BOOTLOADER_BLREPORT("* Prev. launch: \tOK\n");
	}

	/*
	 * CONFIGURED CHECK
	 */
	Configurator::get_configured(&tmp32);
	if(tmp32) {
		BOOTLOADER_BLREPORT("* Configured: \tTRUE\n");
	} else {
		BOOTLOADER_BLREPORT("* Configured: \tFALSE\n");
		if(!mode){
			mode=BOOTLOADER_MODE_COMMANDS;
		}
	}

	/*
	 * FLASH FLAG CHECK
	 */
	Configurator::get_flashflag(&tmp32);
	if(tmp32) {
		BOOTLOADER_BLREPORT("* Flash flag: \tPRESENT\n");
		if(!mode){
			mode = BOOTLOADER_MODE_FLASH;
		}
	} else	{
		BOOTLOADER_BLREPORT("* Flash flag: \tMISSING\n");
	}

    /*
     * If there is main app, on the begin of INTPART_FIRMWARE part there is value 0x20020000
     */
	uint32_t app_data = *(uint32_t *)(INTPART_FIRMWARE);
    if(app_data == 0xFFFFFFFF){
    	BOOTLOADER_BLREPORT("* Main APP: \tMISSING\n");
    	if(!mode){
    		mode = BOOTLOADER_MODE_COMMANDS;
    	}
    } else {
    	BOOTLOADER_BLREPORT("* Main APP: \tPRESENT\n");
    }

	/*
	 * DEFAULT MODE
	 */
	if(!mode){
		mode = BOOTLOADER_MODE_JUMP;
	}

	/*
	 * SHOW MODE
	 */
	switch(mode){
	case BOOTLOADER_MODE_JUMP:
		BOOTLOADER_BLREPORT("* Mode: \t\tJUMP\n");
		break;
	case BOOTLOADER_MODE_FLASH:
		BOOTLOADER_BLREPORT("* Mode: \t\tFLASH\n");
		break;
	case BOOTLOADER_MODE_RESTORE:
		BOOTLOADER_BLREPORT("* Mode: \t\tRESTORE\n");
		break;
	case BOOTLOADER_MODE_COMMANDS:
		BOOTLOADER_BLREPORT("* Mode: \t\tCOMMANDS\n");
		break;
	case BOOTLOADER_MODE_FACTORY:
		BOOTLOADER_BLREPORT("* Mode: \t\tFACTORY\n");
		break;
	default:
		BOOTLOADER_BLREPORT("* Mode: \t\tERROR\n");
		if(!mode){
			mode=BOOTLOADER_MODE_COMMANDS;
		}
	}

	/*
	 * RESOLVE MODE
	 */
	if(mode==BOOTLOADER_MODE_FLASH){

		/*
		 * Autobackup is enabled?
		 * In that case run backup procedure before flashing new firmware
		 */
		rc = Configurator::get_autobackup(&tmp32);
		if(rc==0 && tmp32){
			rc = BinManager::backup_start(true);
			if (rc == BINMGR_BACKUP_STARTED){
				do{
					rc = BinManager::yield();
				} while(rc);
			}
			BOOTLOADER_BLREPORT("* Autobackup \tDONE\n");
		}

		// flash process
		BinManager::flash_firmware(MODE_FLASH);

		// todo: solve rc
		rc = 0;
		Configurator::set_flashflag(0, false);
		Configurator::set_launched(1, true);
		if(rc){
			BOOTLOADER_BLREPORT("* Ioda config save: \terror #%d\n", rc);
			 while(1);
		} else {
			BOOTLOADER_BLREPORT("* Flash flag:\tERASED\n");
			BOOTLOADER_BLREPORT("* Flashing \tDONE\n");
			BOOTLOADER_BLREPORT("* Flag 'launched' set\n");
		}

	} else if (mode==BOOTLOADER_MODE_RESTORE){
		// restore process
		rc = BinManager::flash_firmware(MODE_RESTORE);

		BOOTLOADER_BLREPORT("* Restore \t");
		if(rc){
			BOOTLOADER_BLREPORT("ERROR\n");
		} else {
			BOOTLOADER_BLREPORT("DONE\n");
		}

	} else if (mode==BOOTLOADER_MODE_COMMANDS){

		command_mode();

	} else if (mode==BOOTLOADER_MODE_FACTORY){

		ledRed = BOOTLOADER_LED_ON;
		ledGrn = BOOTLOADER_LED_ON;
		ledBlu = BOOTLOADER_LED_OFF;

		if(commander.parse("defaults", resp)){
			// remove all cr/lf characters
			trim_crlf(resp);

			// send response
			ByzanceLogger::log(resp);

			ByzanceLogger::log("\n");
		}

		command_mode();

	}

	bootloader_end();

	// sem by se to nikdy nemělo dostat ;)
	while(1);
}

void bootloader_end(){

	// second MAIN APP check -> user could destroy main app with his commands in command mode
	uint32_t app_data = *(uint32_t *)(INTPART_FIRMWARE);
    if(app_data == 0xFFFFFFFF){
    	BOOTLOADER_BLREPORT("* Main APP: \tERROR\n");
    	command_mode();
    }

	/*
	 * Watchdog enable
	 */
    // must check for wd_times bigger than 0,
    // otherwise bootloader will fail because of diving by 0 while configuring wd
    uint32_t tmp32b = 0;

    Configurator::get_wdtime(&tmp32);
    Configurator::get_wdenable(&tmp32b);

    if((tmp32)&&(tmp32b)){

    	//Watchdog wd;
    	Watchdog::instance()->configure((float)tmp32);
    	BOOTLOADER_BLREPORT("* Watchdog enabled and configured to %d seconds\n", (int)tmp32);
    } else {
    	BOOTLOADER_BLREPORT("* Watchdog disabled\n");
    }

    BOOTLOADER_BLREPORT("* Jump to 0x%08X\n", INTPART_FIRMWARE);
	BOOTLOADER_BLREPORT("***************************************************************\n");

	ledRed = BOOTLOADER_LED_OFF;
	ledGrn = BOOTLOADER_LED_OFF;
	ledBlu = BOOTLOADER_LED_ON;

	// wait until printf is printed
	wait_ms(1);

	usb.unconfigureDevice();

	usb.disconnect();

	// wait to uncofingure
	while(usb.configured()){
		wait_ms(10);
	}

	wait_ms(100);

	delete usb;

	// USB pins are set to low because computer couldnt't detect reconnection
	DigitalOut dout3(PA_10);
	DigitalOut dout4(PA_11);
	DigitalOut dout5(PA_12);

	dout3 = 0;
	dout4 = 0;
	dout5 = 0;

	wait_ms(100);

	ledBlu = BOOTLOADER_LED_OFF;

	// end of bootloader -> jump to firmware
	mbed_start_application(0x08010000);

}

void command_mode(){

	BOOTLOADER_BLREPORT("* Now you can type serial commands.\n");
	BOOTLOADER_BLREPORT("* Type 'help' for help\n");

	usb.attach(&received_msg);

	dbg->attach(&received_char, Serial::RxIrq);

	uint8_t		crc8 = 0;
	uint32_t	tmpf = 0;
	uint32_t	rc = 0;

	while(1){

		ledRed = BOOTLOADER_LED_OFF;
		ledGrn = BOOTLOADER_LED_OFF;
		ledBlu = BOOTLOADER_LED_ON;

		// when virtual serial port settings is changed, run bootloader again
		if(welcome){
			welcome = 0;
			bootloader_app();
		}

		if(data_flag){

			ledRed = BOOTLOADER_LED_ON;
			ledGrn = BOOTLOADER_LED_ON;
			ledBlu = BOOTLOADER_LED_OFF;

			int i = 0;
			int white_cnt = 0;

			// calculate leading white spaces
			while (cmd[i]){
				if (isspace(cmd[i])){
					white_cnt++;
				} else {
					break;
				}
				i++;
			}

			// remove leading whitespaces
			// +1 -> termination character
			for(i = 0; i<strlen(cmd)-white_cnt+1; i++){
				cmd[i] = cmd[i+white_cnt];
			}

			/*
			 * parse returns 1 - ok, -1 - error, 0 - no response in resp
			 */

			if(commander.parse(cmd, resp)){

				// remove all cr/lf characters
				trim_crlf(resp);

				// send response
				ByzanceLogger::log(resp);

				// add crc to the end if required
				if(crc_required){
					crc8 = calc_crc(resp);
					ByzanceLogger::log("#%02X", crc8);
				}

				ByzanceLogger::log("\n");

			}

			data_flag = 0;

			ledRed = BOOTLOADER_LED_OFF;
			ledGrn = BOOTLOADER_LED_OFF;
			ledBlu = BOOTLOADER_LED_ON;

			autojump.reset();
		}

		/*
		 * Check if autojump timer expired
		 * eventually jump to main app
		 */
		rc = Configurator::get_autojump(&tmpf);
		if(rc==CONFIG_OK){

			if(autojump.read()>(float)tmpf){

				bootloader_end();
			}
		}
	}
}


void led_blink(){

	if(blink_on){
		ledRed	= BOOTLOADER_LED_ON;
		ledGrn	= BOOTLOADER_LED_ON;
	} else {
		ledRed	= BOOTLOADER_LED_OFF;
		ledGrn	= BOOTLOADER_LED_OFF;
	}

	blink_on=!blink_on;

	ledBlu	=  BOOTLOADER_LED_OFF;
}

void catastrophic_failure(){

	ledRed = BOOTLOADER_LED_OFF;
	ledGrn = BOOTLOADER_LED_OFF;
	ledBlu = BOOTLOADER_LED_OFF;

	while(1){
		ledRed=!ledRed;
		wait_ms(500);
	}
}

void bootloader_print(const char* format, ...){

	char buffer[256];
	uint8_t crc8;
	char tmp[8];

	// sprintf to buffer
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, 256, format, args);
	va_end (args);

	// menu is written into both interfaces
	Configurator::get_blreport(&tmp32);
	if(tmp32){
		dbg->printf(buffer);
		usb.printf(buffer);
	}

}
