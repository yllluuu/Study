/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "at-bc28.h"
#include "main.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "at_cmd.h"
#include "dht11.h"
#include "task.h"
#include "list.h"
#include "timers.h"
#include "sht30.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

TaskHandle_t			RespondTask_Handle;

TaskHandle_t			NBIoTinit_Task;

TaskHandle_t			ReportTask_Handle;

TaskHandle_t			ReceiveTask_Handle;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
static void BSP_Init(void);
static void NBIoT_MGR(void *parameter);
static void atcmd_receive_task(void *parameter);
static void Report_Task(void *parameter);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	BSP_Init();
	BaseType_t xReturn = pdPASS;

	comport_open(&comport, NBIOT_UART, 115200, "8N1N");
	g_atcmd.comport = &comport;

	Event_Handle = xEventGroupCreate();
	if(NULL != Event_Handle)
		printf("Event_Handle was created\r\n");

	xSemaphore = xSemaphoreCreateBinary();
	if(NULL != xSemaphore)
		printf("The binary semaphore is successfully created\r\n");

	xStreamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE,TRIGGER_LEVEL);
	if(xStreamBuffer != NULL)
		printf("StreamBuffer was created\r\n");

	xReturn = xTaskCreate(NBIoT_MGR,"NBIoT_MGR", 1024,NULL,3,&NBIoTinit_Task);
	if(xReturn == pdPASS)
		printf("NBIoT_init_Task was created\r\n");

	xReturn = xTaskCreate(Report_Task,"Report_Task", 1024,NULL,4,&ReportTask_Handle);
	if(xReturn == pdPASS)
		printf("Report_Task was created\r\n");

	xReturn = xTaskCreate(atcmd_receive_task,"atcmd_receive_task",1024,NULL,3,&ReceiveTask_Handle);
	if(xReturn == pdPASS)
		printf("Receive_Task was created\r\n");

	printf("Free heap size: %u bytes\n", xPortGetFreeHeapSize());

	vTaskStartScheduler();
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void BSP_Init(void)
{
	HAL_Init();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	MX_TIM6_Init();
	MX_I2C2_Init();
	MX_FREERTOS_Init();
}

static void NBIoT_MGR(void *parameter)
{
	NBconf.status =STAT_INIT;

	while(1)
	{
		switch(NBconf.status)
		{
		case STAT_INIT:
			if(NB_RSET_OK()<0)
			{
				NBconf.status = STAT_INIT;
				break;
			}
			else
				NBconf.status++;

		case STAT_PRESEND:
			if(NB_HDW_OK()<0)
			{
				NBconf.status = STAT_PRESEND;
				break;
			}
			else
				NBconf.status++;

		case STAT_CONF:
			if(NB_CONF_OK()<0)
			{
				NBconf.status = STAT_CONF;
				break;
			}
			else
				NBconf.status++;

		case STAT_RDY:
			vTaskDelay(pdMS_TO_TICKS(1000));
			//break;
			continue;

		default:
			NBconf.status = STAT_INIT;
			break;
		}
	}
}

void atcmd_receive_task(void *parameter)
{
	uint32_t		bytes = 0;
	uint32_t		last_bytes = 0;
	int				timeout = 50;
	char			buf[256];

	while(1)
	{
		xSemaphoreTake(xSemaphore,portMAX_DELAY);
		last_bytes = xStreamBufferBytesAvailable(xStreamBuffer);

		if(!last_bytes)
		{
			printf("no data arrive but receive semaphore\r\n");
			continue;
		}

WAIT_NEWDATA:
		xSemaphoreTake(xSemaphore,pdMS_TO_TICKS(10));
		bytes = xStreamBufferBytesAvailable(xStreamBuffer);

		if(bytes != last_bytes)
		{
			last_bytes = bytes;
			goto WAIT_NEWDATA;
		}

		bytes = bytes>ATBUF_SIZE?ATBUF_SIZE:bytes;
		memset(buf,0,sizeof(buf));
		comport_recv(&comport, buf, bytes, timeout);
		printf("start to parser data\r\n");

		parser_async_message(buf, "+NNMI:");

		atcmd_pars(buf);
	}
}

static void Report_Task(void *parameter)
{
	char			atcmd[256];
	int 			timeout=500;
	char			reply_buf[256];

	while(1)
	{
		if(NBconf.status == STAT_RDY)
		{
			if(sht30_get_temp(atcmd,sizeof(atcmd))<0)
			{
				printf("get temperature and humidity error\r\n");
			}
			else
			{
				if(atcmd_send(atcmd, timeout,AT_OKSTR,AT_ERRSTR,reply_buf,sizeof(reply_buf))<0)
				{
					printf("Send data to cloud failed\r\n");
					NBconf.status =STAT_INIT;
					continue;
				}
				else
				{
					printf("Send data to cloud successfully\r\n");
				}
			}
		}
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
