/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "FreeRTOS.h"
#include "ring_buffer.h"
#include "semphr.h"
#include <uxr/client/profile/multithread/multithread.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE]; // USB 接收硬件缓冲区

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE]; // USB 发送硬件缓冲区

/* USER CODE BEGIN PRIVATE_VARIABLES */

// 发送/接收环形缓冲区的预分配内存
static uint8_t tx_buf_mem[TX_RINGBUF_SIZE];
static uint8_t rx_buf_mem[RX_RINGBUF_SIZE];

// 发送/接收环形缓冲区
static ringbuf_t tx_buf;
static ringbuf_t rx_buf;

// 目前在 usb_transport_read 函数中等待新数据的线程，若无则为 NULL
static TaskHandle_t rx_waiting_task = NULL;

// USB 是否正在发送数据
static bool usb_transmitting = false;

// 发送/接收错误码
static uint8_t tx_errcode = 0;
static uint8_t rx_errcode = 0;

// 自上次成功接收到数据后，上位机程序是否发生过重启
static bool host_restarted = false;

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);

  // ISR 中进入临界区
  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();

  // 初始化环形缓冲区，丢弃其中数据
  ringbuf_init(&tx_buf, tx_buf_mem, TX_RINGBUF_SIZE);
  ringbuf_init(&rx_buf, rx_buf_mem, RX_RINGBUF_SIZE);

  // 清除错误码
  tx_errcode = 0;
  rx_errcode = 0;

  // 清除传输状态设置
  // 当 USB 断连时，传输完成的回调（CDC_TransmitCplt_FS）始终不会被调用
  // 而当 USB 重新连接时，HAL 不会传送先前数据，而是会先调用 CDC_DeInit_FS
  // 释放旧的介质层，再调用 CDC_Init_FS 初始化新的介质层，因此需要在此处清除
  // usb_transmitting 标志。
  usb_transmitting = false;

  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:
    // 当上位机打开 USB 串口时，会发送 CDC_SET_CONTROL_LINE_STATE 命令
    host_restarted = true;
    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  // 当 USB 接收完一个数据包后，调用此函数

  // ISR 中进入临界区
  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();

  // 将接收到的数据写入缓冲区
  size_t overflow = ringbuf_write(&rx_buf, Buf, *Len);

  // 将当前正在等待数据的线程取到 taskToNotify 中，并清除等待线程
  TaskHandle_t taskToNotify = rx_waiting_task;
  rx_waiting_task = NULL;

  if (overflow) {
	  // 若发生缓冲区溢出造成的覆盖，则设置错误标志位
	  rx_errcode |= USB_ERR_RINGBUF_OVERFLOW;
  }

  // 退出临界区
  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  // 准备接收下一个包
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);

  // 若有线程在等待数据，则唤醒它
  if (taskToNotify != NULL) {
	  BaseType_t higherPriorityTaskWoken;
	  xTaskNotifyFromISR(taskToNotify, 0, eNoAction, &higherPriorityTaskWoken);
	  portYIELD_FROM_ISR(
		  higherPriorityTaskWoken); // 若唤醒的线程优先级更高，则允许其抢占
  }

  return USBD_OK;
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  // 当 USB 数据包发送成功后，调用此回调函数

  // ISR 中进入临界区
  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();

  // 从发送缓冲区中取最多 64 个字节
  size_t read_n = ringbuf_read(&tx_buf, UserTxBufferFS, APP_TX_DATA_SIZE);
  if (read_n == 0) {
	  // 若无数据继续发送，则清除传输状态
	  usb_transmitting = false;
  }

  // 退出临界区
  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  if (read_n > 0) {
	  // 有数据要继续发送

	  // 开始发送下一组数据
	  result = CDC_Transmit_FS(UserTxBufferFS, read_n);

	  if (result != USBD_OK) {
		  // 若发生 USB 错误，则进入临界区并设置错误标志位
		  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
		  tx_errcode |= USB_ERR_USBD_ERROR;
		  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	  }
  }

  /* USER CODE END 13 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
bool usb_transport_open(uxrCustomTransport *transport) {

	// 进入临界区
	taskENTER_CRITICAL();

	// 初始化环形缓冲区，清除其中数据
	ringbuf_init(&tx_buf, tx_buf_mem, TX_RINGBUF_SIZE);
	ringbuf_init(&rx_buf, rx_buf_mem, RX_RINGBUF_SIZE);

	// 清除错误码
	tx_errcode = 0;
	rx_errcode = 0;

  // 清除检测上位机重启的标志位
  host_restarted = false;

	taskEXIT_CRITICAL();

	return true;
}

bool usb_transport_close(uxrCustomTransport *transport) {
  return true;
}

size_t usb_transport_write(uxrCustomTransport *transport, const uint8_t *buf,
                           size_t len, uint8_t *err) {

	// 进入临界区
	taskENTER_CRITICAL();

	// 向发送缓冲区中写入数据
	size_t overflow = ringbuf_write(&tx_buf, buf, len);

	if (overflow > 0) {
		// 若缓冲区溢出，则设置错误标志位
		tx_errcode |= USB_ERR_RINGBUF_OVERFLOW;
	}

	if (usb_transmitting) {
		// 如果有数据正在发送，则等待之前的数据传输完成后，就会发送我们的数据
		// 退出临界区
		taskEXIT_CRITICAL();

	} else {
		// 当前无数据正在发送，需要开启发送

    // 从发送缓冲区中取最多 64 字节数据
		size_t read_n = ringbuf_read(&tx_buf, UserTxBufferFS, APP_TX_DATA_SIZE);
		if (read_n > 0) {
      // 若有数据要发送，则设置传输状态
			usb_transmitting = true;
		}

    // 退出临界区
		taskEXIT_CRITICAL();

		if (read_n > 0) {
      // 若有数据要发送，则开始发送
			uint8_t result = CDC_Transmit_FS(UserTxBufferFS, read_n);
			if (result != USBD_OK) {
        // 若发生 USB 错误，则进入临界区并设置错误标志位
				taskENTER_CRITICAL();
				tx_errcode |= USB_ERR_USBD_ERROR;
				taskEXIT_CRITICAL();
			}
		}
	}

  // 返回发送错误码
	*err = tx_errcode;

	return len;
}

size_t usb_transport_read(uxrCustomTransport *transport, uint8_t *buf,
                          size_t len, int timeout, uint8_t *err) {

  // 进入临界区
	taskENTER_CRITICAL();

	if (ringbuf_bytes_used(&rx_buf) == 0) {
    // 接收缓冲区为空，要等待

    // 设置等待线程为当前线程
		rx_waiting_task = xTaskGetCurrentTaskHandle();

    // 退出临界区
		taskEXIT_CRITICAL();

    // 等待至多 timeout 毫秒，如果有数据到达则我们会被提前唤醒
    UXR_UNLOCK_TRANSPORT((&transport->comm));
		uint32_t notifiedValue;
		xTaskNotifyWait(0, 0, &notifiedValue, pdMS_TO_TICKS(timeout));
    UXR_LOCK_TRANSPORT((&transport->comm));

    // 重新进入临界区
		taskENTER_CRITICAL();

    // 清除等待线程
		rx_waiting_task = NULL;
	}

  // 从接收缓冲区中取最多 len 个字节
	size_t read_n = ringbuf_read(&rx_buf, buf, len);

  // 退出临界区
	taskEXIT_CRITICAL();

  // 返回接收错误码
	*err = rx_errcode;

	return read_n;
}

bool check_host_restart(void) {
  bool value = host_restarted;
  host_restarted = false;
  return value;
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
