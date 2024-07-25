/*
 * dht11.c
 *
 *  Created on: 2024年6月9日
 *      Author: 杨璐
 */

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "list.h"
#include "task.h"

typedef struct w1_gpio_s
{
	GPIO_TypeDef	*group;
	uint16_t		pin;
}w1_gpio_t;

static w1_gpio_t 	W1Dat =
{
		.group = GPIOA,
		.pin = GPIO_PIN_5,
};

#define W1DQ_Input()\
{\
	GPIO_InitTypeDef GPIO_InitStruct = {0};\
	GPIO_InitStruct.Pin = W1Dat.pin;\
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;\
	GPIO_InitStruct.Pull = GPIO_PULLUP;\
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;\
	HAL_GPIO_Init(W1Dat.group,&GPIO_InitStruct);\
}

#define W1DQ_Output()\
{\
	GPIO_InitTypeDef GPIO_InitStruct = {0};\
	GPIO_InitStruct.Pin = W1Dat.pin;\
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;\
	GPIO_InitStruct.Pull = GPIO_NOPULL;\
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;\
	HAL_GPIO_Init(W1Dat.group,&GPIO_InitStruct);\
}

#define W1DQ_Write(x)	HAL_GPIO_WritePin(W1Dat.group, W1Dat.pin, (x==1)?GPIO_PIN_SET:GPIO_PIN_RESET)
#define W1DQ_Read()		HAL_GPIO_ReadPin(W1Dat.group, W1Dat.pin)


/*主机发送起始信号*/
static void Dht11_startsignal(void)
{
	W1DQ_Output();
	/*主机拉低*/
	W1DQ_Write(0);
	//vTaskDelay(pdMS_TO_TICKS(20));
	HAL_Delay(20);

	/*主机拉高*/
	W1DQ_Write(1);
	delay_us(30);

	W1DQ_Input();
}

uint8_t Dht11_respondsignal(void)
{
	uint8_t		retry = 0;

	/*检测低电平信号，等待电平变成低电平*/
	while(W1DQ_Read() && retry<100)
	{
		retry++;
		delay_us(1);
	}
	if(retry>=100)
		return 1;

	retry = 0;
	/*检测高电平信号*/
	while(!W1DQ_Read() && retry<100)
	{
		retry++;
		delay_us(1);
	}
	if(retry>=100)
		return 1;

	return 0;
}

uint8_t Dht11_readbit(void)
{
	uint8_t		retry = 0;

	while(W1DQ_Read() && retry<100)
	{
		retry++;
		delay_us(1);
	}

	retry = 0;

	while(!W1DQ_Read() && retry<100)
	{
		retry++;
		delay_us(1);
	}

	delay_us(40);
	if(W1DQ_Read())
		return 1;
	else
		return 0;
}

uint8_t Dht11_readByte(void)
{
	uint8_t		i,dat=0;

	for(i=0; i<8; i++)
	{
		dat <<= 1;
		dat |= Dht11_readbit();
	}

	return dat;
}

int Dht11_sampledata(float *temperature, float *humidity)
{
	uint8_t		humi_hb;
	uint8_t		humi_lb;
	uint8_t		temp_hb;
	uint8_t		temp_lb;
	uint8_t		check;

	if(!temperature || !humidity)
		return -1;

	Dht11_startsignal();
	if(Dht11_respondsignal())
	{
		return -2;
	}

	humi_hb = Dht11_readByte();
	humi_lb = Dht11_readByte();
	temp_hb = Dht11_readByte();
	temp_lb = Dht11_readByte();
	check = Dht11_readByte();

	if((humi_hb+humi_lb+temp_hb+temp_lb) != check)
		return -3;

	*temperature = (temp_hb*100+temp_lb)/100.00;
	*humidity = (humi_hb*100+humi_lb)/100.00;

	return 0;
}
