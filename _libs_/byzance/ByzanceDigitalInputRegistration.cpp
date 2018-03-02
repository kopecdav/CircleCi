#include "ByzanceDigitalInputRegistration.h"

#include "ByzanceCore.h"

ByzanceDigitalInputRegistration::ByzanceDigitalInputRegistration(const char* name, void (*fptr)(bool)) {
    _name = name;
    _function = callback(fptr);
    ByzanceCore::add_digital_input_registration(this);
}

const char* ByzanceDigitalInputRegistration::get_name() {
    return _name;
}

void ByzanceDigitalInputRegistration::call(bool value) {
    _lastValue = value;
    _function.call(value);
}

bool ByzanceDigitalInputRegistration::get_last_value() {
    return _lastValue;
}
