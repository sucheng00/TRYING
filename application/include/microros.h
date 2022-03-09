#pragma once

#include "imu.h"

bool microros_publish_imu_data(const imu_data_t *imu);

void microros_task(const void *arg);
