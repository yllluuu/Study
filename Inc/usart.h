/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "stream_buffer.h"

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */

#ifdef CONFIG_OS_LINUX
typedef int						UART_Handle_t
typedef pthread_mutex_lock		lock_t
#else
typedef UART_HandleTypeDef *	UART_Handle_t;
#endif

#define NBIOT_UART		&huart3

typedef struct comport_s
{
	UART_Handle_t 		dev;
}comport_t;

extern comport_t comport;

int comport_open(comport_t *comport, void *devname, long baudrate, char *settings);

int comport_send(comport_t *comport, char *data, int bytes);

int comport_recv(comport_t *comport, char *buf, int bytes, unsigned long timeout);

void comport_close(comport_t *comport);

extern StreamBufferHandle_t	xStreamBuffer;
extern uint8_t				data;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

