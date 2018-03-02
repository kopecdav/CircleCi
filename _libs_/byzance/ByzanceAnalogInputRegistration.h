#ifndef BYZANCEANALOGINPUTREGISTRATION_H
#define BYZANCEANALOGINPUTREGISTRATION_H

#include "mbed.h"

/**
* \class ByzanceAnalogInputRegistration
* \brief TO DO. 
*
* Further information TO DO 
*/
class ByzanceAnalogInputRegistration {
public:
	/**
	* Constructor ByzanceAnalogInputRegistration
	* \param number TO DO
	* \param *ftpr TO DO
	*/
    ByzanceAnalogInputRegistration(const char* number, void (*fptr)(float));
	/**
	* getName() TO DO
	* \return
	*/
    const char* getName();
	/**
	* call TO DO
	* \param value TO DO
	* \return
	*/
    void call(float value);
	/**
	* getLastValue() TO DO 
	* \return
	*/
    float getLastValue();
protected:
    const char* _name;
    float _lastValue = 0.0f;
    Callback<void(float)> _function;
};

#endif /* BYZANCEANALOGINPUTREGISTRATION_H */
