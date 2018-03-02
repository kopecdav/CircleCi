#include "MQTTCommands.h"

REGISTER_DEBUG(MQTTCommands);

uint32_t 	MQTTCommands::_bin_crc_calculated		= 0;
uint32_t 	MQTTCommands::_bin_crc_received 		= 0;
uint32_t 	MQTTCommands::_part_index_expected		= 0;

Callback<void(worker_result*)>	MQTTCommands::_req_done;

size_t 		MQTTCommands::_cmdcount;

worker_result		MQTTCommands::_result;

void MQTTCommands::init(picojson::object* json_outcomming){

	_result.json_out = json_outcomming;

	// https://stackoverflow.com/questions/1898657/result-of-sizeof-on-array-of-structs-in-c
	_cmdcount = sizeof cmd_topics / sizeof cmd_topics[0];

}

void MQTTCommands::process(picojson::value& json_data_incomming, const char* subtopic_incomming){

	__TRACE("callback\n");

	char* poscmd;

	for(uint32_t i = 0; i<_cmdcount; i++){

		// find requested command in command set
		poscmd = strstr(subtopic_incomming, cmd_topics[i].name);

		// doplnit druhou část
		if(poscmd==NULL){
			continue;
		}

		// restart can match restartbl, so check whole lengths
		if(strlen(poscmd)!=strlen(cmd_topics[i].name)){
			continue;
		}

		__INFO("topic found -> %s\n", cmd_topics[i].name);

		cmd_topics[i].fp(json_data_incomming);

		return;

	}

	__ERROR("unknown topic\n");

	// not found
	_result.rc = ERROR_CODE_UNKNOWN_TOPIC;
	_req_done(&_result);
	return;

}

void MQTTCommands::upload_start(picojson::value& json_data_incomming){

	int rc = 0;

	// check compulsory labels "size", "crc" and "buildId" and "version"
	if(!json_data_incomming.contains("size") ||\
	   !json_data_incomming.contains("crc") ||\
	   !json_data_incomming.contains("build_id") ||\
	   !json_data_incomming.contains("version")){
			_result.rc = ERROR_CODE_MISSING_LABEL;
			_req_done(&_result);
			return;
	}

	bin_state_t tmp_state;
	// erase is in progress and another erase request received
	BinManager::get_state(BIN_COMPONENT_BUFFER, &tmp_state);
	if(tmp_state==BINSTRUCT_STATE_ERASING){
		_result.rc = ERROR_CODE_COMMAND_ERASE_IN_PROGRESS;
		_req_done(&_result);
		return;
	}

	__INFO("upload/start - request accepted\n");

	struct_binary tmp_bin;

	defaults_struct_bin(&tmp_bin);

	/*
	 * update temporary struct in RAM with given parameters
	 */
	tmp_bin.size	= (uint32_t)round(json_data_incomming.get("size").get<double>());
	tmp_bin.crc		= (uint32_t)round(json_data_incomming.get("crc").get<double>());
	_bin_crc_received	= tmp_bin.crc;
	strncpy(tmp_bin.build_id, json_data_incomming.get("build_id").get<string>().c_str(), BINARY_BUILD_ID_MAX_LENGTH - 1);	// Minus 1, aby byla jistota, ze je strign null terminated.
	strncpy(tmp_bin.version,  json_data_incomming.get("version").get<string>().c_str(), BINARY_VERSION_MAX_LENGTH - 1);		// Minus 1, aby byla jistota, ze je strign null terminated.

	// update optional parameters if given
	if(json_data_incomming.contains("timestamp")){
		tmp_bin.timestamp = (uint32_t)round(json_data_incomming.get("timestamp").get<double>());
	}

	if(json_data_incomming.contains("usr_version")){
		strncpy(tmp_bin.usr_version, json_data_incomming.get("usr_version").get<string>().c_str(), BINARY_USR_VERSION_MAX_LENGTH - 1);	// Minus 1, aby byla jistota, ze je strign null terminated.
	}

	if(json_data_incomming.contains("usr_name")){
		strncpy(tmp_bin.usr_name, json_data_incomming.get("usr_name").get<string>().c_str(), BINARY_USR_NAME_MAX_LENGTH - 1);	// Minus 1, aby byla jistota, ze je strign null terminated.
	}

	BinManager::show_binary(&tmp_bin);

	/*
	 * BINARY UPDATE REQUEST IS VALID FROM NOW
	 */

	/*
	 * Check if buffer binary is different than current one
	 */
	rc = BinManager::buffer_check(&tmp_bin);
	if(rc==0){

		/*
		 * requested and current binary are the same (zero different bytes)
		 * is current binary valid?
		 * If yes, we are sure current binary is ready and it's true duplicity
		 */

		BinManager::get_state(BIN_COMPONENT_BUFFER, &tmp_state);
		if(tmp_state==BINSTRUCT_STATE_VALID){
			_result.rc = ERROR_CODE_COMMAND_BIN_DUPLICITY;
			_req_done(&_result);
			return;
		} else {
			__WARNING("upload/start: binary is currently in buffer, but it's invalid\n");
		}
	} else {
		__WARNING("upload/start: requested bin is different than bin in buffer, diff=%d\n", rc);
	}

	__WARNING("upload/start: upload process continues\n");

	/*
	 * disable flashflag in all cases, from now there can be mess in extmem and integrity of files can be broken
	 */
	uint32_t tmp32;
	rc = Configurator::get_flashflag(&tmp32);
	if(rc){
		__ERROR(" error getting flash flag from Configurator rc=%d\n",rc);
	}else if(tmp32){

		__INFO("upload/start: disabling flashflag\n");
		rc = Configurator::set_flashflag(0);
		if(rc != 0){
			_result.rc = ERROR_CODE_EXTMEM_WRITE_ERROR;
			_req_done(&_result);
			return;
		}
	} else {
		__INFO("upload/start: flashflag wasn't enabled -> OK\n");
	}

	/*
	 * Start erase
	 */
	BinManager::buffer_prepare(&tmp_bin);

	/*
	 * PREPARE ERASE EXTMEM PROCESS
	 */
	_part_index_expected	= 0;
	_bin_crc_calculated 	= 0;

	BinManager::upload_start();

	/*
	 * send response
	 */
	__INFO("upload/start: command done\n");
	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;
}

