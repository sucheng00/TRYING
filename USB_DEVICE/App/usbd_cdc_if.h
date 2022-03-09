/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.h
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_cdc_if.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

/* USER CODE BEGIN INCLUDE */
#include <rmw_microros/custom_transport.h>
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief For Usb device.
  * @{
  */

/** @defgroup USBD_CDC_IF USBD_CDC_IF
  * @brief Usb VCP device module
  * @{
  */

/** @defgroup USBD_CDC_IF_Exported_Defines USBD_CDC_IF_Exported_Defines
  * @brief Defines.
  * @{
  */

/* Define size for the receive and transmit buffer over CDC */
// USB FS 下，单个包最大为 64 字节
#define APP_RX_DATA_SIZE 64
#define APP_TX_DATA_SIZE 64

/* USER CODE BEGIN EXPORTED_DEFINES */

// 指示环形缓冲区发生过溢出，导致旧数据被覆盖的错误标志位
#define USB_ERR_RINGBUF_OVERFLOW 0x1

// 指示 USB 发生过底层错误（HAL 返回错误）的错误标志位
#define USB_ERR_USBD_ERROR 0x2

// 发送/接收环形缓冲区的大小（有 1 个字节保留不可用）
#define TX_RINGBUF_SIZE 2048
#define RX_RINGBUF_SIZE 2048

/* USER CODE END EXPORTED_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Types USBD_CDC_IF_Exported_Types
  * @brief Types.
  * @{
  */

/* USER CODE BEGIN EXPORTED_TYPES */

/* USER CODE END EXPORTED_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Macros USBD_CDC_IF_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

/** CDC Interface callback. */
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_FunctionsPrototype USBD_CDC_IF_Exported_FunctionsPrototype
  * @brief Public functions declaration.
  * @{
  */

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

/* USER CODE BEGIN EXPORTED_FUNCTIONS */

/**
 * 检测到自上次成功接收到数据后，上位机程序是否发生过重启。
 * @returns true 如果自上次成功接收到数据后，上位机程序发生过重启
 */
bool check_host_restart(void);

// XRCE-DDS 通信驱动函数

/**
 * 打开 USB 信道，此操作会清空缓冲区，并清除错误状态。
 * @param transport 未使用，请传 NULL
 * @returns true 操作始终成功
 */
bool usb_transport_open(uxrCustomTransport *transport);

/**
 * 关闭 USB 信道。
 * @param transport 未使用，请传 NULL
 * @returns true 操作始终成功
 */
bool usb_transport_close(uxrCustomTransport *transport);

/**
 * 将 buf 中 len 个字节的数据写入 USB 发送缓冲区。
 * 若当前 USB 处于空闲状态，则会立即启动发送；
 * 若当前 USB 正在发送数据，则数据会在之前的传输完成后发送。
 * 若发送缓冲区无法容纳数据，则最旧的数据将被覆盖。
 * 此函数立即返回，不会阻塞，发送将在后台进行。
 * @param transport 未使用，请传 NULL
 * @param buf 要发送的数据的首地址
 * @param len 要发送的数据的长度
 * @param err 错误码。若发送过程中曾发生（不论是否是本次发送）缓冲区溢出，则
 * USB_ERR_RINGBUF_OVERFLOW 标志位会被设置；若曾发生 USB 底层错误，则
 * USB_ERR_USBD_ERROR 标志位会被设置。
 * @returns 实际写入长度，总是为 len（完全写入）
 */
size_t usb_transport_write(uxrCustomTransport *transport, const uint8_t *buf,
                           size_t len, uint8_t *err);

/**
 * 从接收缓冲区读取最多 len 个字节到 buf，若无数据可读，则最多等待 timeout 毫秒。
 * 在接收缓冲区有数据的情况下，该函数会立即返回。
 * 若数据未能被及时读取导致接收缓冲区溢出，则最旧的数据会被覆盖而丢失。
 * @param transport 未使用，请传 NULL
 * @param buf 存储接收数据的首地址
 * @param len 最多接收的长度
 * @param timeout 无数据接收时最多等待的毫秒数
 * @param err 错误码。若接收过程中曾发生（不论是否是本次接收）缓冲区溢出，则
 * USB_ERR_RINGBUF_OVERFLOW 标志位会被设置；若曾发生 USB 底层错误，则
 * USB_ERR_USBD_ERROR 标志位会被设置。
 * @returns 实际接收数据长度
 */
size_t usb_transport_read(uxrCustomTransport *transport, uint8_t *buf,
                          size_t len, int timeout, uint8_t *err);

/* USER CODE END EXPORTED_FUNCTIONS */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_H__ */
