#include "pid_controller.h"

#define LimitMax(input, max)                                                   \
	{                                                                          \
		if (input > max) {                                                     \
			input = max;                                                       \
		} else if (input < -max) {                                             \
			input = -max;                                                      \
		}                                                                      \
	}

void pid_controller_init(pid_controller_t *pid,
                         const pid_controller_param_t *param) {
	pid->param = *param;
	pid->out = 0;
	pid->p_out = 0;
	pid->i_out = 0;
	pid->d_out = 0;
	pid->error = 0;
	pid->last_error = 0;
	pid->second_last_error = 0;
}

float pid_controller_update(pid_controller_t *pid, float feedback, float set) {
	pid->set = set;
	pid->feedback = feedback;
	pid->second_last_error = pid->last_error;
	pid->last_error = pid->error;
	pid->error = set - feedback;
	if (pid->param.mode == PID_CONTROLLER_POSITION) {
		pid->p_out = pid->param.k_p * pid->error;
		pid->i_out += pid->param.k_i * pid->error;
		pid->d_out = pid->param.k_d * (pid->error - pid->last_error);
		LimitMax(pid->i_out, pid->param.integral_limit);
		pid->out = pid->p_out + pid->i_out + pid->d_out;
		LimitMax(pid->out, pid->param.out_limit);
	} else if (pid->param.mode == PID_CONTROLLER_DELTA) {
		pid->p_out = pid->param.k_p * (pid->error - pid->last_error);
		pid->i_out = pid->param.k_i * pid->error;
		pid->d_out = pid->param.k_d * (pid->error - 2.0f * pid->last_error +
		                               pid->second_last_error);
		pid->out += pid->p_out + pid->i_out + pid->d_out;
		LimitMax(pid->out, pid->param.out_limit);
	}
	return pid->out;
}

void pid_controller_reset(pid_controller_t *pid) {
	pid->out = 0;
	pid->p_out = 0;
	pid->i_out = 0;
	pid->d_out = 0;
	pid->error = 0;
	pid->last_error = 0;
	pid->second_last_error = 0;
	pid->feedback = pid->set;
}
