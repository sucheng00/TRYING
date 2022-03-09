#pragma once

#include "quaternion.h"

void madgwick_update_9d(quaternion_t *q, float beta, float dt, float gx,
                        float gy, float gz, float ax, float ay, float az,
                        float mx, float my, float mz);

void madgwick_update_6d(quaternion_t *q, float beta, float dt, float gx,
                        float gy, float gz, float ax, float ay, float az);
