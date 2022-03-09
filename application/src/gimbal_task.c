/*
 * @brief      完成对两个2305电机的输出控制、 微动开关的读取
 */
#include "gimbal_task.h"
#include "math.h"

#define shoot_fric_off() fric_off()           

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t gimbal_high_water;
#endif

ramp_function_source_t fric1_ramp,fric2_ramp;
void gimbal_task(void const *pvParameters)
{
  vTaskDelay(GIMBAL_TASK_INIT_TIME);
  fric_init();
  while (1)
  {
    fric_control_loop();
    micro_sw_update();


#if GIMBAL_TEST_MODE
    J_scope_gimbal_test();
#endif
    vTaskDelay(GIMBAL_CONTROL_TIME);

#if INCLUDE_uxTaskGetStackHighWaterMark
    gimbal_high_water = uxTaskGetStackHighWaterMark(NULL);
#endif
  }
}

extern uint16_t fric_pwm1,fric_pwm2;
void fric_control_loop(void)
{
  if(fric_pwm1 && fric_pwm2){
    ramp_calc(&fric1_ramp, SHOOT_FRIC_PWM_ADD_VALUE);
    ramp_calc(&fric2_ramp, SHOOT_FRIC_PWM_ADD_VALUE);
  }
  else{
    ramp_calc(&fric1_ramp, -SHOOT_FRIC_PWM_ADD_VALUE);
    ramp_calc(&fric2_ramp, -SHOOT_FRIC_PWM_ADD_VALUE);
  }
  //需打开定时器
    fric1_on(fric_pwm1);
    fric2_on(fric_pwm2);
}

void fric_init(void)
{
    fric_pwm1 = FRIC_OFF;
    fric_pwm2 = FRIC_OFF;
    ramp_init(&fric1_ramp,FRIC_CONTROL_TIME * 0.001f, FRIC_MAX, FRIC_MIN);
    ramp_init(&fric2_ramp,FRIC_CONTROL_TIME * 0.001f, FRIC_MAX, FRIC_MIN);
}

bool_t micro_sw;
void micro_sw_update(void){
  micro_sw = HAL_GPIO_ReadPin(BUTTON_TRIG_GPIO_Port, BUTTON_TRIG_Pin);
}


/** @brief          斜波函数初始化
  ********          斜波struct 间隔时间(s) MAX MIN
  */
void ramp_init(ramp_function_source_t *ramp_source_type, float frame_period, float max, float min)
{
    ramp_source_type->frame_period = frame_period;
    ramp_source_type->max_value = max;
    ramp_source_type->min_value = min;
    ramp_source_type->input = 0.0f;
    ramp_source_type->out = 0.0f;
}


/** @brief          斜波计算,根据input叠加, s 即一秒后增加输入的值
  ********          斜波函数结构体 输入值 滤波参数
  */
void ramp_calc(ramp_function_source_t *ramp_source_type, float input)
{
    ramp_source_type->input = input;
    ramp_source_type->out += ramp_source_type->input * ramp_source_type->frame_period;
    if (ramp_source_type->out > ramp_source_type->max_value)
    {
        ramp_source_type->out = ramp_source_type->max_value;
    }
    else if (ramp_source_type->out < ramp_source_type->min_value)
    {
        ramp_source_type->out = ramp_source_type->min_value;
    }
}