void MQTTCommands::upload_end(picojson::value& json_data_incomming){

	__INFO("upload/end\n");

	/*
	 * CRC check before upload end
	 */
	if(_bin_crc_received==_bin_crc_calculated){
		__INFO("crc OK (received=%d, calculated=%d)\n", _bin_crc_received, _bin_crc_calculated);
	} else {

		BinManager::upload_stop();

		__ERROR(" crc ERROR (received=%d, calculated=%d)\n", _bin_crc_received, _bin_crc_calculated);
		_result.rc = ERROR_CODE_COMMAND_CRC_ERROR;
		_req_done(&_result);
		return;
	}

	// legit stop -> argument = 0
	BinManager::upload_stop(0);

	__INFO("upload/end: sucessfuly ended\n");
	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTCommands::upload_data(picojson::value& json_data_incomming){

	int rc = 0;

	__TRACE("upload/data\n");

	// compulsory labels "part", "size", "data", "crc"
	if(!json_data_incomming.contains("part") ||\
	   !json_data_incomming.contains("size") ||\
	   !json_data_incomming.contains("data") ||\
	   !json_data_incomming.contains("crc")){

		BinManager::upload_reset();

		_result.rc = ERROR_CODE_MISSING_LABEL;
		_req_done(&_result);
		return;
	}

	uint32_t part_index_received = 0;
	uint32_t part_size = 0;
	uint32_t part_crc_received = 0;
	uint32_t part_crc_calculated = 0;

	part_index_received	= (uint32_t)round(json_data_incomming.get("part").get<double>());
	part_size			= (uint32_t)round(json_data_incomming.get("size").get<double>());

	__TRACE("upload/data: part_index = %d, part_size before base64 = %d bytes\n", part_index_received, json_data_incomming.get("data").get<string>().size());

	// TODO: #DEFINE VELIKOSTI
	unsigned char decoded_buffer[1024];
	size_t decoded_size;

	if(mbedtls_base64_decode(decoded_buffer, sizeof(decoded_buffer), &decoded_size, (unsigned char*)json_data_incomming.get("data").get<string>().c_str(), json_data_incomming.get("data").get<string>().size()) != 0){

		BinManager::upload_reset();

		_result.rc = ERROR_CODE_GENERAL_ERROR;
		_req_done(&_result);
		return;
	}

	__DEBUG("upload/data: base64 decode ok, decoded_size = %d ***\n", decoded_size);

	// at first CRC check must be done
	// othrewise part might be invalid and operations with part may be invalid too
	for(uint32_t i=0; i<decoded_size; i++){
		part_crc_calculated^=decoded_buffer[i];
	}

	part_crc_received = (uint32_t)round(json_data_incomming.get("crc").get<double>());
	__DEBUG("upload/data: part CRC received = 0x%08X, calculated = 0x%08X\n", part_crc_received, part_crc_calculated);

	if(part_crc_received==part_crc_calculated){
		__DEBUG("upload/data: part CRC OK\n");
	} else {

		BinManager::upload_reset();

		_result.rc = ERROR_CODE_COMMAND_CRC_ERROR;
		_req_done(&_result);
		return;
	}

	// check if part is expected
	if(part_index_received!=_part_index_expected){
		// vyhodí chybu o tom, že čeká něco jiného
		(*_result.json_out)["part_required"] = picojson::value((double)_part_index_expected);

		BinManager::upload_reset();

		__ERROR(" upload/data: part #%d is unexpected, required part = %d\n", part_index_received, _part_index_expected);
		_result.rc = ERROR_CODE_COMMAND_UNEXPECTED_PART;
		_req_done(&_result);
		return;
	}

	/*
	 * FROM NOW PART IS CONSIDERED AS VALID
	 */

	BinManager::upload_reset();

	// will be removed in upload/end
	BinManager::set_state(BIN_COMPONENT_BUFFER, BINSTRUCT_STATE_INVALID);
	BinManager::set_busy(BIN_COMPONENT_BUFFER, true);

	// part is accepted now

	// whole binary CRC update
	// note: XOR is not associative (XOR cannot be undone)
	// so this must be done after part index is verified
	for(uint32_t i=0; i<decoded_size; i++){
		_bin_crc_calculated^=decoded_buffer[i];
	}

	__INFO("upload/data: receiving part %d of binary, decoded part size %d\n", part_index_received, decoded_size);

	rc = BinManager::write_bin_part((char*)decoded_buffer, decoded_size);
	if(rc){
		_result.rc = ERROR_CODE_EXTMEM_WRITE_ERROR;
		_req_done(&_result);
		return;
	}

	_part_index_expected++;

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTCommands::update_start(picojson::value& json_data_incomming){

	bin_component_t component;

	// povinný label "component"
	if((!json_data_incomming.contains("component"))){
		_result.rc = ERROR_CODE_MISSING_LABEL;
		_req_done(&_result);
		return;
	}

	if(strcmp((char*)json_data_incomming.get("component").get<string>().c_str(), "firmware") == 0){
		component = BIN_COMPONENT_FIRMWARE;
		__INFO("update of component = firmware\n");
	} else if(strcmp((char*)json_data_incomming.get("component").get<string>().c_str(), "bootloader") == 0){
		component = BIN_COMPONENT_BOOTLOADER;
		__INFO("update of component = bootloader\n");
	} else if(strcmp((char*)json_data_incomming.get("component").get<string>().c_str(), "backup") == 0){
		component = BIN_COMPONENT_BACKUP;
		__INFO("update of component = backup\n");
	} else {
		_result.rc = ERROR_CODE_COMMAND_UNKNOWN_COMPONENT;
		_req_done(&_result);
		return;
	}

	/*
	 * KONTROLA CRC
	 *
	 * Existují 2 způsoby, jak dostat do Yody binárku pro update
	 *
	 * - buď v tomto běhu programu byla binárka doručena, tudíž její CRC mám ve statické proměnné
	 *
	 * - nebo byla doručena do bufferu třeba před týdnem a mezi tím se zařízení několikrát restartovalo
	 *   tudíž crc v RAM nemám (ukládá se tam při komunikaci s Homerem)
	 *   V takovém případě přečtu očekávanou hodnotu z flashky a provedu kontrolu celé flashky.
	 *
	 */

	// pokud je přijatá hodnota nenulová,
	// znamená to, že se v tomto běhu programu posílala nějaká binárka
	// nemusím procházet celou flashku (trvá to), ale jenom porovnám hodnoty od Homera
	// todo: CRC klidně může být 0, co s tím?!
	if(_bin_crc_received){

		if(_bin_crc_received==_bin_crc_calculated){
			__INFO("crc OK (received=%d, calculated=%d)\n", _bin_crc_received, _bin_crc_calculated);
		} else {

			__WARNING("crc ERROR (received=%d, calculated=%d)\n", _bin_crc_received, _bin_crc_calculated);
			_result.rc = ERROR_CODE_COMMAND_CRC_ERROR;
			_req_done(&_result);
			return;
		}

	} else {

		__WARNING("crc ERROR (received=%d, calculated=%d)\n", _bin_crc_received, _bin_crc_calculated);
		_result.rc = ERROR_CODE_COMMAND_CRC_ERROR;
		_req_done(&_result);
		return;
	}

	uint32_t	tmp_size;
	bin_state_t	tmp_state;

	BinManager::get_size(BIN_COMPONENT_BUFFER, &tmp_size);
	// binary has zero size -> missing
	if(tmp_size==0){
		_result.rc = ERROR_CODE_COMMAND_BIN_MISSING;
		_req_done(&_result);
		return;
	}

	BinManager::get_state(BIN_COMPONENT_BUFFER, &tmp_state);
	// binary is not valid
	if(tmp_state!=BINSTRUCT_STATE_VALID){
		_result.rc = ERROR_CODE_COMMAND_BIN_INVALID;
		_req_done(&_result);
		return;
	}

	__INFO("bin in buffer seems to be OK, update continues\n");

	uint32_t rc = 0;

	switch(component){
	case BIN_COMPONENT_FIRMWARE:

		/*
		 * for IODA firmware update just enable flashflag and bootloader will do the rest
		 * to start update process Homer must restart this device
		 */
		__INFO("update - IODA FW\n");

		rc = Configurator::set_flashflag(1);
		if(rc){
			_result.rc = ERROR_CODE_EXTMEM_WRITE_ERROR;
			_req_done(&_result);
			return;
		}

		__INFO("update - flashflag enabled\n");
		_result.rc = ERROR_CODE_OK;
		_req_done(&_result);
		return;

		break;

	case BIN_COMPONENT_BOOTLOADER:

		__INFO("update: ioda bootloader update start\n");
		rc = BinManager::update_component(BIN_COMPONENT_BOOTLOADER);
		if(rc){
			_result.rc = ERROR_CODE_EXTMEM_WRITE_ERROR;
			_req_done(&_result);
			return;
		}

		__INFO("update: ioda bootloader updated succesfully\n");
		_result.rc = ERROR_CODE_OK;
		_req_done(&_result);
		return;

		break;

	case BIN_COMPONENT_BACKUP:

		uint32_t tmp32;
		rc = Configurator::get_autobackup(&tmp32);
		if(rc){
			_result.rc = ERROR_CODE_LOCK_ERROR;
			_req_done(&_result);
			return;
		}
		if(tmp32){
			// if autobackup is enabled, static backup update is declined
			_result.rc = ERROR_CODE_COMMAND_AUTOBACKUP_ENABLED;
			_req_done(&_result);
			return;
		}

		__INFO("update: ioda BACKUP update start\n");
		rc = BinManager::update_component(BIN_COMPONENT_BACKUP);
		if(rc){
			_result.rc = ERROR_CODE_EXTMEM_WRITE_ERROR;
			_req_done(&_result);
			return;
		}

		__INFO("update: ioda BACKUP updated succesfully\n");
		_result.rc = ERROR_CODE_OK;
		_req_done(&_result);
		return;

		break;

	}

}

void MQTTCommands::system_restart(picojson::value& json_data_incomming){

	__INFO("system/restart\n");

	__ERROR(" restarting\n");
	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	// NO RETURN

	// restart in 1 second
	ByzanceCore::restart(1);

	return;

}

void MQTTCommands::system_ping(picojson::value& json_data_incomming){

	__ERROR("system/ping\n");

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;
}

void MQTTCommands::system_blink(picojson::value& json_data_incomming){

	__INFO("system/blink\n");

	// disable automatic blinking
	Byzance::led_module(false);

	// do the led blink sequence
	LedModule::set(LED_COLOR_RED);
	Thread::wait(200);
	LedModule::set(LED_COLOR_GREEN);
	Thread::wait(200);
	LedModule::set(LED_COLOR_BLUE);
	Thread::wait(200);

	// enable automatic led blinking
	Byzance::led_module(true);

	_result.rc = ERROR_CODE_OK;
	_req_done(&_result);
	return;

}

void MQTTCommands::attach_done(void (*function)(worker_result*)) {
	__TRACE("attaching done interrupt\n");
	_req_done = callback(function);
}
