
#ifndef BYZANCEPARSER_H
#define BYZANCEPARSER_H
 
#include "ByzanceTypes.h"
#include "byzance_debug.h"
#include "ByzanceLogger.h"
#include "byzance_config.h"

/**
* \class ByzanceParser
* \brief brief TO DO. 
*
* Description to do 
*/ 
class ByzanceParser {
public:

	/**
	* constructor 
	* 
	* \param
	* \param
	*/ 
    ByzanceParser(char *value, int valueLen);
    ~ByzanceParser();
	
	/**
	* getByzanceInt
	* 
	* \param
	* \return
	*/
    ByzanceInt getByzanceInt(const char position);
	
	/**
	* getByzanceString
	* 
	* \param
	* \return
	*/
    ByzanceString getByzanceString(const char position);
	
	/**
	* getByzanceBool
	* 
	* \param
	* \return
	*/
    ByzanceBool getByzanceBool(const char position);
	
	/**
	* getByzanceFloat
	* 
	* \param
	* \return
	*/
    ByzanceFloat getByzanceFloat(const char position);


    int validate();

protected:

    static char _value[MQTT_MAX_PAYLOAD_SIZE];

    int _valueLen;
    int _pointer;

    bool _invalid;

    char _argTypes[8];

	/**
	* reverseCopy
	*
	* \param 
	* \param 
	* \param
	*/
    void reverseCopy(void *dst, const void *src, size_t n);
};

#endif /* BYZANCEPARSER_H */
