/*
 * MPU9150.h
 *
 *  Created on: 14. 12. 2017
 *      Author: martin
 */

#ifndef MPU9150_H_
#define MPU9150_H_

#include "mbed.h"
#include "I2CWrapper.h"

// Define registers per MPU6050, Register Map and Descriptions, Rev 4.2, 08/19/2013 6 DOF Motion sensor fusion device
// Invensense Inc., www.invensense.com
// See also MPU-9150 Register Map and Descriptions, Revision 4.0, RM-MPU-9150A-00, 9/12/2012 for registers not listed in
// above document; the MPU6050 and MPU 9150 are virtually identical but the latter has an on-board magnetic sensor
//
//Magnetometer Registers
#define WHO_AM_I_AK8975A 0x00 // should return 0x48
#define INFO             0x01
#define AK8975A_ST1      0x02  // data ready status bit 0
#define AK8975A_ADDRESS  0x0C<<1
#define AK8975A_XOUT_L   0x03  // data
#define AK8975A_XOUT_H   0x04
#define AK8975A_YOUT_L   0x05
#define AK8975A_YOUT_H   0x06
#define AK8975A_ZOUT_L   0x07
#define AK8975A_ZOUT_H   0x08
#define AK8975A_ST2      0x09  // Data overflow bit 3 and data read error status bit 2
#define AK8975A_CNTL     0x0A  // Power down (0000), single-measurement (0001), self-test (1000) and Fuse ROM (1111) modes on bits 3:0
#define AK8975A_ASTC     0x0C  // Self test control
#define AK8975A_ASAX     0x10  // Fuse ROM x-axis sensitivity adjustment value
#define AK8975A_ASAY     0x11  // Fuse ROM y-axis sensitivity adjustment value
#define AK8975A_ASAZ     0x12  // Fuse ROM z-axis sensitivity adjustment value

