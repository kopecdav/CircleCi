#include "mbed.h"

#include "ByzanceParser.h"
#include "ByzanceProtocol.h"

REGISTER_DEBUG(ByzanceParser);

char ByzanceParser::_value[MQTT_MAX_PAYLOAD_SIZE];

ByzanceParser::ByzanceParser(char *value, int valueLen){

	if (valueLen < 5 || valueLen > MQTT_MAX_PAYLOAD_SIZE) {
		__ERROR("Invalid message protocol length.\n");
		_invalid = 1;
		return;
	}

	if (value[0] != BYZANCE_PROTOCOL_VERSION_1) {
		__ERROR("Invalid message protocol version; code = %d\n");
		_invalid = 1;
		return;
	}

	memcpy(_value, value, valueLen);

	_valueLen = valueLen;
	_pointer = 0;
	_invalid = 0;

	if (_valueLen < 5) {
		__ERROR("Invalid message protocol length.\n");
		_invalid = 1;
		return;
	}

	_argTypes[0] = _value[1] >> 4;
	_argTypes[1] = _value[1] & 0x0F;
	_argTypes[2] = _value[2] >> 4;
	_argTypes[3] = _value[2] & 0x0F;
	_argTypes[4] = _value[3] >> 4;
	_argTypes[5] = _value[3] & 0x0F;
	_argTypes[6] = _value[4] >> 4;
	_argTypes[7] = _value[4] & 0x0F;

	_pointer = 5;

}

ByzanceParser::~ByzanceParser() {

}

ByzanceInt ByzanceParser::getByzanceInt(const char position)
{
	ByzanceInt ret = 0;

	// invalid message
	if (_invalid) {
		__ERROR("invalid message\n");
		return 0;
	}

	// invalid argument
	if (_argTypes[position-1] != BYZANCE_PROTOCOL_INT_TYPE) {
		_invalid = 1;
		__ERROR("Invalid type on arg%d (expected %02X got %02X).\n", position, BYZANCE_PROTOCOL_INT_TYPE, _argTypes[position-1]);
		return 0;
	}

	// too big
	int retLen = sizeof(ret);
	if ((_pointer + retLen) > _valueLen) {
		_invalid = 1;
		__ERROR("request too big\n");
		return 0;
	}

	// get argument
	reverseCopy(&ret, (_value + _pointer), retLen);
	_pointer += retLen;

	return ret;
}

ByzanceString ByzanceParser::getByzanceString(const char position)
{
	ByzanceString ret = (ByzanceString)"";

	// invalid message
	if (_invalid) {
		__ERROR("invalid message\n");
		return 0;
	}

	// invalid argument
	if (_argTypes[position-1] != BYZANCE_PROTOCOL_STRING_TYPE) {
		_invalid = 1;
		__ERROR("Invalid type on arg%d (expected %02X got %02X).\n", position, BYZANCE_PROTOCOL_STRING_TYPE, _argTypes[position-1]);
		return 0;
	}

	// too big
	int strLen = (strlen(_value + _pointer)+1);
	if ((_pointer + strLen) > _valueLen) {
		_invalid = 1;
		__ERROR("end reached\n");
		return 0;
	}

	// get argument
	// free is used in macros_byzance.h
	// byzanceParser->get##ARGX
	// ARGX ## Free(argX);
	ret = (ByzanceString)malloc(strLen);
	strcpy(ret, (_value + _pointer));
	_pointer += strLen;

	return ret;
}

ByzanceBool ByzanceParser::getByzanceBool(const char position)
{
	ByzanceBool ret = 0;

	// invalid message
	if (_invalid) {
		__ERROR("invalid message\n");
	}

	// invalid argument
	if (_argTypes[position-1] != BYZANCE_PROTOCOL_BOOL_TYPE) {
		_invalid = 1;
		__ERROR("Invalid type on arg%d (expected %02X got %02X).\n", position, BYZANCE_PROTOCOL_BOOL_TYPE, _argTypes[position-1]);
		return 0;
	}

	// too big
	int retLen = sizeof(ret);
	if ((_pointer + retLen) > _valueLen) {
		_invalid = 1;
		__ERROR("end reached\n");
		return 0;
	}

	// get argument
	reverseCopy(&ret, (_value + _pointer), retLen);
	_pointer += retLen;

	return ret;
}

ByzanceFloat ByzanceParser::getByzanceFloat(const char position)
{
	ByzanceFloat ret = 0.0f;

	// invalid message
	if (_invalid) {
		__ERROR("invalid message\n");
		return 0;
	}

	// invalid argument
	if (_argTypes[position-1] != BYZANCE_PROTOCOL_FLOAT_TYPE) {
		_invalid = 1;
		__ERROR("Invalid type on arg%d (expected %02X got %02X).\n", position, BYZANCE_PROTOCOL_FLOAT_TYPE, _argTypes[position-1]);
		return 0;
	}

	// too big
	int retLen = sizeof(ret);
	if ((_pointer + retLen) <= _valueLen) {
		_invalid = 1;
		__ERROR("end reached\n");
		return 0;
	}

	// get argument
	reverseCopy(&ret, (_value + _pointer), retLen);
	_pointer += retLen;

	return ret;
}

/*
 * 0 - ok, cokoliv jiného - chyba
 */
int ByzanceParser::validate(){

	return _invalid;

}

void ByzanceParser::reverseCopy(void *dst, const void *src, size_t n)
{
	size_t i;
	for (i=0; i < n; ++i)
		((char*)dst)[n-1-i] = ((char*)src)[i];
}
