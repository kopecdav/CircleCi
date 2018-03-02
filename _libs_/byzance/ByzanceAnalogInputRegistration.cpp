#include "ByzanceAnalogInputRegistration.h"

#include "ByzanceCore.h"

ByzanceAnalogInputRegistration::ByzanceAnalogInputRegistration(const char* name, void (*fptr)(float)) {
    _name = name;
    _function = callback(fptr);
    ByzanceCore::add_analog_input_registration(this);
}

const char* ByzanceAnalogInputRegistration::getName() {
    return _name;
}

void ByzanceAnalogInputRegistration::call(float value) {
    _lastValue = value;
    _function.call(value);
}

float ByzanceAnalogInputRegistration::getLastValue() {
    return _lastValue;
}
