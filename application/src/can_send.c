#include "can_send.h"
#include "cmsis_os.h"
#include "main.h"
#include "bsp_rng.h"
#include "imu.h"
#include "quaternion.h"
#include "struct_typedef.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

static CAN_TxHeaderTypeDef  gimbal_tx_message;
static uint8_t              gimbal_can_send_data[8];

extern float yaw, pitch, roll;

typedef struct
{
    float yaw;
    float pitch;
}__attribute__((packed)) can_yaw_pitch_t;

typedef struct
{
    float roll;
    // bool_t microsw;
}__attribute__((packed)) can_roll_microsw_t;


void CAN_cmd_send_yaw_pitch(void)
{
    can_yaw_pitch_t p;
    p.yaw = yaw;
    p.pitch = pitch;
    void *buf = (void*) &p;
    //send(buf, sizeof(packet_yaw_pitch));

    uint32_t send_mail_box;
    gimbal_tx_message.StdId = CAN2_ID_A;
    gimbal_tx_message.IDE = CAN_ID_STD;
    gimbal_tx_message.RTR = CAN_RTR_DATA;
    gimbal_tx_message.DLC = 0x08;
    gimbal_can_send_data[0] = *(char *)buf++; //yaw
    gimbal_can_send_data[1] = *(char *)buf++;
    gimbal_can_send_data[2] = *(char *)buf++;
    gimbal_can_send_data[3] = *(char *)buf++;
    gimbal_can_send_data[4] = *(char *)buf++; //pitch
    gimbal_can_send_data[5] = *(char *)buf++;
    gimbal_can_send_data[6] = *(char *)buf++;
    gimbal_can_send_data[7] = *(char *)buf;
    // gimbal_can_send_data[0] = *(int *)buf>>(8*7); //yaw
    // gimbal_can_send_data[1] = *(int *)buf>>(8*6);
    // gimbal_can_send_data[2] = *(int *)buf>>(8*5);
    // gimbal_can_send_data[3] = *(int *)buf>>(8*4);
    // gimbal_can_send_data[4] = *(int *)buf>>(8*3); //pitch
    // gimbal_can_send_data[5] = *(int *)buf>>(8*2);
    // gimbal_can_send_data[6] = *(int *)buf>>(8*1);
    // gimbal_can_send_data[7] = *(int *)buf>>(8*0);
    HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
} 
extern bool_t micro_sw;
void CAN_cmd_send_roll_microsw(void)
{
  can_roll_microsw_t p;
  p.roll = roll;
//   p.microsw = micro_sw;
  void *buf = (void*) &p;
  // send(buf, sizeof(packet_yaw_pitch));

  uint32_t send_mail_box;
  gimbal_tx_message.StdId = CAN2_ID_B;
  gimbal_tx_message.IDE = CAN_ID_STD;
  gimbal_tx_message.RTR = CAN_RTR_DATA;
  gimbal_tx_message.DLC = 0x08;
  gimbal_can_send_data[0] = *(char *)buf++; //roll
  gimbal_can_send_data[1] = *(char *)buf++;
  gimbal_can_send_data[2] = *(char *)buf++;
  gimbal_can_send_data[3] = *(char *)buf++;
  gimbal_can_send_data[4] = (uint8_t)micro_sw;//*buf>>(8*0); //microsw
  gimbal_can_send_data[5] = 0;
  gimbal_can_send_data[6] = 0;
  gimbal_can_send_data[7] = 0;
  HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
} 

