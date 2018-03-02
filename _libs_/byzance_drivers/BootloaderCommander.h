#ifndef BOOTLOADER_COMMANDER_H
#define BOOTLOADER_COMMANDER_H

#if BYZANCE_BOOTLOADER

#include "mbed.h"
#include <string>
#include <deque>
#include "ExtMem.h"
#include "IntMem.h"
#include "otp.h"

// logging support
#include "byzance_debug.h"
#include "ByzanceLogger.h"

#include "struct_wifi_credentials.h"
#include "struct_mqtt.h"
#include "macros_bootloader.h"
#include "support_functions.h"
#include "BinManager.h"
#include "MqttManager.h"

#include "Configurator.h"

typedef enum{
	COMMAND_TYPE_WITHOUT,
	COMMAND_TYPE_PARAM,
	COMMAND_TYPE_BOTH
} cmd_type_t;

typedef enum{
	COMMAND_RESPONSE_OK,
	COMMAND_RESPONSE_MISSING,
	COMMAND_RESPONSE_ERROR
} cmd_resp_t;

struct struct_bootloader_cmd {
	cmd_type_t	type;
	const char*	name;
	int (*fp)(cmd_type_t type, const char*, char*);
};

class BootloaderCommander{

	public:
		BootloaderCommander();
		void	init(ExtMem *extmem, IntMem *intmem);

		size_t	size(void);

		/*
		 * @return
		 * 	positive - resp present
		 * 	0 - no response
		 * 	-1 - error, there is an error msg in response
		 */
		int		parse(char* cmd, char* resp);
		int		get(uint8_t index, char* cmd);

    /*
     * Without parameter
     */
		static int cmd_ping(cmd_type_t type, const char* arg, char* resp);
		static int cmd_help(cmd_type_t type, const char* arg, char* resp);
		static int cmd_overview(cmd_type_t type, const char* arg, char* resp);
		static int cmd_restart(cmd_type_t type, const char* arg, char* resp);
		static int cmd_target(cmd_type_t type, const char* arg, char* resp);
		static int cmd_fullid(cmd_type_t type, const char* arg, char* resp);
		static int cmd_launch_reset(cmd_type_t type, const char* arg, char* resp);
		static int cmd_defaults(cmd_type_t type, const char* arg, char* resp);

#if BYZANCE_SUPPORTS_DEVLIST
		static int cmd_devlist_clear(cmd_type_t type, const char* arg, char* resp);
		static int cmd_devlist_counter(cmd_type_t type, const char* arg, char* resp);
		static int cmd_devlist_list(cmd_type_t type, const char* arg, char* resp);
#endif

    /*
     * With and without parameter
     */
		static int cmd_normal_mqtt_hostname(cmd_type_t type, const char* arg, char* resp);
		static int cmd_normal_mqtt_port(cmd_type_t type, const char* arg, char* resp);
		static int cmd_backup_mqtt_hostname(cmd_type_t type, const char* arg, char* resp);
		static int cmd_backup_mqtt_port(cmd_type_t type, const char* arg, char* resp);
		static int cmd_mqtt_username(cmd_type_t type, const char* arg, char* resp);
		static int cmd_mqtt_password(cmd_type_t type, const char* arg, char* resp);

#if BYZANCE_SUPPORTS_WIFI
		static int cmd_wifi_ssid(cmd_type_t type, const char* arg, char* resp);
		static int cmd_wifi_pass(cmd_type_t type, const char* arg, char* resp);
#endif

		static int cmd_alias(cmd_type_t type, const char* arg, char* resp);
		static int cmd_mac(cmd_type_t type, const char* arg, char* resp);
		static int cmd_blreport(cmd_type_t type, const char* arg, char* resp);
		static int cmd_wdenable(cmd_type_t type, const char* arg, char* resp);
		static int cmd_wdtime(cmd_type_t type, const char* arg, char* resp);
		static int cmd_autobackup(cmd_type_t type, const char* arg, char* resp);
		static int cmd_netsource(cmd_type_t type, const char* arg, char* resp);
		static int cmd_configured(cmd_type_t type, const char* arg, char* resp);
		static int cmd_timestamp(cmd_type_t type, const char* arg, char* resp);
		static int cmd_webview(cmd_type_t type, const char* arg, char* resp);
		static int cmd_webport(cmd_type_t type, const char* arg, char* resp);
		static int cmd_timeoffset(cmd_type_t type, const char* arg, char* resp);
		static int cmd_timesync(cmd_type_t type, const char* arg, char* resp);
		static int cmd_lowpanbr(cmd_type_t type, const char* arg, char* resp);
		static int cmd_restartbl(cmd_type_t type, const char* arg, char* resp);
		static int cmd_revision(cmd_type_t type, const char* arg, char* resp);
		static int cmd_autojump(cmd_type_t type, const char* arg, char* resp);

		static int cmd_info(cmd_type_t type, const char* arg, char* resp);
		static int cmd_memsize(cmd_type_t type, const char* arg, char* resp);
		static int cmd_format(cmd_type_t type, const char* arg, char* resp);
		static int cmd_firmware(cmd_type_t type, const char* arg, char* resp);

		static const char* type_to_str(cmd_type_t type);

		static void show_binary(struct_binary* bin, char* resp);

		/** TODO add brief
	     *
	     * TODO add long description
	     *
	     * @param bool
	     *
	     */
		static void attach_led_blink(void (*function)(void));

		/** TODO add brief
	     *
	     * TODO add long description
	     *
	     * @param bool
	     *
	     */
	    template<typename T>
		static void attach_led_blink(T *object, void (T::*member)(void)) {
			_led_blink.attach(object, member);
		}

