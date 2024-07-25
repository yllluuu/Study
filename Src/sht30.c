/*
 * sht30.c
 *
 *  Created on: Jun 13, 2024
 *      Author: 杨璐
 */

#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "sht30.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"

//#define CONFIG_SHT30_DEBUG

#ifdef CONFIG_SHT30_DEBUG
#define sht30_print(format,args...)	printf(format,##args)
#else
#define sht30_print(format,args...)	do{} while(0)
#endif

static int sht30_send_cmd(SHT30_CMD cmd)
{
	uint8_t		buf[2];

	buf[0] = cmd >> 8;
	buf[1] = cmd & 0xFF;

	return HAL_I2C_Master_Transmit(&hi2c2, SHT30_ADDR_WR, (uint8_t*)buf, 2, 0xFFFF);
}

static void sht30_soft_reset(void)
{
	sht30_send_cmd(SOFT_RESET_CMD);
	vTaskDelay(pdMS_TO_TICKS(1));
}

static int sht30_single_shot_measurement(uint8_t *buf, uint8_t buf_size)
{
	uint16_t	cmd = HIGH_ENABLED_CMD;
	uint8_t		rv;

	if( !buf || buf_size<SHT30_DATA_SIZE )
	{
		sht30_print("%s():Invalid input arguments\n",__func__);
		return -1;
	}

	rv = sht30_send_cmd(cmd);
	if( rv )
	{
		sht30_print("error:sht30 send measurement command failure, rv=%d\n", rv);
		sht30_soft_reset();
		return -2;
	}

	rv = HAL_I2C_Master_Receive(&hi2c2, SHT30_ADDR_RD, buf, SHT30_DATA_SIZE, 0xFFFF);

	if( rv )
	{
		sht30_print("error:sht30 read measurement result failure, rv=%d\n", rv);
		return -3;
	}

	return 0;
}

static uint8_t sht30_crc8(const uint8_t *data, int len)
{
	const uint8_t 	POLYNOMIAL = 0x31;/*0x31=x8+x5+x4+1*/
	uint8_t			crc = 0xFF;
	int 			i,j;

	for(i=0; i<len; ++i)
	{
		crc ^= *data++;
		for(j=0; j<8; ++j)
		{
			crc = ( crc & 0x80 )?(crc << 1)^POLYNOMIAL:(crc << 1);
		}
	}
	return crc;
}

int SHT30_SmapleData(float *temperature, float *humidity)
{
	uint8_t		buf[SHT30_DATA_SIZE];
	int			rv;
	uint16_t	temp;
	uint16_t	humd;
	uint8_t		crc;

	if(!temperature || !humidity)
	{
		sht30_print("%s():Invalid input arguments\n",__func__);
		return -1;
	}

	rv = sht30_single_shot_measurement(buf, SHT30_DATA_SIZE);
	if(rv)
	{
		sht30_print("SHT30 single short measurement failure, rv=%d\n",rv);
		printf("$$$$$$$$$$sht30rv=%d\r\n",rv);
		return -2;
	}

#ifdef CONFIG_SHT30_DEBUG
	{
		int		i;
		sht30_print("sht30 get %d bytes sample data:\n",SHT30_DATA_SIZE);
		for(i=0; i<SHT30_DATA_SIZE; i++)
		{
			sht30_print("0x%02x",buf[i]);
		}
		sht30_print("\n");
	}
#endif

	/*byte[0-1] is temperature value,byte[2] is temperatue crc*/
	crc = sht30_crc8(buf, 2);
	sht30_print("sht30 temperature cal_crc:[%02x] exp_crc:[%02x]\n",crc, buf[2]);
	if(crc!=buf[2])
	{
		sht30_print("sht30 measurement temperature got crc error\n");
		return -3;
	}

	/*byte[3-4] is humidity value,byte[5] is humiduty crc*/
	crc = sht30_crc8(&buf[3], 2);
	sht30_print("sht30 humidity cal_crc:[%02x] exp_crc:[%02x]\n",crc,buf[5]);

	if(crc != buf[5])
	{
		sht30_print("sht30 measurement temperature got crc error\n");
		return -4;
	}

	temp = (buf[0]<<8) | buf[1];
	humd = (buf[3]<<8) | buf[4];

	*temperature = -45 + 175*((float)temp/65535);
	*humidity = 100 * ((float)humd/65535);

	return 0;
}

void float_to_hex(float f, char hex[9])
{
	uint8_t *byteptr ;
	byteptr = (uint8_t *)&f;
	snprintf(hex,9,"%02X%02X%02X%02X",byteptr[3],byteptr[2],byteptr[1],byteptr[0]);
}

int sht30_get_temp(char *buf,int size)
{
	float			temperature,humidity;
	char			hex1[9],hex2[9];
	int				rv;

	memset(hex1,0,sizeof(hex1));
	memset(hex2,0,sizeof(hex2));

	rv=SHT30_SmapleData(&temperature, &humidity);
	if(rv<0)
	{
		printf("error\r\n");
		return -1;
	}
	float_to_hex(temperature, hex1);
	float_to_hex(humidity, hex2);
	snprintf(buf, size,"AT+QLWULDATAEX=13,0200250008%s%s,0x0100",\
													hex2,hex1);
	return 0;
}
