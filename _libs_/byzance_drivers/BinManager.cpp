#include "BinManager.h"

ExtMem*			BinManager::_extmem;
IntMem*			BinManager::_intmem;

bin_wrapper		BinManager::_bin_buffer;
bin_wrapper		BinManager::_bin_backup;
bin_wrapper		BinManager::_bin_firmware;
bin_wrapper		BinManager::_bin_bootloader;

// used for backup procedure
uint32_t 		BinManager::_backup_index;  	// current part
uint32_t 		BinManager::_backup_counter;	// number of all parts

uint32_t		BinManager::_erase_index;
uint32_t		BinManager::_erase_counter;

bool 			BinManager::_initialised = false;

uint32_t 		BinManager::_bin_size_written = 0;

BINMANAGER_MUTEX_DEFINITION_C;

Callback<void(bin_component_t comp,	bin_state_t state)>	BinManager::_state_changed;

Callback<void(void)>									BinManager::_led_blink;

Ticker 			BinManager::_ticker;

bool			BinManager::_upload_running;
int				BinManager::_upload_time;

// TODO: add bootloader - interface bootloader_print
REGISTER_DEBUG(BinManager);

int BinManager::init(ExtMem* extmem, IntMem* intmem){

	int rc = 0;

	_extmem	= extmem;
	_intmem	= intmem;

	struct_clean(&_bin_buffer, sizeof(bin_wrapper));
	struct_clean(&_bin_backup, sizeof(bin_wrapper));
	struct_clean(&_bin_firmware, sizeof(bin_wrapper));
	struct_clean(&_bin_bootloader, sizeof(bin_wrapper));

	_backup_index	= 0;
	_backup_counter	= 0;

	_erase_index	= 0;
	_erase_counter	= 0;

	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_BUFFER, &_bin_buffer.bin, sizeof(struct_binary));
	if(rc){
		__ERROR("init: init error (code=%d)\n", rc);
		return BINMGR_ERROR_MEMORY;
	}

	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_BACKUP, &_bin_backup.bin, sizeof(struct_binary));
	if(rc){
		__ERROR("init: init error (code=%d)\n", rc);
		return BINMGR_ERROR_MEMORY;
	}

	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_FIRMWARE, &_bin_firmware.bin, sizeof(struct_binary));
	if(rc){
		__ERROR("init: init error (code=%d)\n", rc);
		return BINMGR_ERROR_MEMORY;
	}

	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_BOOTLOADER, &_bin_bootloader.bin, sizeof(struct_binary));
	if(rc){
		__ERROR("init: init error (code=%d)\n", rc);
		return BINMGR_ERROR_MEMORY;
	}

	rc = set_defaults();
	if(rc>0){
		__WARNING("init: some component set to default\n");
	} else if (rc<0){
		__ERROR("init: some component set to default error\n");
	}

	__DEBUG("init: bootloader state:\n");
	show_binary(&_bin_bootloader.bin);
	__DEBUG("init: firmware state:\n");
	show_binary(&_bin_firmware.bin);
	__DEBUG("init: buffer state:\n");
	show_binary(&_bin_buffer.bin);
	__DEBUG("init: backup state:\n");
	show_binary(&_bin_backup.bin);

	/*
	 * If some process failed previously, its restored now
	 */
	restore_processes();

	_initialised = true;

	return BINMGR_OK;
}

int BinManager::_fix_component(bin_component_t comp, bool forced){

	int		rc		= 0;
	bool	erase	= 0;
	bool	correct = 0;

	// check if field should be set to default
	if(!forced){

		if(struct_is_empty(&_component_ptr(comp)->bin, sizeof(struct_binary))){
			erase = 1;
		} else {

			// check and eventually set to correct
			if(_component_ptr(comp)->bin.state>_BINSTRUCT_STATE_LAST){
				_component_ptr(comp)->bin.state = DEFAULTS_BIN_STATE;
				correct = 1;
			}
		}

	}

	// set to default
	if(erase||forced||correct){
		if(erase||forced){
			defaults_struct_bin(&_component_ptr(comp)->bin);
		}

		rc = _save_binary(comp);
		if(rc){
			return -1;
		}

		return 1;
	}

	return 0;
}

int BinManager::set_defaults(bool forced){

	int rc		= 0;
	int ret		= 0;

	rc = _fix_component(BIN_COMPONENT_BOOTLOADER, forced);
	if(rc>0){
		ret +=rc;
		__WARNING("bootloader set to default\n");
	} else if (rc<0){
		__ERROR("bootloader set to default error\n");
		return -1;
	}

	rc = _fix_component(BIN_COMPONENT_FIRMWARE, forced);
	if(rc>0){
		ret +=rc;
		__WARNING("firmware set to default\n");
	} else if (rc<0){
		__ERROR("firmware set to default error\n");
		return -2;
	}

	rc = _fix_component(BIN_COMPONENT_BACKUP, forced);
	if(rc>0){
		ret +=rc;
		__WARNING("backup set to default\n");
	} else if (rc<0){
		__ERROR("backup set to default error\n");
		return -3;
	}

	rc = _fix_component(BIN_COMPONENT_BUFFER, forced);
	if(rc>0){
		ret +=rc;
		__WARNING("buffer set to default\n");
	} else if (rc<0){
		__ERROR("buffer set to default error\n");
		return -4;
	}

	return ret;


}

