#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	PID_CONTROLLER_POSITION,
	PID_CONTROLLER_DELTA
} pid_controller_mode_t;

typedef struct {
	pid_controller_mode_t mode;
	float k_p;
	float k_i;
	float k_d;
	float integral_limit;
	float out_limit;
} pid_controller_param_t;

typedef struct {
	pid_controller_param_t param;

	float set;
	float feedback;

	float error;
	float last_error;
	float second_last_error;

	float p_out;
	float i_out;
	float d_out;
	float out;
} pid_controller_t;

void pid_controller_init(pid_controller_t *pid,
                         const pid_controller_param_t *param);

float pid_controller_update(pid_controller_t *pid, float feedback, float set);

void pid_controller_reset(pid_controller_t *pid);
