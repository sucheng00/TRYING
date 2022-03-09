#pragma once

typedef struct {
	float w;
	float x;
	float y;
	float z;
} quaternion_t;

void quaternion_to_rpy(const quaternion_t *q, float *yaw, float *pitch,
                       float *roll);
