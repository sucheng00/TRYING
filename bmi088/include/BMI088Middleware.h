#pragma once

#include <stdint.h>

void BMI088_GPIO_Init(void);
void BMI088_Com_Init(void);
void BMI088_Delay_ms(uint16_t ms);
void BMI088_Delay_us(uint16_t us);

void BMI088_ACCEL_NS_L(void);
void BMI088_ACCEL_NS_H(void);

void BMI088_GYRO_NS_L(void);
void BMI088_GYRO_NS_H(void);

uint8_t BMI088_Read_Write_Byte(uint8_t reg);