int BinManager::get_size(bin_component_t comp, uint32_t* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	*val = _component_ptr(comp)->bin.size;

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_crc(bin_component_t comp, uint32_t* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	*val = _component_ptr(comp)->bin.crc;

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_build_id(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	strcpy(val, _component_ptr(comp)->bin.build_id);

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_version(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	strcpy(val, _component_ptr(comp)->bin.version);

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_timestamp(bin_component_t comp, uint32_t* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	*val = _component_ptr(comp)->bin.timestamp;

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_usr_version(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	strcpy(val, _component_ptr(comp)->bin.usr_version);

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_usr_name(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	strcpy(val, _component_ptr(comp)->bin.usr_name);

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::get_state(bin_component_t comp, bin_state_t* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	*val = _component_ptr(comp)->bin.state;

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::set_size(bin_component_t comp, uint32_t val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_component_ptr(comp)->bin.size){
		// save it
		_component_ptr(comp)->bin.size = val;
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;

}

int BinManager::set_crc(bin_component_t comp, uint32_t val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_component_ptr(comp)->bin.crc){
		// save it
		_component_ptr(comp)->bin.crc = val;
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::set_build_id(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _component_ptr(comp)->bin.build_id)!=0){
		// save it
		strcpy(_component_ptr(comp)->bin.build_id, val);
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::set_version(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _component_ptr(comp)->bin.version)!=0){
		// save it
		strcpy(_component_ptr(comp)->bin.version, val);
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;

}

int BinManager::set_timestamp(bin_component_t comp, uint32_t val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_component_ptr(comp)->bin.timestamp){
		// save it
		_component_ptr(comp)->bin.timestamp = val;
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::set_usr_version(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _component_ptr(comp)->bin.usr_version)!=0){
		// save it
		strcpy(_component_ptr(comp)->bin.usr_version, val);
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::set_usr_name(bin_component_t comp, char* val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_INIT_CHECK;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _component_ptr(comp)->bin.usr_name)!=0){
		// save it
		strcpy(_component_ptr(comp)->bin.usr_name, val);
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::set_state(bin_component_t comp, bin_state_t val, uint32_t ms){

	BINMANAGER_UNUSED_MS;

	BINMANAGER_MUTEX_LOCK;

	int rc = 0;

	char _name[16];
	char _state_old[16];
	char _state_new[16];

	name_component(comp, _name);
	name_state(_component_ptr(comp)->bin.state, _state_old);
	name_state(val, _state_new);

	// parameter has changed
	if(val!=_component_ptr(comp)->bin.state){

		__DEBUG("state of comp = %s has changed from %s to %s\n", _name, _state_old, _state_new);

		_component_ptr(comp)->bin.state = val;
		rc = _save_binary(comp);
		if(rc){
			BINMANAGER_MUTEX_UNLOCK;
			return BINMGR_ERROR_MEMORY;
		}

		// call state changed callback
		if(_state_changed){
			_state_changed.call(comp, val);
		}

	} else {
		__TRACE("state of comp = %s is %s (unchanged)\n", _name, _state_old);
	}

	BINMANAGER_MUTEX_UNLOCK;
	return BINMGR_OK;
}

int BinManager::yield(){

	int rc = 0;

	// if some components needs attention, rc!=0

	//__TRACE("going to do backup function; rc=%d\n", rc);
	rc += _backup_function();
	//__TRACE("backup function done; rc=%d\n", rc);

	//__TRACE("going to do erase function; rc=%d\n", rc);
	rc += _erase_function();
	//__TRACE("erase function done; rc=%d\n", rc);

	return rc;

}

int BinManager::restore_processes(){

	int rc = 0;
	int ret = 0;

	/*
	// do backup
	// - if backup is invalid &&
	// - firmware is valid
	if(_bin_backup.bin.state==BINSTRUCT_STATE_INVALID && _bin_firmware.bin.state==BINSTRUCT_STATE_VALID){
		__WARNING("restore_processes: backup process wasn't sucesfully finished, finishing now\n");

		rc = BinManager::backup_start();
		if(rc == BINMGR_BACKUP_STARTED){

			__WARNING("restore_processes: backup started\n");

			// yield until BinManager needs to yield
			do{
				rc = BinManager::yield();
			} while(rc);

			__WARNING("restore_processes: backup finished\n");

			ret++;

		} else if (rc == BINMGR_SKIPPED){

			__WARNING("restore_processes: no backup is needed\n");

		} else {

			__ERROR("restore_processes: backup error = %d\n", rc);

		}
	}
	*/

	// erase memory
	if(_bin_buffer.bin.state==BINSTRUCT_STATE_ERASING){
		__WARNING("restore_processes: erase memory process wasn't sucesfully finished, finishing now\n");

		buffer_erase(EXTMEM_SIZEOF_BUFFER);

		// yield until BinManager needs to yield
		do{
			rc = BinManager::yield();
		} while(rc);

		__WARNING("restore_processes: erase finished\n");

		ret++;

		return ret;
	}

	return rc;

}

int BinManager::fix_build_id(const char* build_id){

	int rc = 0;

	// check if BUILD_ID of running program is the same as in flash
	if((strcmp(build_id, _bin_firmware.bin.build_id)) != 0){

		// expected and currently running ID's are different
		__DEBUG("fix_bid: previous id = %s, current id = %s\n", _bin_firmware.bin.build_id, build_id);

		_bin_firmware.bin.size				  = INTMEM_SIZEOF_FIRMWARE;
		_bin_firmware.bin.crc 				  = 0;
		strncpy(_bin_firmware.bin.build_id, 	build_id, BINARY_BUILD_ID_MAX_LENGTH - 1);
		strcpy(_bin_firmware.bin.version,		DEFAULTS_BIN_VERSION);
		_bin_firmware.bin.timestamp 		  = 0;
		strcpy(_bin_firmware.bin.usr_version,	DEFAULTS_BIN_USR_VERSION);
		strcpy(_bin_firmware.bin.usr_name,		DEFAULTS_BIN_USR_NAME);

		set_state(BIN_COMPONENT_FIRMWARE, BINSTRUCT_STATE_VALID);

		__DEBUG("fix_bid: new firmware info:\n");
		show_binary(&_bin_firmware.bin);

		// save changes
		rc = _save_binary(BIN_COMPONENT_FIRMWARE);
		if(rc){
			__ERROR("fix_bid: extmem write error, catastrophic failure (code=%d)\n", rc);
			return BINMGR_ERROR_MEMORY;
		}

		return BINMGR_FIXED;

	}

	__INFO(" fix_bid: build ID check OK\n");
	return BINMGR_OK;

}

int BinManager::fix_bootloader(const char* version){

	int rc = 0;

	bool bootloader_changed = 0;

	// version has changed
	if(strcmp(version, _bin_bootloader.bin.version)!=0){

		_bin_bootloader.bin.size 			= INTMEM_SIZEOF_BOOTLOADER;
		_bin_bootloader.bin.crc 			= 0;
		_bin_bootloader.bin.timestamp 		= 0;
		strcpy(_bin_bootloader.bin.build_id, version);
		strcpy(_bin_bootloader.bin.version, version);
		strcpy(_bin_bootloader.bin.usr_version, version);
		strcpy(_bin_bootloader.bin.usr_name, 	"BYZANCE BOOTLOADER");
		_bin_bootloader.bin.state = BINSTRUCT_STATE_VALID;

		rc = _save_binary(BIN_COMPONENT_BOOTLOADER);
		if(rc){
			return BINMGR_ERROR_MEMORY;
		}

		return BINMGR_FIXED;
	}

	__INFO(" fix_bid: build ID check OK\n");
	return BINMGR_OK;
}

int BinManager::backup_start(bool check_build_id){

	int rc;

	__WARNING("backup_start: function start\n");

	if(_bin_backup.busy){
		__WARNING("backup_start: already in progress\n");
		return BINMGR_ERROR_BUSY;
	}

	/*
	 * if buffer is busy during update
	 * (probably because of running upload)
	 * don't start backup process
	 * in all cases backup should be valid from constructor
	 */
	if(_bin_buffer.busy){
		__WARNING("backup_start: buffer is busy - probably erasing or uploading data\n");
		return BINMGR_ERROR_BUSY;
	}

	/*
	 * Skip backup process if firmware is invalid
	 * It's usually after "defaults" command in bootloader
	 */
	if(_bin_firmware.bin.state != BINSTRUCT_STATE_VALID){
		__WARNING("backup_start: invalid firmware, nothing to backup\n");
		return BINMGR_ERROR_FAILED;
	}

	// backup file is invalid or current backup doesn't match running firmware
	if((_bin_backup.bin.state!=BINSTRUCT_STATE_VALID)||(check_build_id&&(strcmp(_bin_firmware.bin.build_id, _bin_backup.bin.build_id)!=0))){

		__WARNING("backup_start: backup is invalid, starting procedure\n");

		rc = _backup_prepare();
		if(rc==0){

			__WARNING("backup_start: already backed up\n");
			set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_VALID);
			_bin_backup.busy	= false;

			return -1;

		} else if (rc<0){

			__ERROR("backup_start: backup fail %d\n", rc);
			set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_INVALID);
			_bin_backup.busy	= false;

			return BINMGR_ERROR_FAILED;

		} else {

			_backup_index 	= 0;
			_backup_counter = rc;

			__WARNING("backup_start: has started, cnt = %d\n", rc);
			set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_INVALID);
			_bin_backup.busy	= true;

			return BINMGR_BACKUP_STARTED;

		}

	}

	__WARNING("backup: backup already done\n");

	return BINMGR_SKIPPED;
}

int BinManager::backup_stop(){

	BINMANAGER_INIT_CHECK;

	set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_INVALID);
	_bin_backup.busy	= false;
	_bin_backup.progress= 0;

	return 0;

}

int BinManager::buffer_check(struct_binary* bin){

	BINMANAGER_INIT_CHECK;

	int rc = 0;

	// compare with requested struct
	// return how many bytes are different
	rc = struct_compare(&_bin_buffer.bin, bin, sizeof(struct_binary));

	return rc;

}

int BinManager::buffer_prepare(struct_binary* bin){

	BINMANAGER_INIT_CHECK;

	// keep state of original bin
	_copy_binary(BIN_COMPONENT_BUFFER, bin, 0);

	show_binary(BIN_COMPONENT_BUFFER);

	// buffer wasnt erased earlier -> do it now
	if(_bin_buffer.bin.state != BINSTRUCT_STATE_ERASED){

		buffer_erase(bin->size);

		__INFO(" buffer_prepare: erase extmem from index 0x%02X, num=0x%02X\n", _erase_index, _erase_counter);

	}

	// reset bin size counter
	_bin_size_written = 0;

	return BINMGR_OK;
}

void BinManager::buffer_erase(uint32_t size){

	_erase_index	= _extmem->calculate_subsector_index(EXTPART_BUFFER);

	// end
	_erase_counter	= _extmem->calculate_subsector_index(EXTPART_BUFFER + size);
	// minus begin
	_erase_counter -= _extmem->calculate_subsector_index(EXTPART_BUFFER);

	// TODO: ne nutně ++
	_erase_counter++;

	// set erasing flag and busy flag
	set_state(BIN_COMPONENT_BUFFER, BINSTRUCT_STATE_ERASING);
	set_busy(BIN_COMPONENT_BUFFER, true);

	return;
}

int BinManager::write_bin_part(char* data, uint32_t size){

	int rc = 0;

	// zápis samotné binárky
	// poslední parametr je uvedený nepovinně proto, že před zápisem nebudu stránku promazávat, protože to se stalo v upload_start
	rc = _extmem->write_data(EXTPART_BUFFER, _bin_size_written, data, size, false);
	if(rc){
		__DEBUG("write_bin_part: flash write error\n");
		return 1;
	}

	_bin_size_written += size;

	uint32_t tmp32 = 0;
	uint32_t tmp_progress = 0;

	BinManager::get_size(BIN_COMPONENT_BUFFER, &tmp32);

	tmp_progress = (_bin_size_written*100)/tmp32;
	BinManager::set_progress(BIN_COMPONENT_BUFFER, (uint8_t)tmp_progress);

	__DEBUG("write_bin_part: succesfully written %d bytes so far\n", _bin_size_written);

	return 0;
}

int	BinManager::update_component(bin_component_t comp){

	BINMANAGER_INIT_CHECK;

	int		rc = 0;
	uint32_t cnt = 0;

	__TRACE("update start\n");

	switch(comp){
	case BIN_COMPONENT_BOOTLOADER:
		__TRACE("update component bootloader\n");
		break;
	case BIN_COMPONENT_BACKUP:
		__TRACE("update component backup\n");
		break;
	default:
		__ERROR("update component ERROR\n");
	}

	BinManager::show_binary(&_bin_buffer.bin);

	// todo: asi malloc a navratova hodnota z neho
	char ext_memory_buffer[ADDR_FLASH_DEVICE_PAGE_SIZE];

	// todo: kontrola velikosti dané komponenty

	/*
	 * ERASE MEMORY
	 */

	// set component to invalid state
	// erasing and erased state doesnt really makes sense
	set_state(comp, BINSTRUCT_STATE_INVALID);
	// todo: busy and percentage

	uint32_t subsector_begin 	= 0;
	uint32_t subsector_counter	= 0;

	switch(comp){

	case BIN_COMPONENT_BOOTLOADER:

		// erase bootloader section
		__INFO(" erase intmem addr 0x%08X-0x%08X\n", INTMEM_BEGIN, INTMEM_BEGIN + _bin_buffer.bin.size);
		rc = _intmem->erase(INTMEM_BEGIN, INTMEM_BEGIN + _bin_buffer.bin.size);
		if(rc != INTMEM_OK){
			__ERROR("intmem erase error");
			return BINMGR_ERROR_MEMORY;
		}
		break;

	case BIN_COMPONENT_BACKUP:

		// erase backup section
		subsector_begin		= _extmem->calculate_subsector_index(EXTPART_BACKUP);
		// todo: skutečně +1 na konci?!
		subsector_counter	= _extmem->calculate_subsector_index(EXTPART_BACKUP + _bin_buffer.bin.size) - _extmem->calculate_subsector_index(EXTPART_BACKUP) + 1;
		__DEBUG("erase extmem %d sectors from sector %d\n", subsector_counter, subsector_begin);
		rc = _extmem->erase_subsectors(subsector_begin, subsector_counter);
		if(rc != 0){
			__ERROR("erase error.\n");
			return BINMGR_ERROR_MEMORY;
		}
		break;
	default:
		return BINMGR_ERROR_FAILED;
	}

	/*
	 * Calculate number of iterations
	 */
	cnt = _bin_buffer.bin.size/ADDR_FLASH_DEVICE_PAGE_SIZE;
	if(_bin_buffer.bin.size % ADDR_FLASH_DEVICE_PAGE_SIZE) cnt++;

	__TRACE("ready to copy %d pages\n", cnt);

	/*
	 * UPDATE PROCESS
	 */
	for(uint32_t i = 0; i < cnt; i++){

		/*
		 * READ PAGE FROM SOURCE BUFFER
		 */
		__TRACE("reading page %03d/%03d\n", i, cnt);
		rc = _extmem->read_data(EXTPART_BUFFER, i * ADDR_FLASH_DEVICE_PAGE_SIZE, ext_memory_buffer, ADDR_FLASH_DEVICE_PAGE_SIZE);
		if(rc != EXTMEM_OK){
			__ERROR("extmem read error");
			return BINMGR_ERROR_MEMORY;
		}

		/*
		 * WRITE PAGE TO ITS DESTINATION
		 */
		__TRACE("writing page %03d/%03d\n", i, cnt);

		switch(comp){

		case BIN_COMPONENT_BOOTLOADER:
			rc = _intmem->write_data(INTMEM_BEGIN, i * ADDR_FLASH_DEVICE_PAGE_SIZE, (uint8_t*) ext_memory_buffer, ADDR_FLASH_DEVICE_PAGE_SIZE, false);
			if(rc != INTMEM_OK){
				__ERROR("write error");
				return BINMGR_ERROR_MEMORY;
			}
			break;

		case BIN_COMPONENT_BACKUP:
			rc = _extmem->write_data(EXTPART_BACKUP, i * ADDR_FLASH_DEVICE_PAGE_SIZE, (uint8_t*) ext_memory_buffer, ADDR_FLASH_DEVICE_PAGE_SIZE, false);
			if(rc != EXTMEM_OK){
				__ERROR("write error");
				return BINMGR_ERROR_MEMORY;
			}
			break;

		default:
			return BINMGR_ERROR_FAILED;
		}

		__TRACE("done %8d KiB\n", (i + 1) * ADDR_FLASH_DEVICE_PAGE_SIZE);


		/*
		 * update progress in BinManager
		 */
		uint32_t tmp32 = 0;
		uint32_t tmp_progress = 0;

		// get source size
		BinManager::get_size(BIN_COMPONENT_BUFFER, &tmp32);

		// calculate destination progress
		tmp_progress = (((i + 1) * ADDR_FLASH_DEVICE_PAGE_SIZE)*100)/tmp32;
		BinManager::set_progress(comp, (uint8_t)tmp_progress);

		// callback for external led blinking
		if(_led_blink){
			_led_blink.call();
		}
	}

	__TRACE("updating component info\n");

	// update destination (bootloader or backup) with informations from source (buffer)
	_copy_binary(comp, BIN_COMPONENT_BUFFER, false);

	// mark binary as valid
	set_state(comp, BINSTRUCT_STATE_VALID);
	// percentage and busy

	__TRACE("update end\n");

	return BINMGR_OK;

}

int BinManager::flash_firmware(FlashMode_t mode){

	//struct_binary 		bin_firmware;
	uint32_t 			i = 0;
	uint32_t 			cnt = 0;
	uint32_t 			rc = 0;

	bin_component_t		comp = BIN_COMPONENT_BUFFER;

	__TRACE("***************************************************************\n");
	if(mode == MODE_FLASH){
		comp = BIN_COMPONENT_BUFFER;
		__TRACE("flash_firmware: FLASH process start\n");
	} else if (mode == MODE_RESTORE){
		comp = BIN_COMPONENT_BACKUP;
		__TRACE("flash_firmware: RESTORE process start\n");
	}
	__TRACE("***************************************************************\n");

	BinManager::show_binary(comp);

	/*
	 * check source bin validity
	 */
	if(_component_ptr(comp)->bin.state!=BINSTRUCT_STATE_VALID){
		__ERROR("* Firmware is invalid, skipping process\n");
		return BINMGR_ERROR_INVALID;
	}

	/*
	 * TODO: zčeknout, jestli už daná komponenta není flashnutá/restorovaná
	 */

	/*
	 * CHECK AND REPAIR BIN SIZE
	 */
	// MCU program size is not set -> assume to full MCU flash size
	if((_component_ptr(comp)->bin.size == 0x00000000)||(_component_ptr(comp)->bin.size == 0xFFFFFFFF)){

		// in flash mode assume some size to unknown firmware
		if(mode==MODE_FLASH){
			_component_ptr(comp)->bin.size = INTMEM_SIZEOF_FIRMWARE;

			// TODO: save bin

			__TRACE("* Unknown firmware size, assumed to %d bytes\n", _component_ptr(comp)->bin.size);

		// in restore mode don't restore unknown firmware
		} else if (mode==MODE_RESTORE){
			__ERROR("* Unknown size to restore, ending restore procedure\n");
			return BINMGR_ERROR_SIZE;
		}
	}

	// binary is bigger than FIRMWARE section -> assume to full MCU flash size
	if(_component_ptr(comp)->bin.size>INTMEM_SIZEOF_FIRMWARE){
		__ERROR("* Firmware is too big (%d, max %d)\n", _component_ptr(comp)->bin.size, INTMEM_SIZEOF_FIRMWARE);
		return BINMGR_ERROR_SIZE;
	}

	/*
	 * ERASE MCU
	 */

	// set firmware to invalid state
	// erasing and erased state doesnt really makes sense
	set_state(BIN_COMPONENT_FIRMWARE, BINSTRUCT_STATE_INVALID);
	// todo: busy and percentage

	__TRACE("* MCU erasing %d bytes, wait few seconds\n", _component_ptr(comp)->bin.size);

	rc = _intmem->erase(INTPART_FIRMWARE, INTPART_FIRMWARE + _component_ptr(comp)->bin.size);
	if(rc){
		__TRACE("* MCU erase ERROR (%d)\n");
		return BINMGR_ERROR_MEMORY;
	} else {
		__TRACE("* MCU erased succesfully\n");
	}

	/*
	 * COPY FIRMWARE FROM EXT FLASH TO MCU
	 */
	if(mode==MODE_FLASH){
		__TRACE("* Copying %8d bytes from external flash to MCU\n", _component_ptr(comp)->bin.size);
	} else if(mode==MODE_RESTORE){
		__TRACE("* Restoring %8d bytes from external flash to MCU\n", _component_ptr(comp)->bin.size);
	}

	uint8_t fwbuffer[SPIFLASH_SUBSECTOR_SIZE];
	cnt = _component_ptr(comp)->bin.size/SPIFLASH_SUBSECTOR_SIZE;
	if(_component_ptr(comp)->bin.size%SPIFLASH_SUBSECTOR_SIZE) cnt++;

	for(i=0; i<cnt; i++){

		// read one page from external flash
		if(mode==MODE_FLASH){
			rc = _extmem->read_data(EXTPART_BUFFER, i*SPIFLASH_SUBSECTOR_SIZE, fwbuffer, SPIFLASH_SUBSECTOR_SIZE);
		} else if(mode==MODE_RESTORE){
			rc = _extmem->read_data(EXTPART_BACKUP, i*SPIFLASH_SUBSECTOR_SIZE, fwbuffer, SPIFLASH_SUBSECTOR_SIZE);
		}

		if(rc){
			__TRACE("* extmem read error (%d)\n", rc);
			return BINMGR_ERROR_MEMORY;
		}

		// write to intmem
		rc = _intmem->write_data(INTPART_FIRMWARE, i*SPIFLASH_SUBSECTOR_SIZE, fwbuffer, SPIFLASH_SUBSECTOR_SIZE, 0);
		if(rc){
			__TRACE("* intmem write error (%d)\n", rc);
			return BINMGR_ERROR_MEMORY;
		}


		if(mode == MODE_FLASH){
			__TRACE("* Copied %8d KiB\n", (i+1)*SPIFLASH_SUBSECTOR_SIZE);
		} else  {
			__TRACE("* Restored %8d KiB\n", (i+1)*SPIFLASH_SUBSECTOR_SIZE);
		}

		// callback for external led blinking
		if(_led_blink){
			_led_blink.call();
		}
	}

	// copy informations from SOURCE (buffer or backup) to firmware
	_copy_binary(BIN_COMPONENT_FIRMWARE, comp, false);

	// mark binary as valid
	rc = set_state(BIN_COMPONENT_FIRMWARE, BINSTRUCT_STATE_VALID);
	// percentage and busy
	// not really necessarry because this function is called only in bootloader

	/*
	 * UPDATE SECTION WITH FIRMWARE INFORMATIONS
	 */
	if(rc){
		__ERROR("* extmem write error (%d)\n", rc);
		return BINMGR_ERROR_MEMORY;
	} else {
		__TRACE("* this bin has been flashed:\n");
		BinManager::show_binary(comp);

		__TRACE("* Succesfully flashed %8d bytes\n", _component_ptr(comp)->bin.size);
	}

	return BINMGR_OK;

}

bool BinManager::is_busy(bin_component_t comp){

	BINMANAGER_INIT_CHECK;

	return _component_ptr(comp)->busy;

}

bool BinManager::is_busy(){

	if(_bin_bootloader.busy){
		return true;
	}

	if(_bin_firmware.busy){
		return true;
	}

	if(_bin_backup.busy){
		return true;
	}

	if(_bin_buffer.busy){
		return true;
	}

	return false;
}

bool BinManager::set_busy(bin_component_t comp, bool busy){

	BINMANAGER_INIT_CHECK;

	_component_ptr(comp)->busy = busy;

	return BINMGR_OK;
}

bool BinManager::get_progress(bin_component_t comp, uint8_t* val){

	BINMANAGER_INIT_CHECK;

	*val = _component_ptr(comp)->progress;

	return BINMGR_OK;

}

bool BinManager::set_progress(bin_component_t comp, uint8_t val){

	BINMANAGER_INIT_CHECK;

	_component_ptr(comp)->progress = val;

	return BINMGR_OK;
}

bool BinManager::get_bin_info(bin_component_t comp, struct_binary* bin){

	BINMANAGER_INIT_CHECK;

	memcpy(bin, _component_ptr(comp), sizeof(struct_binary));

	return BINMGR_OK;
}

void BinManager::show_binary(bin_component_t comp){

	__DEBUG("*** component = %d ***\n", comp);

	show_binary(&_component_ptr(comp)->bin);
}

void BinManager::show_binary(struct_binary* bin){
	__DEBUG("*** bin overview begin ***\n");
	__DEBUG("- size:		\t %d bytes\n", 	bin->size);
	__DEBUG("- crc:		\t 0x%08X\n", 		bin->crc);
	__DEBUG("- build_id:	\t %s\n",			bin->build_id);
	__DEBUG("- version:	\t %s\n",			bin->version);
	__DEBUG("- timestamp:  \t %u\n",			bin->timestamp);
	__DEBUG("- usr_version:\t %s\n",			bin->usr_version);
	__DEBUG("- usr_name:	\t %s\n",			bin->usr_name);
	__DEBUG("- state:		\t %d\n",			bin->state);
	__DEBUG("*** bin overview end ***\n");
}

int BinManager::_backup_prepare(){

	uint32_t	cnt = 0;
	int 		rc = 0;

	/*
	 * CHECK AND REPAIR BIN SIZE
	 */
	// MCU program size is not set -> assume to full MCU flash size
	if((_bin_firmware.bin.size == 0x00000000)||(_bin_firmware.bin.size == 0xFFFFFFFF)){
		_bin_firmware.bin.size = INTMEM_SIZEOF_FIRMWARE;
		__WARNING("_backup_prepare: unknown firmware size, assumed to %d bytes\n", _bin_firmware.bin.size);

		// save firmware bin with corrected size
		rc = _save_binary(BIN_COMPONENT_FIRMWARE);
		if(rc){
			__ERROR("_backup_prepare: firmware write unsuccessful\n");
			return -1;
		}
	}

	// binary is bigger than real FIRMWARE section
	if(_bin_firmware.bin.size>INTMEM_SIZEOF_FIRMWARE) {
		__ERROR("_backup_prepare: firmware is too big (%d, max %d)\n", _bin_firmware.bin.size, INTMEM_SIZEOF_FIRMWARE);
		return -2;
	}

	/*
	 * FROM NOW FIRMWARE IS VALID, show it
	 */
	__TRACE("_backup_prepare: build_id=%s, size=%d\n", _bin_firmware.bin.build_id, _bin_firmware.bin.size);
	show_binary(&_bin_firmware.bin);

	/*
	 * FIRMWARE IS BACKED UP AND BACKUP IS VALID?
	 */
	if((_bin_backup.bin.build_id == _bin_firmware.bin.build_id) && (_bin_backup.bin.state==BINSTRUCT_STATE_VALID)){
		__ERROR("_backup_prepare: backup is not needed\n");
		return 0;
	}

	// switch to invalid and save
	set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_INVALID);

	// calculate how many iterations are necessary for firmware backup
	cnt = _bin_firmware.bin.size/SPIFLASH_SUBSECTOR_SIZE;
	if(_bin_firmware.bin.size%SPIFLASH_SUBSECTOR_SIZE) cnt++;

	__INFO(" _backup_prepare: required iterations = %d\n", cnt);

	return cnt;

}

int BinManager::_backup_function(){

	int rc = 0;

	/*
	 * PERFORM BACKUP PROCEDURE
	 */
	if(_bin_backup.busy){ // možné ověření na invalid

		__TRACE("_backup_function: backup busy\n");

		// backup required part
		if(_backup_index<_backup_counter){

			__TRACE("_backup_function: backup counter, index=%d, counter=%d\n", _backup_index, _backup_counter);

			uint8_t		fwbuffer[SPIFLASH_SUBSECTOR_SIZE];

			rc = _intmem->read_data(INTPART_FIRMWARE, (uint32_t)(_backup_index*SPIFLASH_SUBSECTOR_SIZE), fwbuffer, SPIFLASH_SUBSECTOR_SIZE);
			if(rc){
				set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_INVALID);
				_bin_backup.busy	= false;
				_bin_backup.progress= 0;
				__ERROR("_backup_function: backup procedure failed during step %d/%d\n", _backup_index, _backup_counter);
				return -1;
			}

			__TRACE("_backup_function: reading ok\n");

			rc = _extmem->write_data(EXTPART_BACKUP, 	(uint32_t)(_backup_index*SPIFLASH_SUBSECTOR_SIZE), fwbuffer, SPIFLASH_SUBSECTOR_SIZE);
			if(rc){
				set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_INVALID);
				_bin_backup.busy	= false;
				_bin_backup.progress= 0;
				__ERROR("_backup_function: backup procedure failed during step %d/%d\n", _backup_index, _backup_counter);
				return -2;
			}

			__TRACE("_backup_function: writing ok\n");

			uint32_t tmp32 = (100*_backup_index)/_backup_counter;
			_bin_backup.progress = (uint8_t)tmp32;

			_backup_index++;

			__DEBUG("_backup_function: backup step %d/%d OK, progress=%d%%\n", _backup_index, _backup_counter, _bin_backup.progress);

			// callback for external led blinking
			if(_led_blink){
				_led_blink.call();
			}

			return 1;

		// backup is finished, switch to finish procedure
		} else if(_backup_index==_backup_counter){

			__DEBUG("_backup_function: finishing\n");

			// copy firmware struct to backup struct
			_copy_binary(BIN_COMPONENT_BACKUP, BIN_COMPONENT_FIRMWARE);

			// mark as valid and automatically save
			set_state(BIN_COMPONENT_BACKUP, BINSTRUCT_STATE_VALID);

			_bin_backup.busy	= false;
			_bin_backup.progress= 0;
			__WARNING("_backup_function: backup finished\n");

		}
	}

	return 0;

}

int BinManager::_erase_function(){

	if(_bin_buffer.bin.state == BINSTRUCT_STATE_ERASING){

		/*
		 * Buffer erase in progress
		 */
		if(_erase_index<_erase_counter){
			// todo: pak to bude debug
			__INFO(" _erase_function: erasing subsector no. 0x%08X\n", _erase_index);
			_extmem->erase_subsectors(_erase_index, 1);
			_erase_index++;

			_bin_buffer.busy		= true;
			_bin_buffer.progress	= (100*_erase_index)/_erase_counter;

			// callback for external led blinking
			if(_led_blink){
				_led_blink.call();
			}

			return 1;

		/*
		 * Buffer erase done
		 */
		} else {

			__WARNING("_erase_function: erased = %d bytes\n", _erase_counter*_extmem->get_subsector_size());

			set_state(BIN_COMPONENT_BUFFER, BINSTRUCT_STATE_ERASED);
			_bin_buffer.busy	= false;
			_bin_buffer.progress= 0;

		}

	}

	return 0;
}

int BinManager::_save_binary(bin_component_t comp){

	int rc = 0;

	switch(comp){
	case BIN_COMPONENT_BOOTLOADER:
		rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_BOOTLOADER,	&_bin_bootloader.bin, sizeof(struct_binary));
		break;
	case BIN_COMPONENT_FIRMWARE:
		rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_FIRMWARE,	&_bin_firmware.bin, sizeof(struct_binary));
		break;
	case BIN_COMPONENT_BACKUP:
		rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_BACKUP,		&_bin_backup.bin, sizeof(struct_binary));
		break;
	case BIN_COMPONENT_BUFFER:
		rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_BUFFER,		&_bin_buffer.bin, sizeof(struct_binary));
		break;
	}

	char buff[16];
	name_component(comp, buff);
	__DEBUG("_save_binary: struct %s has been saved\n", buff);

	return rc;

}

