/*
 * gyro521.h
 *
 *  Created on: Jul 1, 2024
 *      Author: rusta
 */

#ifndef mpu6050_H_
#define mpu6050_H_

#include "main.h"

#define mpu6050_CONFIG 0x1A
#define mpu6050_SMPLRT_DIV 0x19
#define mpu6050_GYRO_CONFIG 0x1B
#define mpu6050_ACCEL_CONFIG 0x1C
#define mpu6050_INT_PIN_CFG 0x37
#define mpu6050_INT_ENABLE 0x38
#define mpu6050_INT_STATUS 0x3A
#define mpu6050_ACCEL_XOUT_H 0x3B // (14 bytes of sensor data starting from this address)
#define mpu6050_PWR_MGMT_1 0x6B
#define mpu6050_WHO_AM_I 0x75
#define mpu6050_DEV_ADDR 0x68

#define mpu6050_INT_PORT 	GPIOB
#define mpu6050_INT_PIN 	GPIO_PIN_5

// used to access the sensor data
typedef struct mpu6050_t {
	short accel_x_raw;
	short accel_y_raw;
	short accel_z_raw;
	short temp_raw;
	short gyro_x_raw;
	short gyro_y_raw;
	short gyro_z_raw;

	float accel_x;
	float accel_y;
	float accel_z;
	float temp;
	float gyro_x;
	float gyro_y;
	float gyro_z;

	float gyro_x_err;
	float gyro_y_err;
	float gyro_z_err;
} struct_mpu6050;

extern struct_mpu6050 mpu6050;

void mpu6050_Init(void);
void mpu6050_ReadBytes(uint8_t reg_addr, uint8_t len, uint8_t* data);
void mpu6050_WriteBytes(uint8_t reg_addr, uint8_t len, uint8_t* data);
void mpu6050_GetData(struct_mpu6050*);
int mpu6050_DataReady();
void get_raw_data(struct_mpu6050*);
void convert_raw_data(struct_mpu6050*);
void get_lsb_sens(uint8_t ACC_SEL, uint8_t GYRO_SEL);


#endif /* mpu6050_H_ */
