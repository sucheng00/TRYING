#pragma once

#include "pid_controller.h"
#include "quaternion.h"
#include <stdbool.h>

/**
 * IMU 校准数据
 */
typedef struct {
	float gx;
	float gy;
	float gz;
} imu_calibration_t;

typedef struct {
	/* Epoch 纳秒时间戳，需要 micro-ROS 初始化后才有，否则为 0 */
	int64_t epoch_ns;
	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;
	quaternion_t orientation;
	float temperature;
} imu_data_t;

/**
 * 读取 IMU 数据
 */
void imu_get_data(imu_data_t *result);

/**
 * 从 Flash 读取 IMU 校准数据，若未校准过则为 false
 */
bool imu_read_stored_calibration(imu_calibration_t *result);

/**
 * 将 IMU 校准数据并写入 Flash，若传入 NULL 则擦除
 */
void imu_write_stored_calibration(const imu_calibration_t *result);

void imu_task(const void *arg);