void BinManager::_copy_binary(bin_component_t dest, bin_component_t src, bool copy_state){

	bin_state_t tmp_state = BINSTRUCT_STATE_INVALID;

	char buff_src[16];
	char buff_dest[16];

	name_component(dest, buff_dest);
	name_component(src, buff_src);

	__DEBUG("_copy_binary: copying binary info from %s to %s\n", buff_src, buff_dest);

	if(copy_state){
		// original state of destination before memcpy
		tmp_state = _component_ptr(dest)->bin.state;
	}

	// copy from source to destination included state
	memcpy(_component_ptr(dest), _component_ptr(src), sizeof(struct_binary));

	// restore previous state
	_component_ptr(dest)->bin.state = tmp_state;

	if(copy_state){
		// state of destination has changed during memcpy
		if(tmp_state!=_component_ptr(src)->bin.state){

			// call set state function with new state
			set_state(dest, _component_ptr(src)->bin.state);
		}
	}
}

void BinManager::_copy_binary(bin_component_t dest, struct_binary* src, bool copy_state){

	bin_state_t tmp_state = BINSTRUCT_STATE_INVALID;

	char buff_dest[16];

	name_component(dest, buff_dest);

	__DEBUG("_copy_binary: copying binary info from some temp STRUCT to %s\n", buff_dest);

	if(copy_state){
		// original state of destination before memcpy
		tmp_state = _component_ptr(dest)->bin.state;
	}

	// copy from source to destination included state
	memcpy(_component_ptr(dest), src, sizeof(struct_binary));

	// restore previous state
	_component_ptr(dest)->bin.state = tmp_state;

	if(copy_state){
		// state of destination has changed during memcpy
		if(tmp_state!=src->state){

			// call set state function with new state
			set_state(dest, src->state);
		}
	}
}