#define XGOFFS_TC        0x00 // Bit 7 PWR_MODE, bits 6:1 XG_OFFS_TC, bit 0 OTP_BNK_VLD
#define YGOFFS_TC        0x01
#define ZGOFFS_TC        0x02
#define X_FINE_GAIN      0x03 // [7:0] fine gain
#define Y_FINE_GAIN      0x04
#define Z_FINE_GAIN      0x05
#define XA_OFFSET_H      0x06 // User-defined trim values for accelerometer
#define XA_OFFSET_L_TC   0x07
#define YA_OFFSET_H      0x08
#define YA_OFFSET_L_TC   0x09
#define ZA_OFFSET_H      0x0A
#define ZA_OFFSET_L_TC   0x0B
#define SELF_TEST_X      0x0D
#define SELF_TEST_Y      0x0E
#define SELF_TEST_Z      0x0F
#define SELF_TEST_A      0x10
#define XG_OFFS_USRH     0x13  // User-defined trim values for gyroscope, populate with calibration routine
#define XG_OFFS_USRL     0x14
#define YG_OFFS_USRH     0x15
#define YG_OFFS_USRL     0x16
#define ZG_OFFS_USRH     0x17
#define ZG_OFFS_USRL     0x18
#define SMPLRT_DIV       0x19
#define CONFIG           0x1A
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define FF_THR           0x1D  // Free-fall
#define FF_DUR           0x1E  // Free-fall
#define MOT_THR          0x1F  // Motion detection threshold bits [7:0]
#define MOT_DUR          0x20  // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define ZMOT_THR         0x21  // Zero-motion detection threshold bits [7:0]
#define ZRMOT_DUR        0x22  // Duration counter threshold for zero motion interrupt generation, 16 Hz rate, LSB = 64 ms
#define FIFO_EN          0x23
#define I2C_MST_CTRL     0x24
#define I2C_SLV0_ADDR    0x25
#define I2C_SLV0_REG     0x26
#define I2C_SLV0_CTRL    0x27
#define I2C_SLV1_ADDR    0x28
#define I2C_SLV1_REG     0x29
#define I2C_SLV1_CTRL    0x2A
#define I2C_SLV2_ADDR    0x2B
#define I2C_SLV2_REG     0x2C
#define I2C_SLV2_CTRL    0x2D
#define I2C_SLV3_ADDR    0x2E
#define I2C_SLV3_REG     0x2F
#define I2C_SLV3_CTRL    0x30
#define I2C_SLV4_ADDR    0x31
#define I2C_SLV4_REG     0x32
#define I2C_SLV4_DO      0x33
#define I2C_SLV4_CTRL    0x34
#define I2C_SLV4_DI      0x35
#define I2C_MST_STATUS   0x36
#define INT_PIN_CFG      0x37
#define INT_ENABLE       0x38
#define DMP_INT_STATUS   0x39  // Check DMP interrupt
#define INT_STATUS       0x3A
#define ACCEL_XOUT_H     0x3B
#define ACCEL_XOUT_L     0x3C
#define ACCEL_YOUT_H     0x3D
#define ACCEL_YOUT_L     0x3E
#define ACCEL_ZOUT_H     0x3F
#define ACCEL_ZOUT_L     0x40
#define TEMP_OUT_H       0x41
#define TEMP_OUT_L       0x42
#define GYRO_XOUT_H      0x43
#define GYRO_XOUT_L      0x44
#define GYRO_YOUT_H      0x45
#define GYRO_YOUT_L      0x46
#define GYRO_ZOUT_H      0x47
#define GYRO_ZOUT_L      0x48
#define EXT_SENS_DATA_00 0x49
#define EXT_SENS_DATA_01 0x4A
#define EXT_SENS_DATA_02 0x4B
#define EXT_SENS_DATA_03 0x4C
#define EXT_SENS_DATA_04 0x4D
#define EXT_SENS_DATA_05 0x4E
#define EXT_SENS_DATA_06 0x4F
#define EXT_SENS_DATA_07 0x50
#define EXT_SENS_DATA_08 0x51
#define EXT_SENS_DATA_09 0x52
#define EXT_SENS_DATA_10 0x53
#define EXT_SENS_DATA_11 0x54
#define EXT_SENS_DATA_12 0x55
#define EXT_SENS_DATA_13 0x56
#define EXT_SENS_DATA_14 0x57
#define EXT_SENS_DATA_15 0x58
#define EXT_SENS_DATA_16 0x59
#define EXT_SENS_DATA_17 0x5A
#define EXT_SENS_DATA_18 0x5B
#define EXT_SENS_DATA_19 0x5C
#define EXT_SENS_DATA_20 0x5D
#define EXT_SENS_DATA_21 0x5E
#define EXT_SENS_DATA_22 0x5F
#define EXT_SENS_DATA_23 0x60
#define MOT_DETECT_STATUS 0x61
#define I2C_SLV0_DO      0x63
#define I2C_SLV1_DO      0x64
#define I2C_SLV2_DO      0x65
#define I2C_SLV3_DO      0x66
#define I2C_MST_DELAY_CTRL 0x67
#define SIGNAL_PATH_RESET  0x68
#define MOT_DETECT_CTRL   0x69
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
#define DMP_BANK         0x6D  // Activates a specific bank in the DMP
#define DMP_RW_PNT       0x6E  // Set read/write pointer to a specific start address in specified DMP bank
#define DMP_REG          0x6F  // Register in DMP from which to read or to which to write
#define DMP_REG_1        0x70
#define DMP_REG_2        0x71
#define FIFO_COUNTH      0x72
#define FIFO_COUNTL      0x73
#define FIFO_R_W         0x74
#define WHO_AM_I_MPU9150 0x75 // Should return 0x68

#define MPU9150_ADDRESS 0x68<<1  // Device address when ADO = 0

// scaling parameters
typedef enum  {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
} Ascale;

typedef enum  {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
} Gscale;

