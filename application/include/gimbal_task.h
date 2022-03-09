/**
  * @brief      完成对两个2305电机的输出控制、 微动开关的读取
  */
#pragma once

#include "main.h"
#include <cmsis_os.h>
#include "bsp_fric.h"
#include "can_send.h"
#include "can_receive.h"
#include "imu.h"
#include "quaternion.h"
#include "struct_typedef.h"
#include <stdint.h>

//任务初始化 空闲一段时间
#define GIMBAL_TASK_INIT_TIME 201
#define GIMBAL_CONTROL_TIME 1
#define FRIC_CONTROL_TIME  1
#define FRIC_PWM_ADD_VALUE    100.0f
#define FRIC_DUTY_MAX 10000
#define FRIC_DUTY_MIN 0
#define FRIC_MAX 1300
#define FRIC_MIN 1000
#define SHOOT_FRIC_PWM_ADD_VALUE  100.0f

typedef __packed struct
{
    float input;        //输入数据
    float out;          //输出数据
    float min_value;  //限幅最小值
    float frame_period;  //时间间隔
    float max_value;    //限幅最大值 
} ramp_function_source_t;

void fric_control_loop(void);

void fric_init(void);

void ramp_init(ramp_function_source_t *ramp_source_type, float frame_period, float max, float min);

void ramp_calc(ramp_function_source_t *ramp_source_type, float input);

void micro_sw_update(void);