bin_wrapper* BinManager::_component_ptr(bin_component_t comp){

	switch(comp){
	case BIN_COMPONENT_BOOTLOADER:
		return &_bin_bootloader;
	case BIN_COMPONENT_FIRMWARE:
		return &_bin_firmware;
	case BIN_COMPONENT_BACKUP:
		return &_bin_backup;
	case BIN_COMPONENT_BUFFER:
		return &_bin_buffer;
	}

	return NULL;
}

void BinManager::_tick_callback(void){

	_upload_time++;

	// ticker overflow
	// stop upload
	if(_upload_time>BIN_UPLOAD_TIMEOUT){

		__ERROR("timeout for upload reached (t=%u), switching to idle\n", _upload_time);

		upload_stop();
	}

}

int BinManager::upload_start(void){

	if(_upload_running){
		__ERROR("upload already running\n");
	} else {
		_ticker.attach(callback(&BinManager::_tick_callback), 1.0);
		_upload_running = true;
		__ERROR("upload has started\n");
	}

	upload_reset();

	return 0;
}

int BinManager::upload_stop(bool forced){

	_ticker.detach();
	_upload_running = false;

	if(forced){
		BinManager::set_state(BIN_COMPONENT_BUFFER, BINSTRUCT_STATE_INVALID);
		__ERROR("forced stop -> bin is invalid\n");
	} else {
		BinManager::set_state(BIN_COMPONENT_BUFFER, BINSTRUCT_STATE_VALID);
		__ERROR("legit stop -> bin is valid\n");
	}

	BinManager::set_busy(BIN_COMPONENT_BUFFER, false);
	BinManager::set_progress(BIN_COMPONENT_BUFFER, 0);

	return 0;
}

