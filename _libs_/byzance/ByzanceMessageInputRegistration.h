#ifndef BYZANCEMESSAGEINPUTREGISTRATION_H
#define BYZANCEMESSAGEINPUTREGISTRATION_H

#include "mbed.h"

class ByzanceParser;

/** 
* \class ByzanceMessageInputRegistration 
* \brief brief TO DO.  
*  
* Description TO DO 
*/
class ByzanceMessageInputRegistration {
public:
	
	/**
	* Constructor 
	* \param
	* \param 
	*/
    ByzanceMessageInputRegistration(const char* name, void (*fptr)(ByzanceParser*));
	
	/**
	* get_name 
	* \return
	*/ 
    const char* get_name();
		
	/**
	* call
	* \param 
	*/ 
    void call(ByzanceParser* value);

	
protected:
    const char* _name;
    Callback<void(ByzanceParser*)> _function;
};

#endif /* BYZANCEMESSAGEINPUTREGISTRATION_H */