class MPU9150{
public:
	MPU9150(PinName sda, PinName scl)  : _mpu(sda, scl, MPU9150_ADDRESS){//,_ak(sda, scl, AK8975A_ADDRESS){ //- no ak sensor present (magnetometer)
		uint8_t data;
		int rc = _mpu.read_reg(WHO_AM_I_MPU9150,&data);
		/*
		if(data==0x68){
			printf("correct device\n");
		}else{
			printf("device is incorrect whoiam %d\n",data);
		}*/

		rc += _mpu.write_reg(PWR_MGMT_1,0x80); //set sleep bit to zero, reset
		rc += _mpu.write_reg(PWR_MGMT_1, 0x01);  // Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
		rc += _mpu.write_reg(CONFIG, 0x03);
		rc += _mpu.write_reg(SMPLRT_DIV, 0x04);
		rc += _mpu.write_reg(PWR_MGMT_1, 0x01);
		rc += _mpu.write_reg(PWR_MGMT_2, 0x00);
		wait(0.2);

		// Configure device for bias calculation
		rc += _mpu.write_reg(INT_ENABLE, 0x00);   // Disable all interrupts
		rc += _mpu.write_reg(FIFO_EN, 0x00);      // Disable FIFO
		rc += _mpu.write_reg(PWR_MGMT_1, 0x00);   // Turn on internal clock source
		rc += _mpu.write_reg(I2C_MST_CTRL, 0x00); // Disable I2C master
		rc += _mpu.write_reg(USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
		rc += _mpu.write_reg(USER_CTRL, 0x0C);    // Reset FIFO and DMP
		wait(0.015);

		// Configure MPU9150 gyro and accelerometer for bias calculation
		rc += _mpu.write_reg(CONFIG, 0x01);      // Set low-pass filter to 188 Hz
		rc += _mpu.write_reg(SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
		rc += _mpu.read_reg(GYRO_CONFIG,&data);
		rc += _mpu.write_reg(GYRO_CONFIG, data & ~0xF8); // Clear AFS and self-test bits [8:3]
		rc += _mpu.read_reg(ACCEL_CONFIG,&data);
		rc += _mpu.write_reg(ACCEL_CONFIG, data & ~0xF8); // Clear AFS and self-test bits [8:3]
		rc += _mpu.write_reg( INT_PIN_CFG, 0x22);
		rc += _mpu.write_reg(INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
		rc += _mpu.write_reg( PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
		wait(0.1);

		// get stable time source
		// Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
		rc += _mpu.write_reg( PWR_MGMT_1, 0x01);
		rc += _mpu.write_reg( PWR_MGMT_2, 0x00);
		wait(0.2);

		// Configure device for bias calculation
		rc += _mpu.write_reg( INT_ENABLE, 0x00);   // Disable all interrupts
		rc += _mpu.write_reg( FIFO_EN, 0x00);      // Disable FIFO
		rc += _mpu.write_reg( PWR_MGMT_1, 0x00);   // Turn on internal clock source
		rc += _mpu.write_reg( I2C_MST_CTRL, 0x00); // Disable I2C master
		rc += _mpu.write_reg( USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
		rc += _mpu.write_reg( USER_CTRL, 0x0C);    // Reset FIFO and DMP
		wait(0.015);

		// Configure MPU9150 gyro and accelerometer for bias calculation
		rc += _mpu.write_reg( CONFIG, 0x01);      // Set low-pass filter to 188 Hz
		rc += _mpu.write_reg( SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
		//set PLL
		set_Ascale(_Ascale);
		set_Gscale(_Gscale);
/* no magnetometer present
		rc += _ak.write_reg(AK8975A_CNTL, 0x00);
		rc += _ak.write_reg(AK8975A_CNTL, 0x00); // Power down
		wait(0.01);
		rc += _ak.write_reg(AK8975A_CNTL, 0x0F); // Enter Fuse ROM access mode
		wait(0.01);
		uint8_t rawData[3];
		rc += _ak.read_reg(AK8975A_ASAX, &rawData[0], 3);  // Read the x-, y-, and z-axis calibration values
		rc += _mpu.write_reg(INT_PIN_CFG, 0x02); //set i2c bypass enable pin to true to access magnetometer
		rc += _mpu.write_reg(INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
		_ak.search();
*/
		if(rc){
			printf("some error during init\n");
		}

	}

	int update_motion(){
		uint8_t buffer[14];
		int rc = _mpu.read_reg(ACCEL_XOUT_H,buffer,14);
		int16_t ax_d=(((int16_t)buffer[0]) << 8) | buffer[1];
		int16_t ay_d = (((int16_t)buffer[2]) << 8) | buffer[3];
		int16_t az_d = (((int16_t)buffer[4]) << 8) | buffer[5];
		int16_t temp = (((int16_t)buffer[6]) << 8) | buffer[7];
		int16_t gx_d = (((int16_t)buffer[8]) << 8) | buffer[9];
		int16_t gy_d = (((int16_t)buffer[10]) << 8) | buffer[11];
		int16_t gz_d = (((int16_t)buffer[12]) << 8) | buffer[13];
		ax=(float)ax_d*_Ascalings[_Ascale];
		ay=(float)ay_d*_Ascalings[_Ascale];
		az=(float)az_d*_Ascalings[_Ascale];
		gx=(float)gx_d*_Gscalings[_Gscale];
		gy=(float)gy_d*_Gscalings[_Gscale];
		gz=(float)gz_d*_Gscalings[_Gscale];
		t=(float)temp/340.0+35.0;
		return rc;
	}

	int update_mag(){
		return -1;	//we use 6050 - there is no magnetometer
		/*
		bool data_ready=0;

		int rc = _mpu.write_reg(INT_PIN_CFG, 0x02);
		Thread::wait(10);
		rc += _ak.write_reg(AK8975A_CNTL, 0x01);
		Thread::wait(10);
		rc +=_ak.read_bit(AK8975A_ST1,1,&data_ready);

		if(rc){
			printf("communication error\n");
			return rc;
		}
		if(data_ready){
			uint8_t data[6];
			int16_t out[3];
			rc = _ak.read_reg(AK8975A_XOUT_L, data, 6);
			out[0] = ((int16_t)data[1] << 8) | data[0] ;  // Turn the MSB and LSB into a signed 16-bit value
			out[1] = ((int16_t)data[3] << 8) | data[2] ;
			out[2] = ((int16_t)data[5] << 8) | data[4] ;
			printf("data ready %d %d %d\n",out[0],out[1],out[2]);
			return rc;
		}else{
			printf("data not ready\n");
		}
		return -2;*/
		/*
	    _mpu.read_reg(MPU9150_RA_MAG_XOUT_L, buffer, 6);
	    int16_t mx_d = (((int16_t)buffer[0]) << 8) | buffer[1];
	    int16_t my_d = (((int16_t)buffer[2]) << 8) | buffer[3];
	    int16_t mz_d = (((int16_t)buffer[4]) << 8) | buffer[5];*/
	}

	void set_Ascale(Ascale scale){
		_Ascale = scale;
		uint8_t c;
		_mpu.read_reg(ACCEL_CONFIG,&c);	//read current value
		c = c & (~(0x03<<3));	//mask it
		_mpu.write_reg(ACCEL_CONFIG,c | scale << 3);	//and set with right scale
	}

	void set_Gscale(Gscale scale){
		_Gscale = scale;
		uint8_t c;
		_mpu.read_reg(GYRO_CONFIG,&c);
		c = c & (~(0x03<<3));
		_mpu.write_reg(GYRO_CONFIG,c | scale << 3);
	}

	float ax,ay,az,gx,gy,gz,mx,my,mz,t;

protected:

	Ascale _Ascale = AFS_2G;
	Gscale _Gscale = GFS_2000DPS;

	const float _Gscalings[4]={250.0/32768.0,500.0/32768.0,1000.0/32768.0,2000.0/32768.0};	//scaling constants
	const float _Ascalings[4]={2.0/32768.0, 4.0/32768.0, 8.0/32768.0, 16.0/32768.0};


	void calibrate(){
		{
		  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
		  uint16_t ii, packet_count, fifo_count;
		  int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};

		// reset device, reset all registers, clear gyro and accelerometer bias registers
		  _mpu.write_reg( PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
		  wait(0.1);

		// get stable time source
		// Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
		  _mpu.write_reg( PWR_MGMT_1, 0x01);
		  _mpu.write_reg( PWR_MGMT_2, 0x00);
		  wait(0.2);

		// Configure device for bias calculation
		  _mpu.write_reg( INT_ENABLE, 0x00);   // Disable all interrupts
		  _mpu.write_reg( FIFO_EN, 0x00);      // Disable FIFO
		  _mpu.write_reg( PWR_MGMT_1, 0x00);   // Turn on internal clock source
		  _mpu.write_reg( I2C_MST_CTRL, 0x00); // Disable I2C master
		  _mpu.write_reg( USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
		  _mpu.write_reg( USER_CTRL, 0x0C);    // Reset FIFO and DMP
		  wait(0.015);

		// Configure MPU9150 gyro and accelerometer for bias calculation
		  _mpu.write_reg( CONFIG, 0x01);      // Set low-pass filter to 188 Hz
		  _mpu.write_reg( SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
		  //_mpu.write_reg( GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
		  //_mpu.write_reg( ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity

		  uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
		  uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

		// Configure FIFO to capture accelerometer and gyro data for bias calculation
		  _mpu.write_reg( USER_CTRL, 0x40);   // Enable FIFO
		  _mpu.write_reg( FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO (max size 1024 bytes in MPU9150)
		  wait(0.08); // accumulate 80 samples in 80 milliseconds = 960 bytes

		// At end of sample accumulation, turn off FIFO sensor read
		  _mpu.write_reg( FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
		  _mpu.read_reg(FIFO_COUNTH, &data[0], 2); // read FIFO sample count
		  fifo_count = ((uint16_t)data[0] << 8) | data[1];
		  packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging

		  for (ii = 0; ii < packet_count; ii++) {
		    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
		    _mpu.read_reg(FIFO_R_W, &data[0], 12); // read data for averaging
		    accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
		    accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
		    accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;
		    gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
		    gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
		    gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;

		    accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
		    accel_bias[1] += (int32_t) accel_temp[1];
		    accel_bias[2] += (int32_t) accel_temp[2];
		    gyro_bias[0]  += (int32_t) gyro_temp[0];
		    gyro_bias[1]  += (int32_t) gyro_temp[1];
		    gyro_bias[2]  += (int32_t) gyro_temp[2];

		}
		    accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
		    accel_bias[1] /= (int32_t) packet_count;
		    accel_bias[2] /= (int32_t) packet_count;
		    gyro_bias[0]  /= (int32_t) packet_count;
		    gyro_bias[1]  /= (int32_t) packet_count;
		    gyro_bias[2]  /= (int32_t) packet_count;

		  if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
		  else {accel_bias[2] += (int32_t) accelsensitivity;}

		// Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
		  data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
		  data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
		  data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
		  data[3] = (-gyro_bias[1]/4)       & 0xFF;
		  data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
		  data[5] = (-gyro_bias[2]/4)       & 0xFF;

		/// Push gyro biases to hardware registers
		/*  _mpu.write_reg( XG_OFFS_USRH, data[0]);
		  _mpu.write_reg( XG_OFFS_USRL, data[1]);
		  _mpu.write_reg( YG_OFFS_USRH, data[2]);
		  _mpu.write_reg( YG_OFFS_USRL, data[3]);
		  _mpu.write_reg( ZG_OFFS_USRH, data[4]);
		  _mpu.write_reg( ZG_OFFS_USRL, data[5]);*/
/*
		  dest1[0] = (float) gyro_bias[0]/(float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
		  dest1[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
		  dest1[2] = (float) gyro_bias[2]/(float) gyrosensitivity;
*/
		// Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
		// factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
		// non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
		// compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
		// the accelerometer biases calculated above must be divided by 8.

		  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
		  _mpu.read_reg(XA_OFFSET_H, &data[0], 2); // Read factory accelerometer trim values
		  accel_bias_reg[0] = (int16_t) ((int16_t)data[0] << 8) | data[1];
		  _mpu.read_reg(YA_OFFSET_H, &data[0], 2);
		  accel_bias_reg[1] = (int16_t) ((int16_t)data[0] << 8) | data[1];
		  _mpu.read_reg(ZA_OFFSET_H, &data[0], 2);
		  accel_bias_reg[2] = (int16_t) ((int16_t)data[0] << 8) | data[1];

		  uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
		  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis

		  for(ii = 0; ii < 3; ii++) {
		    if(accel_bias_reg[ii] & mask) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
		  }

		  // Construct total accelerometer bias, including calculated average accelerometer bias from above
		  accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
		  accel_bias_reg[1] -= (accel_bias[1]/8);
		  accel_bias_reg[2] -= (accel_bias[2]/8);

		  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
		  data[1] = (accel_bias_reg[0])      & 0xFF;
		  data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
		  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
		  data[3] = (accel_bias_reg[1])      & 0xFF;
		  data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
		  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
		  data[5] = (accel_bias_reg[2])      & 0xFF;
		  data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

		// Apparently this is not working for the acceleration biases in the MPU-9250
		// Are we handling the temperature correction bit properly?
		// Push accelerometer biases to hardware registers
/*		  _mpu.write_reg( XA_OFFSET_H, data[0]);
		  _mpu.write_reg( XA_OFFSET_L_TC, data[1]);
		  _mpu.write_reg( YA_OFFSET_H, data[2]);
		  _mpu.write_reg( YA_OFFSET_L_TC, data[3]);
		  _mpu.write_reg( ZA_OFFSET_H, data[4]);
		  _mpu.write_reg( ZA_OFFSET_L_TC, data[5]);
*/
/* 		   Output scaled accelerometer biases for manual subtraction in the main program
		   dest2[0] = (float)accel_bias[0]/(float)accelsensitivity;
		   dest2[1] = (float)accel_bias[1]/(float)accelsensitivity;
		   dest2[2] = (float)accel_bias[2]/(float)accelsensitivity;
*/
		}
	}
	I2CWrapper _mpu;
	//I2CWrapper _ak;
/*
	int _mpu.write_reg(uint8_t reg, uint8_t val){
		uint8_t cmd[2];
		cmd[0]=reg;
		cmd[1]=val;
		return _mpu.write(MPU9150_ADDRESS,(char*)cmd,2);
	};

	int _mpu.write_reg(uint8_t reg, uint8_t *val, uint16_t size){
		int rc = _mpu.write(MPU9150_ADDRESS,(char*)&reg,1,1);	//write first byte - addr
		if(rc){
			return rc;
		}
		rc = _mpu.write(MPU9150_ADDRESS,(char*)val,size);
		return rc;
	};

	int _mpu.read_reg(uint8_t reg, uint8_t *val){
		int rc = _mpu.write(MPU9150_ADDRESS,(char*)&reg,1,1);
		if(rc){
			return rc;
		}
		rc = _mpu.read(MPU9150_ADDRESS,(char*)val,1);
		return rc;
	}

	int _mpu.read_reg(uint8_t reg, uint8_t *val, uint16_t size){
		int rc = _mpu.write(MPU9150_ADDRESS,(char*)&reg,1,1);
		if(rc){
			return rc;
		}
		rc = _mpu.read(MPU9150_ADDRESS,(char*)val,size);
		return rc;
	}

	int _mpu.write_bit(uint8_t reg, uint8_t bit, bool val){
		uint8_t temp;
		int rc=_mpu.read_reg(reg,&temp);
		if(rc==0){
			return rc;
		}
		if(val){	//set bit
			rc = _mpu.write_reg(reg,temp|(1<<bit));
		}else{
			rc = _mpu.write_reg(reg,temp&(~(1<<bit)));
		}
		return rc;
	}

	int _mpu.read_bit(uint8_t reg, uint8_t bit, bool *val){
		uint8_t temp;
		int rc = _mpu.read_reg(reg,&temp);
		if(temp&(1<<bit)){
			*val=true;
		}else{
			*val=false;
		}
		return rc;
	}
*/
};


#endif /* MPU9150_H_ */
