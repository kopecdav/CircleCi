/*
 * ms5637.cpp
 *
 *  Created on: 9. 8. 2017
 *      Author: martin
 */
//!
//! @file an520_I2C.c,v
//!
//! Copyright (c) 2009 MEAS Switzerland
//!
//!
//!
//! @brief This C code is for starter reference only. It is written for the
//! MEAS Switzerland MS56xx pressure sensor modules and Atmel Atmega644p
//! microcontroller.
//!
//! @version 1.0 $Id: an520_I2C.c,v 1.0
//!
//! @todo

#include "mbed.h"
#include "ms5637.h"

REGISTER_DEBUG(MS5637);

double P;                       // compensated pressure value (mB)
double T;                       // compensated temperature value (degC)
double A;                       // altitude (ft)
double S;                       // sea level barometer (mB)

uint32_t C[8];                  //coefficient storage

//--------------------------------------------------------------------------------------------------------------------------------------//
// Constructor and destructor

ms5637::ms5637(PinName sda, PinName scl, uint32_t frequency)  : _i2c(sda, scl) {
        _i2c.frequency(frequency);
        _sensor_present=false;
}





//********************************************************
//! @brief send command using I2C hardware interface
//!
//! @return none
//********************************************************

int8_t ms5637::m_i2c_send(char cmd) {

	int8_t ret = _i2c.write(MS5637_ADDR,(char*)&cmd,1);
	if(ret){
		__ERROR("ms5637: i2c write error: %d\n", ret);
	}
	return ret;
	/*
    unsigned char ret;
    ret = m_i2c_start(false);
    if(!(ret)) {
        m_i2c_stop();
    } else {
        ret = m_i2c_write(cmd);
        m_i2c_stop();
    }*/
}

//********************************************************
//! @brief send reset sequence
//!
//! @return -1 on fail, 0 on success
//********************************************************

int8_t ms5637::cmd_reset() {
    int8_t ret = m_i2c_send(MS5637_CMD_RESET);
    wait_ms(4);
    loadCoefs();
    if(ret!=0){
    	_sensor_present=false;
    	return -1;
    }else{
    	_sensor_present=true;
    	return 0;
    }
}

//********************************************************
//! @brief preform adc conversion
//!
//! @return 24bit result or -1 on failure
//********************************************************

int32_t ms5637::cmd_adc(char cmd) {
    char cobuf[3];
    cobuf[0] = 0;
    cobuf[1] = 0;
    cobuf[2] = 0;
    unsigned int ret;
    unsigned long temp = 0;
    ret = m_i2c_send(MS5637_CMD_ADC_CONV + cmd);
    if(ret){
    	return -1;
    }
    switch (cmd & 0x0f) {
        case MS5637_CMD_ADC_256 : wait_us(900); break;
        case MS5637_CMD_ADC_512 : wait_ms(3); break;
        case MS5637_CMD_ADC_1024: wait_ms(4); break;
        case MS5637_CMD_ADC_2048: wait_ms(6); break;
        case MS5637_CMD_ADC_4096: wait_ms(10); break;
    }
    ret = m_i2c_send(MS5637_CMD_ADC_READ);
    if(ret){
		return -1;
	}
    ret = _i2c.read(MS5637_ADDR_R, cobuf, 3, false);
    if(ret){
    	__ERROR("ms5637: i2c error: %d\n", ret);
		return -1;
	}
    temp = (cobuf[0] << 16) + (cobuf[1] << 8) + cobuf[2];
    return temp;
}

//********************************************************
//! @brief Read calibration coefficients
//!
//! @return coefficient
//********************************************************

unsigned int ms5637::cmd_prom(char coef_num) {
    char cobuf[2];
    unsigned int ret;
    unsigned int rC = 0;
    cobuf[0] = 0;
    cobuf[1] = 0;
    m_i2c_send(MS5637_CMD_PROM_RD + coef_num * 2); // send PROM READ command
    ret = _i2c.read(MS5637_ADDR_R, cobuf, 2, false);
    if(ret){
    	__ERROR("ms5637: i2c error: %d\n", ret);
	}
    rC = cobuf[0] * 256 + cobuf[1];
    return rC;
}

//********************************************************
//! @brief calculate the CRC code
//!
//! @return crc code
//********************************************************

unsigned char ms5637::crc4(unsigned int n_prom[]) {
    unsigned int n_rem;
    unsigned int crc_read;
    unsigned char n_bit;
    n_rem = 0x00;
    crc_read = n_prom[7];
    n_prom[7]=(0xFF00 & (n_prom[7]));
    for (int cnt = 0; cnt < 16; cnt++) {
            if (cnt%2 == 1) {
                n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
            } else {
                n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
            }
            for (n_bit = 8; n_bit > 0; n_bit--) {
                if (n_rem & (0x8000)) {
                    n_rem = (n_rem << 1) ^ 0x3000;
                } else {
                    n_rem = (n_rem << 1);
                }
            }
        }
    n_rem= (0x000F & (n_rem >> 12));
    n_prom[7]=crc_read;
    return (n_rem ^ 0x0);
}

