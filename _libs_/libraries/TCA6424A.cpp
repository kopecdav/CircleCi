// I2Cdev library collection - TCA6424A I2C device class
// Based on Texas Instruments TCA6424A datasheet, 9/2010 (document SCPS193B)
// 7/31/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2011-07-31 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2011 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "TCA6424A.h"

REGISTER_DEBUG(TCA6424A);

/** Default constructor, uses default I2C address.
 * @see TCA6424A_DEFAULT_ADDRESS
 */
TCA6424A::TCA6424A() {
    _addr = TCA6424A_DEFAULT_ADDRESS;
    _i2c = NULL;
}

/** Specific address constructor.
 * @param address I2C address
 * @see TCA6424A_DEFAULT_ADDRESS
 * @see TCA6424A_ADDRESS_ADDR_LOW
 * @see TCA6424A_ADDRESS_ADDR_HIGH
 */
TCA6424A::TCA6424A(uint8_t address) {
    _addr = address;
    _i2c = NULL;
}

/** Power on and prepare for general usage.
 * The TCA6424A I/O expander requires no preparation after power-on. All pins
 * will be default to INPUT mode, and the device is ready for usage immediately.
 * This is an empty function for consistency and/or future expansion.
 */
void TCA6424A::initialize(PinName sda, PinName scl) {
	_i2c = new I2C(sda, scl);
	_i2c->frequency(400000);
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
bool TCA6424A::testConnection() {
    return _read_bytes(_addr, TCA6424A_RA_INPUT0 | TCA6424A_AUTO_INCREMENT, 3, _buffer) == 3;
}

// INPUT* registers (x0h - x2h)

/** Get a single INPUT pin's logic level.
 * @return Pin logic level (0 or 1)
 */
bool TCA6424A::readPin(uint16_t pin) {
    _read_bit(_addr, TCA6424A_RA_INPUT0 + (pin / 8), pin % 8, _buffer);
    return _buffer[0];
}
/** Get all pin logic levels from one bank.
 * @param bank Which bank to read (0/1/2 for P0*, P1*, P2* respectively)
 * @return 8 pins' logic levels (0 or 1 for each pin)
 */
uint8_t TCA6424A::readBank(uint8_t bank) {
    _read_byte(_addr, TCA6424A_RA_INPUT0 + bank, _buffer);
    return _buffer[0];
}

/** Get all pin logic levels from all banks.
 * Reads into single 3-byte data container.
 * @param banks Container for all bank's pin values (P00-P27)
 */
bool TCA6424A::readAll(uint8_t *banks) {

	int rc = 0;

    rc = _read_bytes(_addr, TCA6424A_RA_INPUT0 | TCA6424A_AUTO_INCREMENT, 3, banks);

    __TRACE("readAll rc = %d\n", rc);
}
/** Get all pin logic levels from all banks.
 * Reads into individual 1-byte containers.
 * @param bank0 Container for Bank 0's pin values (P00-P07)
 * @param bank1 Container for Bank 1's pin values (P10-P17)
 * @param bank2 Container for Bank 2's pin values (P20-P27)
 */
bool TCA6424A::readAll(uint8_t *bank0, uint8_t *bank1, uint8_t *bank2) {

	int rc = 0;

	rc = _read_bytes(_addr, TCA6424A_RA_INPUT0 | TCA6424A_AUTO_INCREMENT, 3, _buffer);
    *bank0 = _buffer[0];
    *bank1 = _buffer[1];
    *bank2 = _buffer[2];

    __TRACE("readAll rc = %d\n", rc);
}

// OUTPUT* registers (x4h - x6h)

/** Get a single OUTPUT pin's setting.
 * Note that this returns the level set in the flip-flop, and does not
 * necessarily represent the actual logic level present at the pin.
 * @return Pin output setting (0 or 1)
 */
bool TCA6424A::getPinOutputLevel(uint16_t pin) {
    _read_bit(_addr, TCA6424A_RA_OUTPUT0 + (pin / 8), pin % 8, _buffer);
    return _buffer[0];
}
/** Get all pin output settings from one bank.
 * Note that this returns the level set in the flip-flop, and does not
 * necessarily represent the actual logic level present at the pin.
 * @param bank Which bank to read (0/1/2 for P0*, P1*, P2* respectively)
 * @return 8 pins' output settings (0 or 1 for each pin)
 */
uint8_t TCA6424A::getBankOutputLevel(uint8_t bank) {
    _read_byte(_addr, TCA6424A_RA_OUTPUT0 + bank, _buffer);
    return _buffer[0];
}
/** Get all pin output settings from all banks.
 * Reads into single 3-byte data container.
 * @param banks Container for all bank's pin values (P00-P27)
 */
void TCA6424A::getAllOutputLevel(uint8_t *banks) {
    _read_bytes(_addr, TCA6424A_RA_OUTPUT0, 3, banks);
}
/** Get all pin output settings from all banks.
 * Reads into individual 1-byte containers. Note that this returns the level
 * set in the flip-flop, and does not necessarily represent the actual logic
 * level present at the pin.
 * @param bank0 Container for Bank 0's pin values (P00-P07)
 * @param bank1 Container for Bank 1's pin values (P10-P17)
 * @param bank2 Container for Bank 2's pin values (P20-P27)
 */
void TCA6424A::getAllOutputLevel(uint8_t *bank0, uint8_t *bank1, uint8_t *bank2) {
    _read_bytes(_addr, TCA6424A_RA_OUTPUT0, 3, _buffer);
    *bank0 = _buffer[0];
    *bank1 = _buffer[1];
    *bank2 = _buffer[2];
}
/** Set a single OUTPUT pin's logic level.
 * @param pin Which pin to write (0-23)
 * @param value New pin output logic level (0 or 1)
 */
void TCA6424A::writePin(uint16_t pin, bool value) {
    _write_bit(_addr, TCA6424A_RA_OUTPUT0 + (pin / 8), pin % 8, value);
}
/** Set all OUTPUT pins' logic levels in one bank.
 * @param bank Which bank to write (0/1/2 for P0*, P1*, P2* respectively)
 * @param value New pins' output logic level (0 or 1 for each pin)
 */
void TCA6424A::writeBank(uint8_t bank, uint8_t value) {
    _write_byte(_addr, TCA6424A_RA_OUTPUT0 + bank, value);
}
/** Set all OUTPUT pins' logic levels in all banks.
 * @param banks All pins' new logic values (P00-P27) in 3-byte array
 */
void TCA6424A::writeAll(uint8_t *banks) {
    _write_bytes(_addr, TCA6424A_RA_OUTPUT0 | TCA6424A_AUTO_INCREMENT, 3, banks);
}
/** Set all OUTPUT pins' logic levels in all banks.
 * @param bank0 Bank 0's new logic values (P00-P07)
 * @param bank1 Bank 1's new logic values (P10-P17)
 * @param bank2 Bank 2's new logic values (P20-P27)
 */
void TCA6424A::writeAll(uint8_t bank0, uint8_t bank1, uint8_t bank2) {
    _buffer[0] = bank0;
    _buffer[1] = bank1;
    _buffer[2] = bank2;
    _write_bytes(_addr, TCA6424A_RA_OUTPUT0 | TCA6424A_AUTO_INCREMENT, 3, _buffer);
}

// POLARITY* registers (x8h - xAh)

/** Get a single pin's polarity (normal/inverted) setting.
 * @return Pin polarity setting (0 or 1)
 */
bool TCA6424A::getPinPolarity(uint16_t pin) {
    _read_bit(_addr, TCA6424A_RA_POLARITY0 + (pin / 8), pin % 8, _buffer);
    return _buffer[0];
}
/** Get all pin polarity (normal/inverted) settings from one bank.
 * @param bank Which bank to read (0/1/2 for P0*, P1*, P2* respectively)
 * @return 8 pins' polarity settings (0 or 1 for each pin)
 */
uint8_t TCA6424A::getBankPolarity(uint8_t bank) {
    _read_byte(_addr, TCA6424A_RA_POLARITY0 + bank, _buffer);
    return _buffer[0];
}
/** Get all pin polarity (normal/inverted) settings from all banks.
 * Reads into single 3-byte data container.
 * @param banks Container for all bank's pin values (P00-P27)
 */
void TCA6424A::getAllPolarity(uint8_t *banks) {
    _read_bytes(_addr, TCA6424A_RA_POLARITY0, 3, banks);
}
/** Get all pin polarity (normal/inverted) settings from all banks.
 * Reads into individual 1-byte containers.
 * @param bank0 Container for Bank 0's pin values (P00-P07)
 * @param bank1 Container for Bank 1's pin values (P10-P17)
 * @param bank2 Container for Bank 2's pin values (P20-P27)
 */
void TCA6424A::getAllPolarity(uint8_t *bank0, uint8_t *bank1, uint8_t *bank2) {
    _read_bytes(_addr, TCA6424A_RA_POLARITY0, 3, _buffer);
    *bank0 = _buffer[0];
    *bank1 = _buffer[1];
    *bank2 = _buffer[2];
}
/** Set a single pin's polarity (normal/inverted) setting.
 * @param pin Which pin to write (0-23)
 * @param polarity New pin polarity setting (0 or 1)
 */
void TCA6424A::setPinPolarity(uint16_t pin, bool polarity) {
    _write_bit(_addr, TCA6424A_RA_POLARITY0 + (pin / 8), pin % 8, polarity);
}
/** Set all pin polarity (normal/inverted) settings in one bank.
 * @param bank Which bank to write (0/1/2 for P0*, P1*, P2* respectively)
 * @return New pins' polarity settings (0 or 1 for each pin)
 */
void TCA6424A::setBankPolarity(uint8_t bank, uint8_t polarity) {
    _write_byte(_addr, TCA6424A_RA_POLARITY0 + bank, polarity);
}
/** Set all pin polarity (normal/inverted) settings in all banks.
 * @param banks All pins' new logic values (P00-P27) in 3-byte array
 */
void TCA6424A::setAllPolarity(uint8_t *banks) {
    _write_bytes(_addr, TCA6424A_RA_POLARITY0 | TCA6424A_AUTO_INCREMENT, 3, banks);
}
/** Set all pin polarity (normal/inverted) settings in all banks.
 * @param bank0 Bank 0's new polarity values (P00-P07)
 * @param bank1 Bank 1's new polarity values (P10-P17)
 * @param bank2 Bank 2's new polarity values (P20-P27)
 */
void TCA6424A::setAllPolarity(uint8_t bank0, uint8_t bank1, uint8_t bank2) {
    _buffer[0] = bank0;
    _buffer[1] = bank1;
    _buffer[2] = bank2;
    _write_bytes(_addr, TCA6424A_RA_POLARITY0 | TCA6424A_AUTO_INCREMENT, 3, _buffer);
}

// CONFIG* registers (xCh - xEh)

/** Get a single pin's direction (I/O) setting.
 * @return Pin direction setting (0 or 1)
 */
bool TCA6424A::getPinDirection(uint16_t pin) {
    _read_bit(_addr, TCA6424A_RA_CONFIG0 + (pin / 8), pin % 8, _buffer);
    return _buffer[0];
}
/** Get all pin direction (I/O) settings from one bank.
 * @param bank Which bank to read (0/1/2 for P0*, P1*, P2* respectively)
 * @return 8 pins' direction settings (0 or 1 for each pin)
 */
uint8_t TCA6424A::getBankDirection(uint8_t bank) {
    _read_byte(_addr, TCA6424A_RA_CONFIG0 + bank, _buffer);
    return _buffer[0];
}
/** Get all pin direction (I/O) settings from all banks.
 * Reads into single 3-byte data container.
 * @param banks Container for all bank's pin values (P00-P27)
 */
void TCA6424A::getAllDirection(uint8_t *banks) {
    _read_bytes(_addr, TCA6424A_RA_CONFIG0, 3, banks);
}
/** Get all pin direction (I/O) settings from all banks.
 * Reads into individual 1-byte containers.
 * @param bank0 Container for Bank 0's pin values (P00-P07)
 * @param bank1 Container for Bank 1's pin values (P10-P17)
 * @param bank2 Container for Bank 2's pin values (P20-P27)
 */
void TCA6424A::getAllDirection(uint8_t *bank0, uint8_t *bank1, uint8_t *bank2) {
    _read_bytes(_addr, TCA6424A_RA_CONFIG0, 3, _buffer);
    *bank0 = _buffer[0];
    *bank1 = _buffer[1];
    *bank2 = _buffer[2];
}
/** Set a single pin's direction (I/O) setting.
 * @param pin Which pin to write (0-23)
 * @param direction Pin direction setting (0 or 1)
 */
void TCA6424A::setPinDirection(uint16_t pin, bool direction) {
    _write_bit(_addr, TCA6424A_RA_CONFIG0 + (pin / 8), pin % 8, direction);
}
/** Set all pin direction (I/O) settings in one bank.
 * @param bank Which bank to read (0/1/2 for P0*, P1*, P2* respectively)
 * @param direction New pins' direction settings (0 or 1 for each pin)
 */
void TCA6424A::setBankDirection(uint8_t bank, uint8_t direction) {
    _write_byte(_addr, TCA6424A_RA_CONFIG0 + bank, direction);
}
/** Set all pin direction (I/O) settings in all banks.
 * @param banks All pins' new direction values (P00-P27) in 3-byte array
 */
void TCA6424A::setAllDirection(uint8_t *banks) {
    _write_bytes(_addr, TCA6424A_RA_CONFIG0 | TCA6424A_AUTO_INCREMENT, 3, banks);
}
/** Set all pin direction (I/O) settings in all banks.
 * @param bank0 Bank 0's new direction values (P00-P07)
 * @param bank1 Bank 1's new direction values (P10-P17)
 * @param bank2 Bank 2's new direction values (P20-P27)
 */
void TCA6424A::setAllDirection(uint8_t bank0, uint8_t bank1, uint8_t bank2) {
    _buffer[0] = bank0;
    _buffer[1] = bank1;
    _buffer[2] = bank2;
    _write_bytes(_addr, TCA6424A_RA_CONFIG0 | TCA6424A_AUTO_INCREMENT, 3, _buffer);
}

int8_t	TCA6424A::_read_bit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data) {
	uint8_t b;
	uint8_t count = _read_byte(devAddr, regAddr, &b);
	*data = b & (1 << bitNum);
	return count;
}

