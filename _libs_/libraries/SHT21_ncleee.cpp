/* Copyright (c) 2012 Graeme Coapes, Newcastle University, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


 #include "mbed.h"
 #include "SHT21_ncleee.h"

REGISTER_DEBUG(SHT21);
 
 // Class constructor

//SHT21::SHT21(I2C *i2c, Serial *pc) :
//_i2c(i2c), _pc(pc)
//{
//}

SHT21::SHT21(I2C *i2c, uint32_t frequency): _i2c(i2c)
{
	_i2c->frequency(frequency);
}

int SHT21::triggerTemp()
{
    return wr(SHT_TRIG_TEMP);
}
 
int SHT21::requestTemp()
{
    int res;
    
    int rx_bytes = 3;
    
    char rx[rx_bytes];
    
    res = _i2c->read(SHT_I2C_ADDR,rx,rx_bytes);
    if(res){
    	__ERROR("read failed, code = %d\n", res);
    	return 0;
    }
    
//    _pc->printf("%x %x %x \n", rx[0], rx[1], rx[2]);
    
    //should use checksum here
    
    //shift the MSByte to the left of the 16-bit temperature value
    //don't shift the LSByte
    //Clear bit 1 and bit 0 of the result - these are status bits sent from the sensor
    //temperature = ((rx[0] << 8) || (rx[1] << 0)) ; //&& (0x00FC);
    unsigned short msb = (rx[0] << 8);
    unsigned short lsb = (rx[1] << 0);
    
    temperature = msb + lsb;
    
    //_pc->printf("msb: 0x%x, lsb: 0x%x, r: 0x%x, temp: 0x%x \n",msb, lsb,r,  temperature);
    
    return res;
}
 
float SHT21::readTemp() {

	int rc = 0;

    //First of all trigger the temperature reading
    //process on the sensor
    rc = triggerTemp();
    
    if(rc != SHT_SUCCESS) {
    	__ERROR("triggering failed (code=%d), returning last temperature\n", rc);
        //if this has failed, exit function with specific error condition
        return _temp;
    }
    
    //else pause whilst sensor is measuring
    //maximum measuring time is: 85ms
    Thread::wait(100);
    
    rc = requestTemp();

    //Now request the temperature
    if(rc != SHT_SUCCESS) {
    	__ERROR("error getting temp (code=%d), returning last temperature\n", rc);
        //if this has failed, exit function with specific error condition
        return _temp;
    }    
    
    //the received temperature value should now
    //be stored in the temperature field
    
    
    
    _temp = -46.85 + 175.72 * ( ((float) temperature) / 65536 );

    return _temp;
} 
 
int SHT21::triggerRH() {
    return wr(SHT_TRIG_RH);
}

int SHT21::requestRH(){
    int res;
    
    char rx[3];
    
    res = _i2c->read(SHT_I2C_ADDR,rx,3);
    
    //should use checksum here
    
    //shift the MSByte to the left of the 16-bit temperature value
    //don't shift the LSByte
    //Clear bit 1 and bit 0 of the result - these are status bits sent from the sensor
    //humidity = ((rx[0] << 8) || (rx[1] << 0)) && (0xFC);
    humidity = (rx[0]<<8) + rx[1];
    
    
    return res;
}
 
float SHT21::readHumidity() {

	int rc = 0;

	rc = triggerRH();

	//First of all trigger the temperature reading
    //process on the sensor
    if(rc != SHT_SUCCESS) {
    	__ERROR("triggering failed (code = %d), returning last humidity\n", rc);
        //if this has failed, exit function with specific error condition
        return _hum;
    }
    
    //else pause whilst sensor is measuring
    //maximum measuring time is: 85ms
    Thread::wait(100);
    
    rc = requestRH();

    //Now request the temperature
    if(rc != SHT_SUCCESS) {
    	__ERROR("error requesting humidity (code = %d), returning last humidity\n", rc);
        //if this has failed, exit function with specific error condition
        return _hum;
    }
    
    _hum = -6 + 125 * ( ((float) humidity) / 65536 );
    return _hum;
    
    //the received temperature value should now
    //be stored in the temperature field
//    return humidity;
    
    
} 
 
int SHT21::reset() {

	int res = 0;

	res = wr(SHT_SOFT_RESET);

    return res;
}


int SHT21::setPrecision(char precision) {
    int res = 0;
    
    char command[2];
    command[0] = SHT_WRITE_REG;
    command[1] = precision;
    
    res = _i2c->write(SHT_I2C_ADDR, command, 2);
    if(res){
    	__ERROR("setPrecision\n");
	}
    
    return res;
}

int SHT21::wr(int cmd){
   
    int res = 0;
    
    // TODO: udělat lépe
    char command[1];
    command[0] = cmd;
    
    res = _i2c->write(SHT_I2C_ADDR, command, 1);
    if(res){
    	__ERROR("WR ERROR = %d\n", res);
    }
    
    return res;
}
 
 
 
 
 
 
