#include "MqttManager.h"

ExtMem*			MqttManager::_extmem;

mqtt_connection		MqttManager::_connection_normal;
mqtt_connection		MqttManager::_connection_backup;
mqtt_credentials	MqttManager::_credentials_broker;

bool 			MqttManager::_initialised = false;

MQTTMANAGER_MUTEX_DEFINITION_C;

// TODO: add bootloader - interface bootloader_print
REGISTER_DEBUG(MqttManager);

int MqttManager::init(ExtMem* extmem){

	int rc = 0;

	_extmem	= extmem;

	// load structs
	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_MQTT_NORMAL, &_connection_normal, sizeof(mqtt_connection));
	if(rc){
		__ERROR("init: extmem read error (code=%d)\n", rc);
		return MQTTMGR_ERROR_MEMORY;
	}

	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_MQTT_BACKUP, &_connection_backup, sizeof(mqtt_connection));
	if(rc){
		__ERROR("init: extmem read error (code=%d)\n", rc);
		return MQTTMGR_ERROR_MEMORY;
	}

	rc = _extmem->read_data(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_MQTT_CREDENTIALS, &_credentials_broker, sizeof(mqtt_credentials));
	if(rc){
		__ERROR("init: extmem read error (code=%d)\n", rc);
		return MQTTMGR_ERROR_MEMORY;
	}

	rc = set_defaults();
	if(rc>0){
		__WARNING("init: some component set to default\n");
	} else if (rc<0){
		__ERROR("init: some component set to default error\n");
	}

	// show structs

	_initialised = true;

	return MQTTMGR_OK;
}

int MqttManager::get_hostname(mqtt_type_t type, char* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	strcpy(val, _connection_ptr(type)->hostname);

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;

}

int MqttManager::get_port(mqtt_type_t type, uint32_t* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	*val = _connection_ptr(type)->port;

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;

}

int MqttManager::get_connection(mqtt_type_t type, mqtt_connection* connection, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	memcpy(connection, _connection_ptr(type), sizeof(mqtt_connection));

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;

}

