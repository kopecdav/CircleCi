#include "ByzanceMessageInputRegistration.h"

#include "ByzanceCore.h"
#include "ByzanceParser.h"

ByzanceMessageInputRegistration::ByzanceMessageInputRegistration(const char* name, void (*fptr)(ByzanceParser*)) {
    _name = name;
    _function = callback(fptr);
    ByzanceCore::add_message_input_registration(this);
}

const char* ByzanceMessageInputRegistration::get_name() {
    return _name;
}

void ByzanceMessageInputRegistration::call(ByzanceParser* value) {
    _function.call(value);
}
