#pragma once

#include <stdint.h>

void imu_heater_init(void);

void imu_heater_set_pwm(uint16_t duty);
