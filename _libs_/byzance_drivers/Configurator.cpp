#include "Configurator.h"

ExtMem*					Configurator::_extmem;

struct_config_ioda		Configurator::_config;
bool 					Configurator::_initialised = false;

CONFIGURATOR_MUTEX_DEFINITION_C;

// TODO: add bootloader - interface bootloader_print
REGISTER_DEBUG(Configurator);

int Configurator::init(ExtMem* extmem){

	int rc = 0;

	_extmem	= extmem;

	// load structs
	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_CONFIGURATION, &_config, sizeof(struct_config_ioda));
	if(rc){
		__ERROR("init: extmem read error (code=%d)\n", rc);
		return CONFIG_ERROR_MEMORY;
	}

	rc = set_defaults();
	if(rc>0){
		__WARNING("init: some component set to default\n");
	} else if (rc<0){
		__ERROR("init: some component set to default error\n");
	}

	// show structs
	_initialised=true;
	return 0;
}

int Configurator::set_defaults(bool forced){

	int rc		= 0;
	bool erase	= 0;
	bool correct	= 0;

	// check if field should be set to default
	if(!forced){

		if(struct_is_empty(&_config, sizeof(struct_config_ioda))){
			erase = 1;
		} else {

			/*
			 * NOW CHECK AND EVENTUALLY CORRECT VALUES
			 */
			if(_config.flashflag==0xFFFFFFFF){
				_config.flashflag = DEFAULTS_CONF_FLASHFLAG;
				correct = 1;
			}

			if(_config.autobackup==0xFFFFFFFF){
				_config.autobackup = DEFAULTS_CONF_AUTOBACKUP;
				correct = 1;
			}

			if(_config.wdenable==0xFFFFFFFF){
				_config.wdenable = DEFAULTS_CONF_WATCHDOG_ENABLED;
				correct = 1;
			}

			if(_config.wdtime==0xFFFFFFFF){
				_config.wdtime = DEFAULTS_CONF_WATCHDOG_TIME;
				correct = 1;
			}

			if(_config.blreport==0xFFFFFFFF){
				_config.blreport = DEFAULTS_CONF_BOOTLOADER_REPORT;
				correct = 1;
			}

			if(_config.netsource==0xFFFFFFFF){
				_config.netsource = DEFAULTS_CONF_NETSOURCE;
				correct = 1;
			}

			if(_config.configured==0xFFFFFFFF){
				_config.configured = DEFAULTS_CONF_CONFIGURED;
				correct = 1;
			}

			if(_config.launched==0xFFFFFFFF){
				_config.launched = DEFAULTS_CONF_LAUNCHED;
				correct = 1;
			}

			// alias must be composed of printable characters and must contain at least one termination byte
			uint8_t count_print = 0;
			uint8_t count_null	= 0;

			for(uint32_t i = 0; i<sizeof(_config.alias); i++){

				// is printable ascii character?
				if(isprint(_config.alias[i])){
					count_print++;

				// is termination byte?
				} else if(_config.alias[i]==0x00){
					count_null++;
					break;
				} else {
					break;
				}
			}

			if(!(count_print>0)||!(count_null>0)){
				correct = 1;
			}

			if(_config.webview==0xFFFFFFFF){
				_config.webview = DEFAULTS_CONF_WEBVIEW;
				correct = 1;
			}

			if(_config.webport==0xFFFFFFFF){
				_config.webport = DEFAULTS_CONF_WEBPORT;
				correct = 1;
			}

			if(_config.timeoffset==0xFFFFFFFF){
				_config.timeoffset = DEFAULTS_CONF_TIMEOFFSET;
				correct = 1;
			}

			if(_config.timesync==0xFFFFFFFF){
				_config.timesync = DEFAULTS_CONF_TIMESYNC;
				correct = 1;
			}

			if(_config.lowpanbr==0xFFFFFFFF){
				_config.lowpanbr = DEFAULTS_CONF_LOWPANBR;
				correct = 1;
			}

			if(_config.restartbl==0xFFFFFFFF){
				_config.restartbl = DEFAULTS_CONF_RESTARTBL;
				correct = 1;
			}

			if(_config.autojump==0xFFFFFFFF){
				_config.autojump = DEFAULTS_CONF_AUTOJUMP;
				correct = 1;
			}
		}
	}

	// set to default
	if(erase||forced||correct){
		if(erase||forced){
			defaults_struct_config_ioda(&_config);
			__DEBUG("set_defaults: set\n");
		}
		rc = _save_config();
		if(rc){
			return -1;
		}

		return 1;
	}

	return 0;
}

int Configurator::get_flashflag(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.flashflag;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;

}

int Configurator::get_autobackup(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.autobackup;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_wdenable(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.wdenable;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_wdtime(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.wdtime;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_blreport(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.blreport;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_netsource(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.netsource;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_configured(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.configured;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_launched(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.launched;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_alias(char* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	strcpy(val, _config.alias);

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_webview(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.webview;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_webport(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.webport;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_timeoffset(int* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.timeoffset;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_timesync(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.timesync;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_lowpanbr(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.lowpanbr;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_restartbl(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.restartbl;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::get_autojump(uint32_t* val, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	*val = _config.autojump;

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_flashflag(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.flashflag){

		_config.flashflag = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_autobackup(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.autobackup){

		_config.autobackup = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_wdenable(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.wdenable){

		_config.wdenable = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_wdtime(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.wdtime){

		_config.wdtime = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_blreport(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.blreport){

		_config.blreport = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_netsource(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.netsource){

		_config.netsource = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_configured(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.configured){

		_config.configured = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_launched(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.launched){

		_config.launched = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_alias(const char* val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _config.alias)!=0){

		strcpy(_config.alias, val);

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_webview(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.webview){

		_config.webview = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_webport(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.webport){

		_config.webport = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_timeoffset(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.timeoffset){

		_config.timeoffset = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_timesync(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.timesync){

		_config.timesync = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_lowpanbr(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.lowpanbr){

		_config.lowpanbr = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_restartbl(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.restartbl){

		_config.restartbl = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::set_autojump(uint32_t val, bool save, uint32_t ms){

	CONFIGURATOR_UNUSED_MS;

	CONFIGURATOR_INIT_CHECK;

	CONFIGURATOR_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_config.autojump){

		_config.autojump = val;

		// save it
		if(save){
			rc = _save_config();
			if(rc){
				CONFIGURATOR_MUTEX_UNLOCK;
				return CONFIG_ERROR_MEMORY;
			}
		}
	}

	CONFIGURATOR_MUTEX_UNLOCK;
	return CONFIG_OK;
}

int Configurator::_save_config(){

	int rc = 0;

	rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_CONFIGURATION, &_config, sizeof(struct_config_ioda));
	__DEBUG("_save_config: config struct has been saved\n");

	return rc;

}