/*
The CRC code is calculated and written in factory with the LSB byte in the prom n_prom[7] set to 0x00 (see
Coefficient table below). It is thus important to clear those bytes from the calculation buffer before proceeding
with the CRC calculation itself:
n_prom[7]=(0xFF00 & (n_prom[7])); //CRC byte is replaced by 0
As a simple test of the CRC code, the following coefficient table could be used:
unsigned int nprom[] = {0x3132,0x3334,0x3536,0x3738,0x3940,0x4142,0x4344,0x4500};
the resulting calculated CRC should be 0xB.

DB  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
Addr
0               16 bit reserved for manufacturer
1               Coefficient 1 (16 bit unsigned)
2               Coefficient 2 (16 bit unsigned)
3               Coefficient 3 (16 bit unsigned)
4               Coefficient 4 (16 bit unsigned)
5               Coefficient 5 (16 bit unsigned)
6               Coefficient 6 (16 bit unsigned)
7                                   0   0   0   0     CRC(0x0)
*/
/*
    //Returns 0x0b as per AP520_004
    C[0] = 0x3132;
    C[1] = 0x3334;
    C[2] = 0x3536;
    C[3] = 0x3738;
    C[4] = 0x3940;
    C[5] = 0x4142;
    C[6] = 0x4344;
    C[7] = 0x4546;
    n_crc = ms.crc4(C); // calculate the CRC
    pc.printf("testing CRC: 0x%x\n", n_crc);
*/

//********************************************************
//! @brief load all calibration coefficients
//!
//! @return none
//********************************************************

void ms5637::loadCoefs() {
    for (int i = 0; i < 8; i++){
        wait_ms(50);
        C[i] = cmd_prom(i);
    }
    //unsigned char n_crc = crc4(C);
}

//********************************************************
//! @brief calculate temperature and pressure
//!
//! @return none
//********************************************************

void ms5637::calcPT() {
    int32_t D2 = cmd_adc(MS5637_CMD_ADC_D2 + MS5637_CMD_ADC_4096); // read D2
    if(D2 < 0){
    	__ERROR("ms5637: could not get D2: %d\n", D2);
    	return;
    }
    int32_t D1 = cmd_adc(MS5637_CMD_ADC_D1 + MS5637_CMD_ADC_4096); // read D1
    if(D1 < 0){
    	__ERROR("ms5637: could not get D1: %d\n", D1);
    	return;
    }
    int64_t dT = D2 - ((uint64_t)C[5] << 8);
    int64_t OFF_var  = ((uint32_t)C[2] << 17) + ((dT * (C[4]) >> 6));     //was  OFF  = (C[2] << 17) + dT * C[4] / (1 << 6);
    int64_t SENS = ((uint32_t)C[1] << 16) + ((dT * (C[3]) >> 7));     //was  SENS = (C[1] << 16) + dT * C[3] / (1 << 7);
    //T = (2000 + (((uint64_t)dT * C[6]) / (float)(1 << 23))) / 100;
    T=(float)(2000+(dT*C[6])/8388608)/100;
    //int32_t TEMP = 2000 + (int64_t)dT * (int64_t)C[6] / (int64_t)(1 << 23);
    int32_t TEMP = 2000 + (int64_t)dT * (int64_t)(C[6] >> 23);
    if(TEMP < 2000) { // if temperature lower than 20 Celsius
        float T1 = (TEMP - 2000) * (TEMP - 2000);
        int64_t OFF1_var  = (61 * T1) / 16;
        int64_t SENS1 = (29 * T1) / 16;

        if(TEMP < -1500) { // if temperature lower than -15 Celsius
            T1 = (TEMP + 1500) * (TEMP + 1500);
            OFF1_var  += 17 * T1;
            SENS1 += 9 * T1 ;
        }
        OFF_var -= OFF1_var;
        SENS -= SENS1;
        T = (float)TEMP / 100;
    }
//    int64_t P1 = ((((int64_t)D1 * SENS) >> 21) - OFF) >> 15;
    //P = ((((int64_t)D1 * SENS ) >> 21) - OFF) / (double) (1 << 15) / 100.0;
    P=(D1*SENS/2097152-OFF)/3276800;
}

//********************************************************
//! @brief calculate temperature
//!
//! @return double temperature degC
//********************************************************

double ms5637::calcTemp() {
	if(!_sensor_present){
		return -300;
	}
    calcPT();
    return(T);
}

//********************************************************
//! @brief calculate pressure
//!
//! @return double barometric pressure millibar
//********************************************************

double ms5637::calcPressure() {
	if(!_sensor_present){
		return 0;
	}
    calcPT();
    return(P);
}

//********************************************************
//! @brief get pressure, no calculation
//!
//! @return double barometric pressure millibar
//********************************************************

double ms5637::getPressure() {
	if(!_sensor_present){
		return 0;
	}
    calcPT();
    return(P);
}

//********************************************************
//! @brief get altitude from known sea level barometer,
//! @      no pre-pressure calculation
//!
//! @enter float sea level barometer
//! @return float altitude in feet
//********************************************************

float ms5637::getAltitudeFT(float sea_pressure) {
	if(!_sensor_present){
		return 0;
	}
    A = (1 - (pow((P / (double)sea_pressure), 0.190284))) * 145366.45;
    return((float)A);
}

//********************************************************
//! @brief get sea level pressure from known altitude(ft),
//! @      no pre-pressure calculation
//!
//! @enter float known altitude in feet
//! @return float seal level barometer in mb
//********************************************************

float ms5637::getSeaLevelBaroFT(float known_alt) {
	if(!_sensor_present){
		return 0;
	}
    S = pow(pow((P * INHG), 0.190284) + 0.00001313 * known_alt , 5.2553026) * MB;
    return((float)S);
}

//********************************************************
//! @brief get sea level pressure from known altitude(m),
//! @      no pre-pressure calculation
//!
//! @enter float known altitude in meters
//! @return float seal level barometer in mb
//********************************************************

float ms5637::getSeaLevelBaroM(float known_alt) {
	if(!_sensor_present){
		return 0;
	}
    S = pow(pow((P * INHG), 0.190284) + 0.00001313 * known_alt * FTMETERS , 5.2553026) * MB;
    return((float)S);
}