		static size_t 				_cmdcount;
		static ExtMem*				_extmem;
		static IntMem*				_intmem;

		// led blink callback
		static Callback<void(void)>	_led_blink;

	private:

		/*
		 * convert function pointer to ascii name
		 */
		static const char* _get_name(int (*fp)(cmd_type_t type, const char*, char*));
};

/*
 * LIST OF COMMANDS
 */
const struct_bootloader_cmd commands[] = {

	/**********************************************
	 *
	 * WITHOUT PARAMETER
	 *
	 **********************************************/

	{COMMAND_TYPE_WITHOUT, "ping",				BootloaderCommander::cmd_ping},
	{COMMAND_TYPE_WITHOUT, "help",				BootloaderCommander::cmd_help},
	{COMMAND_TYPE_WITHOUT, "overview",			BootloaderCommander::cmd_overview},
	{COMMAND_TYPE_WITHOUT, "restart",			BootloaderCommander::cmd_restart},
	{COMMAND_TYPE_WITHOUT, "target",			BootloaderCommander::cmd_target},
	{COMMAND_TYPE_WITHOUT, "fullid",			BootloaderCommander::cmd_fullid},
	{COMMAND_TYPE_WITHOUT, "launch_reset",		BootloaderCommander::cmd_launch_reset},
	{COMMAND_TYPE_WITHOUT, "defaults",			BootloaderCommander::cmd_defaults},

#if BYZANCE_SUPPORTS_DEVLIST
	{COMMAND_TYPE_WITHOUT, "devlist_clear",		BootloaderCommander::cmd_devlist_clear},
	{COMMAND_TYPE_WITHOUT, "devlist_counter",	BootloaderCommander::cmd_devlist_counter},
	{COMMAND_TYPE_WITHOUT, "devlist_list",		BootloaderCommander::cmd_devlist_list},
#endif

	/**********************************************
	 *
	 * WITH AND WITHOUT PARAMETER
	 *
	 **********************************************/

	{COMMAND_TYPE_BOTH,  "normal_mqtt_hostname",	BootloaderCommander::cmd_normal_mqtt_hostname},
	{COMMAND_TYPE_BOTH,  "normal_mqtt_port",		BootloaderCommander::cmd_normal_mqtt_port},
	{COMMAND_TYPE_BOTH,  "backup_mqtt_hostname",	BootloaderCommander::cmd_backup_mqtt_hostname},
	{COMMAND_TYPE_BOTH,  "backup_mqtt_port",		BootloaderCommander::cmd_backup_mqtt_port},

	// not readable
	{COMMAND_TYPE_PARAM, "mqtt_username",			BootloaderCommander::cmd_mqtt_username},
	{COMMAND_TYPE_PARAM, "mqtt_password",			BootloaderCommander::cmd_mqtt_password},

	{COMMAND_TYPE_BOTH, "alias",		BootloaderCommander::cmd_alias},
	{COMMAND_TYPE_BOTH, "mac",			BootloaderCommander::cmd_mac},
	{COMMAND_TYPE_BOTH, "blreport", 	BootloaderCommander::cmd_blreport},
	{COMMAND_TYPE_BOTH, "wdenable",		BootloaderCommander::cmd_wdenable},
	{COMMAND_TYPE_BOTH, "wdtime",		BootloaderCommander::cmd_wdtime},
	{COMMAND_TYPE_BOTH, "autobackup",	BootloaderCommander::cmd_autobackup},
	{COMMAND_TYPE_BOTH, "netsource",	BootloaderCommander::cmd_netsource},
	{COMMAND_TYPE_BOTH, "configured",	BootloaderCommander::cmd_configured},
	{COMMAND_TYPE_BOTH, "timestamp",	BootloaderCommander::cmd_timestamp},
	{COMMAND_TYPE_BOTH, "webview",		BootloaderCommander::cmd_webview},
	{COMMAND_TYPE_BOTH, "webport",		BootloaderCommander::cmd_webport},
	{COMMAND_TYPE_BOTH, "timeoffset",	BootloaderCommander::cmd_timeoffset},
	{COMMAND_TYPE_BOTH, "timesync",		BootloaderCommander::cmd_timesync},
	{COMMAND_TYPE_BOTH, "lowpanbr",		BootloaderCommander::cmd_lowpanbr},
	{COMMAND_TYPE_BOTH, "restartbl",	BootloaderCommander::cmd_restartbl},
	{COMMAND_TYPE_BOTH, "revision",		BootloaderCommander::cmd_revision},
	{COMMAND_TYPE_BOTH, "autojump",		BootloaderCommander::cmd_autojump},
  
#if BYZANCE_SUPPORTS_WIFI
	{COMMAND_TYPE_BOTH, "wifi_ssid",	BootloaderCommander::cmd_wifi_ssid},
	{COMMAND_TYPE_BOTH, "wifi_pass",	BootloaderCommander::cmd_wifi_pass},
#endif

	/**********************************************
	 *
	 * PARAMETER ONLY
	 *
	 **********************************************/

	// bootloader, firmware, buffer, backup
	{COMMAND_TYPE_PARAM, "info",		BootloaderCommander::cmd_info},

	// bootloader, firmware, buffer, backup
	{COMMAND_TYPE_PARAM, "memsize",		BootloaderCommander::cmd_memsize},

	// intmem, extmem
	{COMMAND_TYPE_PARAM, "format",		BootloaderCommander::cmd_format},

	// backup, restore
	{COMMAND_TYPE_PARAM, "firmware",	BootloaderCommander::cmd_firmware},

};

#endif /* BYZANCE_BOOTLOADER */

#endif /* BOOTLOADER_COMMANDER_H */
