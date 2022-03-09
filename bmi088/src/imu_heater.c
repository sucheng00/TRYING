#include "imu_heater.h"
#include "tim.h"

void imu_heater_init(void) {
	HAL_TIM_Base_Start(&htim10);
	HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
}

void imu_heater_set_pwm(uint16_t duty) {
	__HAL_TIM_SetCompare(&htim10, TIM_CHANNEL_1, duty);
}
