/*
 * TLC59116.h
 *
 *  Created on: 18. 10. 2017
 *      Author: Viktor
 */

#ifndef TLC59116_H_
#define TLC59116_H_


#include "mbed.h"

#include "byzance_debug.h"
#include "ByzanceLogger.h"

// I2C address on ShieldB1L_LED_170920
#define TLC59116_DEFAULT_ADDRESS		((uint8_t)0b11011110)

// Register definitions in TLC59116 driver
#define TLC59116_ALLCALL    0b11010000
#define TLC59116_SUB1       0b11010010
#define TLC59116_SUB2       0b11010100
#define TLC59116_SUB3       0b11011000
#define TLC59116_RESET      0b11010110

#define TLC59116_MODE1      0x00
#define TLC59116_MODE2      0x01
#define TLC59116_PWM0       0x02
#define TLC59116_PWM1       0x03
#define TLC59116_PWM2       0x04
#define TLC59116_PWM3       0x05
#define TLC59116_PWM4       0x06
#define TLC59116_PWM5       0x07
#define TLC59116_PWM6       0x08
#define TLC59116_PWM7       0x09
#define TLC59116_PWM8       0x0A
#define TLC59116_PWM9       0x0B
#define TLC59116_PWM10      0x0C
#define TLC59116_PWM11      0x0D
#define TLC59116_PWM12      0x0E
#define TLC59116_PWM13      0x0F
#define TLC59116_PWM14      0x10
#define TLC59116_PWM15      0x11
#define TLC59116_GRPPWM     0x12
#define TLC59116_GRPFREQ    0x13
#define TLC59116_LEDOUT0    0x14
#define TLC59116_LEDOUT1    0x15
#define TLC59116_LEDOUT2    0x16
#define TLC59116_LEDOUT3    0x17
#define TLC59116_SUBADR1    0x18
#define TLC59116_SUBADR2    0x19
#define TLC59116_SUBADR3    0x1A
#define TLC59116_ALLCALLADR 0x1B
#define TLC59116_IREF       0x1C
#define TLC59116_EFLAG1     0x1D
#define TLC59116_EFLAG2 	0x1E


class TLC59116 {
    public:

		/**
		 * TLC59116 driver constructor.
		 */
		TLC59116();

		/**
		 * Initialization of the TLC59116 driver and configuration to "Byzance" default state,
		 * i.e. all outputs anabled, individual and global PWM active.
		 *
		 * @return Return 0 if success. Non zero value means some error.
		 */
		uint8_t initialize(PinName sda, PinName scl);

		/**
		 * Reset the TLC59116 chip to power on state (good to call "initialize" function to get it work).
		 */
		uint8_t reset_sw(void);

		/**
		 * Set single channel output number "channel" with given PWM value.
		 *
		 * @param channel Numbers 0 to 15 (16 channels).
		 * @param pwm Value from 0 (off) to 255 (fully on).
		 * @return Return 0 if success. Non zero value means some error.
		 */
		uint8_t set_channel(uint8_t channel, uint8_t pwm);

		/**
		 * Set all 16 channels with one command with given PWM values.
		 *
		 * @param data Array with 16 uint8_t items with PWM value (array element 0 = channel 0, etc).
		 * @return Return 0 if success. Non zero value means some error.
 		 */
		uint8_t set_all_channels(uint8_t * data);

		/**
		 * Direct write to single register in the TLC59116.
		 * Can be used to get the device to alternative configuration.
		 *
		 * @param reg See this header file with register definitions.
		 * @return Return 0 if success. Non zero value means some error.
		 * */
		uint8_t set_register(uint8_t reg, uint8_t value);

		/**
		 * Default configuration also use global outputs dimming.
		 * Each output has individual (256 steps) and global dimming (256 steps) ability.
		 *
		 * @param pwm Value from 0 (off) to 255 (fully on).
		 * @return Return 0 if success. Non zero value means some error.
		 **/
		uint8_t set_global_pwm(uint8_t pwm);

    private:
		uint8_t _addr;
		I2C*	_i2c;
		bool _initialized;


};


#endif /* TLC59116_H_ */
