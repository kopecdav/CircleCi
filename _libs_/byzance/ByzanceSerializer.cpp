#include "mbed.h"

#include "ByzanceSerializer.h"
#include "ByzanceProtocol.h"
#include "byzance_config.h"

REGISTER_DEBUG(ByzanceSerializer);

Mutex ByzanceSerializer::_lock;
char ByzanceSerializer::_value[MQTT_MAX_PAYLOAD_SIZE];
                           
ByzanceSerializer::ByzanceSerializer() {
	//only one locked instance can exist due to static buffer _value
	//try to lock static mutex to avoid multiple instances working with the same buffer
	if(_lock.lock(10) == osEventTimeout){
		__ERROR("serializer failed to lock buffer - older instance still exists\n");
		_locked = false;
		_invalid = true;
		return;
	}else{
		__TRACE("serializer lock OK\n");
		_locked = true;
	}
	//_value = (char*)malloc(MQTT_MAX_PAYLOAD_SIZE);
	_value[0] = BYZANCE_PROTOCOL_VERSION_1;
	_value[1] = BYZANCE_PROTOCOL_NO_TYPE;
	_value[2] = BYZANCE_PROTOCOL_NO_TYPE;
	_value[3] = BYZANCE_PROTOCOL_NO_TYPE;
	_value[4] = BYZANCE_PROTOCOL_NO_TYPE;
	_size = 5;

	_invalid = false;
}

ByzanceSerializer::~ByzanceSerializer() {
	//free(_value);
	if(_locked){	//unlock only if locked by this instance
		__TRACE("serializer unlocking\n");
		_lock.unlock();
	}
	_locked = false;
}

bool ByzanceSerializer::addByzanceInt(ByzanceInt value, const char position) {

	SERIALIZER_CHCECK_LOCK

	int valueLen = sizeof(value);

	if ((_size + valueLen) > MQTT_MAX_PAYLOAD_SIZE) {
		__ERROR("int serializer failed\n");
		_invalid = true;
		return 1;
	}

	_writeType(position, BYZANCE_PROTOCOL_INT_TYPE);
	_reverseCopy((_value + _size), &value, valueLen);
	_size += valueLen;

	__TRACE("int serialization on pos=%d ok\n", position);

	return 0;
}

bool ByzanceSerializer::addByzanceBool(ByzanceBool value, const char position) {

	SERIALIZER_CHCECK_LOCK

	int valueLen = sizeof(value);

	if ((_size + valueLen) > MQTT_MAX_PAYLOAD_SIZE) {
		__ERROR("bool serializer failed\n");
		_invalid = true;
		return 1;
	}

	_writeType(position, BYZANCE_PROTOCOL_BOOL_TYPE);

	_reverseCopy((_value + _size), &value, valueLen);
	_size += valueLen;

	__TRACE("bool serialization on pos=%d ok\n", position);

	return 0;

}

bool ByzanceSerializer::addByzanceFloat(ByzanceFloat value, const char position) {

	SERIALIZER_CHCECK_LOCK

	int valueLen = sizeof(value);

	if ((_size + valueLen) > MQTT_MAX_PAYLOAD_SIZE) {
		__ERROR("float serializer failed\n");
		_invalid = true;
		return 1;
	}

	_writeType(position, BYZANCE_PROTOCOL_FLOAT_TYPE);

	_reverseCopy((_value + _size), &value, valueLen);
	_size += valueLen;

	__TRACE("float serialization on pos=%d ok\n", position);

	return 0;

}

bool ByzanceSerializer::addByzanceString(ByzanceStringConst value, const char position) {

	SERIALIZER_CHCECK_LOCK

	int valueLen = strlen(value) + 1;

	if ((_size + valueLen) > MQTT_MAX_PAYLOAD_SIZE) {
		__ERROR("string serializer failed\n");
		_invalid = true;
		return 1;
	}

	_writeType(position, BYZANCE_PROTOCOL_STRING_TYPE);

	strcpy((_value + _size), value);
	_size += valueLen;

	__TRACE("string serialization on pos=%d ok\n", position);

	return 0;

}

bool ByzanceSerializer::addByzanceString(ByzanceString value, const char position) {

	SERIALIZER_CHCECK_LOCK

	int valueLen = strlen(value) + 1;

	if ((_size + valueLen) > MQTT_MAX_PAYLOAD_SIZE) {
		__ERROR("string serializer failed\n");
		_invalid = true;
		return 1;
	}

	_writeType(position, BYZANCE_PROTOCOL_STRING_TYPE);
	strcpy((_value + _size), value);
	_size += valueLen;

	__TRACE("string serialization on pos=%d ok\n", position);

	return 0;

}

int ByzanceSerializer::validate(){

	return _invalid;

}

char* ByzanceSerializer::getValue() {

	SERIALIZER_CHCECK_LOCK

	return _value;
}

int ByzanceSerializer::getValueLen() {

	SERIALIZER_CHCECK_LOCK

	return _size;
}

void ByzanceSerializer::_reverseCopy(void *dst, const void *src, size_t n)
{
	size_t i;
	for (i=0; i < n; ++i)
		((char*)dst)[n-1-i] = ((char*)src)[i];
}

void ByzanceSerializer::_writeType(const char position, const char type) {
	int pos = 1+((position-1)/2);
	if (position % 2) {
		_value[pos] |= (type << 4) & 0xF0;
	} else {
		_value[pos] |= type & 0x0F;
	}
}