int MqttManager::get_username(char* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	strcpy(val, _credentials_broker.username);

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::get_password(char* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	strcpy(val, _credentials_broker.password);

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::get_credentials(mqtt_credentials* credentials, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	memcpy(credentials, &_credentials_broker, sizeof(mqtt_credentials));

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::set_hostname(mqtt_type_t type, const char* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _connection_ptr(type)->hostname)!=0){
		// save it
		strcpy(_connection_ptr(type)->hostname, val);
		rc = _save_connection(type);
		if(rc){
			MQTTMANAGER_MUTEX_UNLOCK;
			return MQTTMGR_ERROR_MEMORY;
		}
	}

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::set_port(mqtt_type_t type, const uint32_t val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(val!=_connection_ptr(type)->port){
		// save it
		_connection_ptr(type)->port = val;
		rc = _save_connection(type);
		if(rc){
			MQTTMANAGER_MUTEX_UNLOCK;
			return MQTTMGR_ERROR_MEMORY;
		}
	}

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;

}

int MqttManager::set_connection(mqtt_type_t type, mqtt_connection* connection, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(struct_compare(_connection_ptr(type), connection, sizeof(mqtt_connection))!=0){
		// save it
		memcpy(_connection_ptr(type), connection, sizeof(mqtt_connection));
		rc = _save_connection(type);
		if(rc){
			MQTTMANAGER_MUTEX_UNLOCK;
			return MQTTMGR_ERROR_MEMORY;
		}
	}

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::set_username(const char* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _credentials_broker.username)!=0){
		// save it
		strcpy(_credentials_broker.username, val);
		rc = _save_credentials();
		if(rc){
			MQTTMANAGER_MUTEX_UNLOCK;
			return MQTTMGR_ERROR_MEMORY;
		}
	}

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;

}

int MqttManager::set_password(const char* val, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(strcmp(val, _credentials_broker.password)!=0){
		// save it
		strcpy(_credentials_broker.password, val);
		rc = _save_credentials();
		if(rc){
			MQTTMANAGER_MUTEX_UNLOCK;
			return MQTTMGR_ERROR_MEMORY;
		}
	}

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::set_credentials(mqtt_credentials* credentials, uint32_t ms){

	MQTTMANAGER_UNUSED_MS;

	MQTTMANAGER_INIT_CHECK;

	MQTTMANAGER_MUTEX_LOCK;

	int rc = 0;

	// parameter has changed
	if(struct_compare(&_credentials_broker, credentials, sizeof(mqtt_credentials))!=0){
		// save it
		memcpy(&_credentials_broker, credentials, sizeof(mqtt_credentials));
		rc = _save_credentials();
		if(rc){
			MQTTMANAGER_MUTEX_UNLOCK;
			return MQTTMGR_ERROR_MEMORY;
		}
	}

	MQTTMANAGER_MUTEX_UNLOCK;
	return MQTTMGR_OK;
}

int MqttManager::set_defaults(bool forced){

	int rc		= 0;
	int ret		= 0;

	rc = _fix_connection(MQTT_NORMAL, forced);
	if(rc>0){
		ret +=rc;
		__WARNING("normal connection set to default\n");
	} else if (rc<0){
		__ERROR("normal connection set to default error\n");
		return -1;
	}

	rc = _fix_connection(MQTT_BACKUP, forced);
	if(rc>0){
		ret +=rc;
		__WARNING("backup connection set to default\n");
	} else if (rc<0){
		__ERROR("backup connection set to default error\n");
		return -1;
	}

	rc = _fix_credentials(forced);
	if(rc>0){
		ret +=rc;
		__WARNING("credentials set to default\n");
	} else if (rc<0){
		__ERROR("credentials set to default error\n");
		return -1;
	}

	return ret;
}

int MqttManager::_fix_connection(mqtt_type_t type, bool forced){

	int	 rc		= 0;
	bool erase	= 0;
	bool correct = 0;

	// check if field should be set to default
	if(!forced){

		if(struct_is_empty(_connection_ptr(type), sizeof(mqtt_connection))){
			erase = 1;
		} else {

			// NOW CHECK AND EVENTUALLY CORRECT VALUES
			// we expect array of printable ASCII characters followed by termination 0
			// http://www.cplusplus.com/reference/cctype/isprint/
			// https://cs.wikipedia.org/wiki/ASCII

			// find termination 0
			// check all previous characters - if they are printable
			// if some of them is not printable -> set defaults

		}
	}

	// set to default
	if(erase||forced||correct){
		if(erase||forced){
			defaults_mqtt_connection(_connection_ptr(type));
		}

		rc = _save_connection(type);
		if(rc){
			return -1;
		}

		return 1;
	}

	return 0;

}

int MqttManager::_fix_credentials(bool forced){

	int	 rc		= 0;
	bool erase	= 0;
	bool correct = 0;

	// check if field should be set to default
	if(!forced){

		if(struct_is_empty(&_credentials_broker, sizeof(mqtt_credentials))){
			erase = 1;
		} else {

			// NOW CHECK AND EVENTUALLY CORRECT VALUES
			// we expect array of printable ASCII characters followed by termination 0
			// http://www.cplusplus.com/reference/cctype/isprint/
			// https://cs.wikipedia.org/wiki/ASCII

			// find termination 0
			// check all previous characters - if they are printable
			// if some of them is not printable -> set defaults

		}
	}

	// set to default
	if(erase||forced||correct){
		if(erase||forced){
			defaults_mqtt_credentials(&_credentials_broker);
		}

		rc = _save_credentials();
		if(rc){
			return -1;
		}

		return 1;
	}

	return 0;

}

// todo: fix credentials

mqtt_connection* MqttManager::_connection_ptr(mqtt_type_t type){

	switch(type){
	case MQTT_NORMAL:
		return &_connection_normal;
	case MQTT_BACKUP:
		return &_connection_backup;
	}

	return NULL;

}

int MqttManager::_save_connection(mqtt_type_t type){

	int rc = 0;

	switch(type){
	case MQTT_NORMAL:
		rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_MQTT_NORMAL, &_connection_normal, sizeof(mqtt_connection));
		__DEBUG("_save_connection: normal connection has been saved\n");
		break;
	case MQTT_BACKUP:
		rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_MQTT_BACKUP, &_connection_backup, sizeof(mqtt_connection));
		__DEBUG("_save_connection: backup connection has been saved\n");
		break;
	}

	return rc;

}

int MqttManager::_save_credentials(){

	int rc = 0;

	rc = _extmem->secure_write(EXTPART_CONFIG, EXTMEM_OFFSET_STRUCT_MQTT_CREDENTIALS, &_credentials_broker, sizeof(mqtt_credentials));
	__DEBUG("_save_connection: normal connection has been saved\n");

	return rc;

}