bool	TCA6424A::_write_bit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    _read_byte(devAddr, regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return _write_byte(devAddr, regAddr, b);
}

int8_t	TCA6424A::_read_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data) {
	return _read_bytes(devAddr, regAddr, 1, data);
}

bool	TCA6424A::_write_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
	return _write_bytes(devAddr, regAddr, 1, &data);
}

int8_t	TCA6424A::_read_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {

	__TRACE("_read_bytes: _addr = %d\n", _addr);

	int rc 				= 0;

	// claim read request to given register
	// rc 0 on success, otherwise failure
	rc = _i2c->write(devAddr<<1, (const char*)&regAddr, 1, 1);
	if(rc){
		__ERROR("_read_bytes: write failed\n");
		return 0;
	}

	__TRACE("_read_bytes: write addr = %d, reg = %d, ret1 = %d\n", devAddr, regAddr, rc);

	// read it
	// rc 0 on success, otherwise failure
	rc = _i2c->read(devAddr<<1, (char*)data, length);
	if(rc){
		__ERROR("_read_bytes: read failed\n");
		return 0;
	}

	__TRACE("_read_bytes: read ret1 = %d, returning = %d\n", rc, length);

	return length;
}

bool	TCA6424A::_write_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data) {

	__TRACE("_write_bytes: _addr = %d\n", _addr);

	int rc 				= 0;

	// claim write request to given register
	// rc 0 on success, otherwise failure
	rc = _i2c->write(devAddr<<1, (const char*)&regAddr, 1, 1);
	if(rc){
		__ERROR("_write_bytes: write failed\n");
		return 0;
	}

	__TRACE("_write_bytes: write addr = %d, reg = %d, ret1 = %d\n", devAddr, regAddr, rc);

	// write it
	// rc 0 on success, otherwise failure
	rc = _i2c->write(devAddr<<1, (char*)data, length);
	if(rc){
		__ERROR("_write_bytes: write failed\n");
		return 0;
	}

	__TRACE("_write_bytes: write ret1 = %d, returning = %d\n", rc, length);

	return length;
}
