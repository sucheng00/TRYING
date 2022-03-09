#include "quaternion.h"
#include <math.h>

void quaternion_to_rpy(const quaternion_t *q, float *yaw, float *pitch,
                       float *roll) {
	*roll = atan2f(q->w * q->x + q->y * q->z, 0.5f - q->x * q->x - q->y * q->y);
	*pitch = asinf(-2.0f * (q->x * q->z - q->w * q->y));
	*yaw = atan2f(q->x * q->y + q->w * q->z, 0.5f - q->y * q->y - q->z * q->z);
}
