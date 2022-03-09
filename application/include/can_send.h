#pragma once

#include "main.h"
#include "imu.h"
#include "quaternion.h"
#include <stdint.h>

#define GIMBAL_CAN hcan2

typedef unsigned char bool_t;

/* CAN send and receive ID */
typedef enum
{
  CAN2_ID_A = 0x100, 
  CAN2_ID_B = 0x101, 
  CAN2_ID_C = 0x102, 
} can_msg_id_e;

extern void CAN_cmd_send_angle(void);

