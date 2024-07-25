/*
 * mpu6050.c
 *
 *  Created on: Jul 2, 2024
 *      Author: rusta
 */


#include "mpu6050.h"
#include "i2c.h"
#include <stdio.h>

static float LSB_SENS_ACCEL;
static float LSB_SENS_GYRO;

struct_mpu6050 mpu6050;

void mpu6050_ReadBytes(uint8_t reg_addr, uint8_t len, uint8_t* data) {
	HAL_I2C_Mem_Read(&hi2c1, mpu6050_DEV_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1);
}

void mpu6050_WriteBytes(uint8_t reg_addr, uint8_t len, uint8_t* data) {
	HAL_I2C_Mem_Write(&hi2c1, mpu6050_DEV_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1);
}

void mpu6050_Init(void) {
	HAL_Delay(50);
	uint8_t who_am_i = 0;
	printf("Verifying MPU6050 device...\n");

	mpu6050_ReadBytes(mpu6050_WHO_AM_I, 1, &who_am_i);
	HAL_Delay(50);

	if(who_am_i == mpu6050_DEV_ADDR)
	{
		printf("Device verified. Proceeding with config...\n");
	}
	else
	{
		printf("Configuration error\n");
		while(1)
		{
			printf("MPU6050 not recognized.\n");
			HAL_Delay(100);
		}
	}

	// reset the device
	mpu6050_WriteBytes(mpu6050_PWR_MGMT_1, 1, (uint8_t*)(0x1<<7));
	HAL_Delay(100);

	// wake up from sleep mode
	mpu6050_WriteBytes(mpu6050_PWR_MGMT_1, 1, 0x0);
	HAL_Delay(50);

	// configure sample rate divider
	// Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
	// Gyroscope Output Rate = 8kHz
	// set sample rate to 200HZ
	mpu6050_WriteBytes(mpu6050_SMPLRT_DIV, 1, (uint8_t*)(0x27));
	HAL_Delay(50);

	mpu6050_WriteBytes(mpu6050_CONFIG, 1, (uint8_t*)(0x0));
	HAL_Delay(50);

	// configure accelerometer
	uint8_t AFS_SEL = 0x0; // full scale range, +-2g/s
	mpu6050_WriteBytes(mpu6050_ACCEL_CONFIG, 1, (uint8_t*)(AFS_SEL << 3));
	HAL_Delay(50);

	// configure gyroscope
	uint8_t FS_SEL = 0x0; // +-250 degrees/s
	mpu6050_WriteBytes(mpu6050_GYRO_CONFIG, 1, (uint8_t*)(FS_SEL << 3));
	HAL_Delay(50);

	get_lsb_sens(FS_SEL, AFS_SEL);

	// configure interrupt pin
	// set INT_RD_CLEAR = 1 such that interrupt flag is cleared on any read operation
//	mpu6050_WriteBytes(mpu6050_INT_PIN_CFG, 1, (uint8_t*)(0x1 << 4));
//	HAL_Delay(50);
//
//	// enable Data Ready interrupt
//	mpu6050_WriteBytes(mpu6050_INT_ENABLE, 1, (uint8_t*)(0x1));
//	HAL_Delay(50);

	uint8_t INT_LEVEL = 0x0; //0 - active high, 1 - active low
	uint8_t LATCH_INT_EN = 0x0; //0 - INT 50us pulse, 1 - interrupt clear required
	uint8_t INT_RD_CLEAR = 0x1; //0 - INT flag cleared by reading INT_STATUS, 1 - INT flag cleared by any read operation
	mpu6050_WriteBytes(mpu6050_INT_PIN_CFG, 1, (INT_LEVEL<<7)|(LATCH_INT_EN<<5)|(INT_RD_CLEAR<<4)); //
	HAL_Delay(50);

	//Interrupt enable setting
	uint8_t DATA_RDY_EN = 0x1; // 1 - enable, 0 - disable
	mpu6050_WriteBytes(mpu6050_INT_ENABLE, 1, DATA_RDY_EN);
	HAL_Delay(50);

	printf("MPU6050 configuration completed.\n");
}

int mpu6050_DataReady(void) {
	return HAL_GPIO_ReadPin(mpu6050_INT_PORT, mpu6050_INT_PIN);
}

void mpu6050_GetData(struct_mpu6050 *mpu6050) {
	get_raw_data(mpu6050);
	convert_raw_data(mpu6050);
}

//
void get_lsb_sens(uint8_t FS_SEL, uint8_t AFS_SEL) {
	switch(FS_SEL)
	{
	case 0:
		LSB_SENS_GYRO = 131.0f;
		break;
	case 1:
		LSB_SENS_GYRO = 65.5f;
		break;
	case 2:
		LSB_SENS_GYRO = 32.8f;
		break;
	case 3:
		LSB_SENS_GYRO = 16.4f;
		break;
	}
	switch(AFS_SEL)
	{
	case 0:
		LSB_SENS_ACCEL = 16384.0f;
		break;
	case 1:
		LSB_SENS_ACCEL = 8192.0f;
		break;
	case 2:
		LSB_SENS_ACCEL = 4096.0f;
		break;
	case 3:
		LSB_SENS_ACCEL = 2048.0f;
		break;
	}
}

void get_raw_data(struct_mpu6050 *mpu6050) {
	uint8_t raw_data[14];
	mpu6050_ReadBytes(mpu6050_ACCEL_XOUT_H, 14, raw_data);

	mpu6050->accel_x_raw = (raw_data[0] << 8) | raw_data[1];
	mpu6050->accel_y_raw = (raw_data[2] << 8) | raw_data[3];
	mpu6050->accel_z_raw = (raw_data[4] << 8) | raw_data[5];

	mpu6050->temp_raw = (raw_data[6] << 8) | raw_data[7];

	mpu6050->gyro_x_raw = (raw_data[8] << 8) | raw_data[9];
	mpu6050->gyro_y_raw = (raw_data[10] << 8) | raw_data[11];
	mpu6050->gyro_z_raw = (raw_data[12] << 8) | raw_data[13];
}

void convert_raw_data(struct_mpu6050 *mpu6050) {

	// acceleration = accelerometer raw data / LSB sensitivity

	mpu6050->accel_x = mpu6050->accel_x_raw / LSB_SENS_ACCEL;
	mpu6050->accel_y = mpu6050->accel_y_raw / LSB_SENS_ACCEL;
	mpu6050->accel_z = mpu6050->accel_z_raw / LSB_SENS_ACCEL;

	// from datasheet
	mpu6050->temp = (float) (mpu6050->temp_raw / 340 + 36.53);

	// angular velocity = gyroscope raw data / LSB sensitivity
	mpu6050->gyro_x = mpu6050->gyro_x_raw / LSB_SENS_GYRO;
	mpu6050->gyro_y = mpu6050->gyro_y_raw / LSB_SENS_GYRO;
	mpu6050->gyro_z = mpu6050->gyro_z_raw / LSB_SENS_GYRO;
}

