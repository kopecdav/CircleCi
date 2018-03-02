
#ifndef BYZANCESERIALIZER_H
#define BYZANCESERIALIZER_H

#include "ByzanceTypes.h"
#include "ByzanceProtocol.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "byzance_config.h"
#include "mbed.h"
#include "rtos.h"


#define SERIALIZER_CHCECK_LOCK \
	if(!_locked)\
		return 0;


/**
* \class ByzanceSerializer
* \brief Brief TO DO. 
*
* Description TO DO
*/
class ByzanceSerializer {
public:

	/**
	* Constructor 
	*
	*/
    ByzanceSerializer();
    ~ByzanceSerializer();
    
	
	/**
	*  addByzanceInt
	*
	* \param
	* \param
	* \return
	*/ 
	bool addByzanceInt(ByzanceInt value, const char position);
    
	/**
	* addByzanceBool
	*
	* \param
	* \param
	* \return
	*/ 
	bool addByzanceBool(ByzanceBool value, const char position);
	
	/**
	* addByzanceFloat  
	*
	* \param
	* \param
	* \return
	*/ 
    bool addByzanceFloat(ByzanceFloat value, const char position);
    
	/**
	* addByzanceString  
	*
	* \param
	* \param
	* \return
	*/ 
	bool addByzanceString(ByzanceStringConst value, const char position);
    
	/**
	* addByzanceString  
	*
	* \param
	* \param
	* \return
	*/ 
	bool addByzanceString(ByzanceString value, const char position);
    
	/**
	*  validate
	*
	* \param none
	* \return 0 - ok, cokoliv jiného - chyba
	*/
	int validate();

	/**
	*  getValue 
	*
	* \param
	* \param
	* \return
	*/ 
	char* getValue();
    
	/**
	*  getValueLen 
	*
	* \param
	* \param
	* \return
	*/ 
	int getValueLen();

protected:

	static Mutex _lock;

	//char *_value;

    static char _value[MQTT_MAX_PAYLOAD_SIZE];

	int _size;

	bool _invalid;

	bool _locked;

	/**
	*  reverseCopy 
	*
	* \param
	* \param
	* \return
	*/ 
    void _reverseCopy(void *dst, const void *src, size_t n);
    
	/**
	*  writeType
	*
	* \param
	* \param
	* \return
	*/ 
	void _writeType(const char position, const char type);
};

#endif /* BYZANCESERIALIZER_H */
