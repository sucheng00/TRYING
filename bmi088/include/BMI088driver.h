#pragma once

#include <stdbool.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef struct BMI088_RAW_DATA {
	uint8_t status;
	int16_t accel[3];
	int16_t temp;
	int16_t gyro[3];
} bmi088_raw_data_t;

#pragma pack(pop)

typedef struct BMI088_REAL_DATA {
	uint8_t status;
	float accel[3];
	float temp;
	float gyro[3];
	float time;
} bmi088_real_data_t;

#define BMI088_GYRO_DATA_READY_BIT 0
#define BMI088_ACCEL_DATA_READY_BIT 1
#define BMI088_ACCEL_TEMP_DATA_READY_BIT 2

#define BMI088_ACCEL_RANGE_3G
//#define BMI088_ACCEL_RANGE_6G
//#define BMI088_ACCEL_RANGE_12G
//#define BMI088_ACCEL_RANGE_24G

#define BMI088_GYRO_RANGE_2000
//#define BMI088_GYRO_RANGE_1000
//#define BMI088_GYRO_RANGE_500
//#define BMI088_GYRO_RANGE_250
//#define BMI088_GYRO_RANGE_125

#define BMI088_ACCEL_3G_SEN 0.0008974358974f
#define BMI088_ACCEL_6G_SEN 0.00179443359375f
#define BMI088_ACCEL_12G_SEN 0.0035888671875f
#define BMI088_ACCEL_24G_SEN 0.007177734375f

#define BMI088_GYRO_2000_SEN 0.00106526443603169529841533860381f
#define BMI088_GYRO_1000_SEN 0.00053263221801584764920766930190693f
#define BMI088_GYRO_500_SEN 0.00026631610900792382460383465095346f
#define BMI088_GYRO_250_SEN 0.00013315805450396191230191732547673f
#define BMI088_GYRO_125_SEN 0.000066579027251980956150958662738366f

enum {
	BMI088_NO_Error = 0x00,
	BMI088_ACC_PWR_CTRL_Error = 0x01,
	BMI088_ACC_PWR_CONF_Error = 0x02,
	BMI088_ACC_CONF_Error = 0x03,
	BMI088_ACC_SELF_TEST_Error = 0x04,
	BMI088_ACC_RANGE_Error = 0x05,
	BMI088_INT1_IO_CTRL_Error = 0x06,
	BMI088_INT_MAP_DATA_Error = 0x07,
	BMI088_GYRO_RANGE_Error = 0x08,
	BMI088_GYRO_BANDWIDTH_Error = 0x09,
	BMI088_GYRO_LPM1_Error = 0x0A,
	BMI088_GYRO_CTRL_Error = 0x0B,
	BMI088_GYRO_INT3_INT4_IO_CONF_Error = 0x0C,
	BMI088_GYRO_INT3_INT4_IO_MAP_Error = 0x0D,

	BMI088_SELF_TEST_ACCEL_Error = 0x80,
	BMI088_SELF_TEST_GYRO_Error = 0x40,
	BMI088_NO_Sensor = 0xFF,
};

#define BMI088_LONG_DELAY_TIME 80
#define BMI088_COM_WAIT_SENSOR_TIME 150

uint8_t BMI088_Init(void);
bool bmi088_accel_self_test(void);
bool bmi088_gyro_self_test(void);
bool bmi088_accel_init(void);
bool bmi088_gyro_init(void);

void BMI088_accel_read_over(uint8_t *rx_buf, float accel[3], float *time);
void BMI088_gyro_read_over(uint8_t *rx_buf, float gyro[3]);
void BMI088_temperature_read_over(uint8_t *rx_buf, float *temperate);
void BMI088_Read(float gyro[3], float accel[3], float *temperate);
uint32_t get_BMI088_sensor_time(void);
float get_BMI088_temperate(void);
void get_BMI088_gyro(int16_t gyro[3]);
void get_BMI088_accel(float accel[3]);

void BMI088_read_gyro_who_am_i(void);
uint8_t BMI088_read_accel_who_am_i(void);
