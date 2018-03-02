#ifndef BYZANCEDIGITALINPUTREGISTRATION_H
#define BYZANCEDIGITALINPUTREGISTRATION_H

#include "mbed.h"

/**
* \class ByzanceDigitalInputRegistration
* \brief TO DO.
* 
* Description TO DO
* 
*/
class ByzanceDigitalInputRegistration {
public:
	/**
	* ByzanceDigitalInputRegistration constructor 
	* \param
	* \param
	*/
    ByzanceDigitalInputRegistration(const char* number, void (*fptr)(bool));
	
	/**
	* To do
	* \param
	* \param
	* \return
	*/
    const char* get_name();
	
	/**
	* To do
	* \param
	* \param
	* \return
	*/
    void call(bool value);
	
	/**
	* To do
	* \param
	* \param
	* \return
	*/
    bool get_last_value();
protected:
    const char* _name;
    bool _lastValue = false;
    Callback<void(bool)> _function;
};


#endif /* BYZANCEDIGITALINPUTREGISTRATION_H */