int BinManager::upload_reset(void){

	_upload_time = 0;

	__TRACE("time reseted\n");

	return 0;

}

bool BinManager::upload_running(void){

	return _upload_running;

}

void BinManager::name_component(bin_component_t comp, char* name){

	switch(comp){
	case BIN_COMPONENT_BOOTLOADER:
		strcpy(name, "bootloader");
		return;
	case BIN_COMPONENT_FIRMWARE:
		strcpy(name, "firmware");
		return;
	case BIN_COMPONENT_BACKUP:
		strcpy(name, "backup");
		return;
	case BIN_COMPONENT_BUFFER:
		strcpy(name, "buffer");
		return;
	}

	return;
}

void BinManager::name_state(bin_state_t state, char* name){
	switch(state){
	case BINSTRUCT_STATE_INVALID:
		strcpy(name, "INVALID");
		return;
	case BINSTRUCT_STATE_VALID:
		strcpy(name, "VALID");
		return;
	case BINSTRUCT_STATE_ERASING:
		strcpy(name, "ERASING");
		return;
	case BINSTRUCT_STATE_ERASED:
		strcpy(name, "ERASED");
		return;
	}

	return;
}

void BinManager::attach_state_changed(void (*function)(bin_component_t comp, bin_state_t state)) {
	__TRACE("attaching state changed callback\n");
	_state_changed = callback(function);
}

void BinManager::attach_led_blink(void (*function)(void)) {
	__TRACE("attaching led blink callback\n");
	_led_blink = callback(function);
}
