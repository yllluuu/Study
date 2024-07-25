/*
 * at_cmd.c
 *
 *  Created on: 2024年7月14日
 *      Author: 杨璐
 */
#include "at_cmd.h"
#include "usart.h"
#include <string.h>

EventGroupHandle_t	Event_Handle;
StreamBufferHandle_t	xAsynStreamBuffer;
SemaphoreHandle_t	xSemaphore;

atcmd_t	g_atcmd;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (huart->Instance == USART3)
    {
    	HAL_UART_Receive_IT(g_atcmd.comport->dev, &data, 1);
        if(xStreamBufferSpacesAvailable(xStreamBuffer)>0)
        {
        	xStreamBufferSendFromISR(xStreamBuffer, &data, 1, &xHigherPriorityTaskWoken);
        }
        if('\n' == data)
        {
        	xSemaphoreGiveFromISR(xSemaphore,&xHigherPriorityTaskWoken);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int atcmd_pars(char *buf)
{
	char		*ptr,*end;
	int			bytes = 0;
	size_t		i;

	if('\0' == g_atcmd.xAtCmd[0])
	{
		return 0;
	}

	for(i=0;i<strlen(g_atcmd.xAtCmd);i++)
	{
		if(g_atcmd.xAtCmd[i] == '\n')
		{
			g_atcmd.xAtCmd[i] = '\0';
			break;
		}
	}

	/*AT command echo on*/
	if(!(ptr = strstr(buf,g_atcmd.xAtCmd)))
	{
		return 0;
	}

	if((end = strstr(ptr,AT_OKSTR)))
	{
		end += strlen(AT_OKSTR);
		bytes = end - ptr;
	}
	else if((end = strstr(ptr,AT_ERRSTR)))
	{
		if((end = strstr(ptr,"\r\n")))
		{
			end+=strlen(AT_ERRSTR);
			bytes = end - ptr;
		}
	}

	if(bytes)
	{
		strncpy(g_atcmd.xAtCmdReply,ptr,bytes);
		printf("got AT reply:%s",g_atcmd.xAtCmdReply);
		xEventGroupSetBits(Event_Handle,Receive_EVENT);
		return 1;
	}
	return 0;
}
/*
int atcmd_send(char *at, uint32_t timeout,char *reply,size_t size)
{
	EventBits_t		r_event;
	char			*ptr,*end;
	int				rv = 0;

	if(!at || !strlen(at))
		return -1;

	printf("send AT command:%s\r\n",at);
	snprintf(g_atcmd.xAtCmd,ATCMD_SIZE,"%s%s",at,AT_SUFFIX);
	memset(g_atcmd.xAtCmdReply,0,ATCMD_SIZE);

	if(comport_send(g_atcmd.comport, g_atcmd.xAtCmd, strlen(g_atcmd.xAtCmd)) != HAL_OK)
	{
		printf("Send AT command failed\r\n");
		rv = -2;
		goto out;
	}
	printf("Send AT command OK\r\n");

	r_event = xEventGroupWaitBits(Event_Handle,Receive_EVENT,pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout));
	if(!(r_event&Receive_EVENT))
	{
		rv = -3;
		goto out;
	}

	ptr = strstr(g_atcmd.xAtCmdReply,at);
	if(!ptr)
	{
		rv = -4;
		goto out;
	}

	ptr += strlen(at);
	if(NULL != (end=strstr(ptr,AT_OKSTR)))
	{
		if(reply && size>0)
		{
			end += strlen(AT_OKSTR);
			memset(reply, 0, size);
			strncpy(reply,ptr,(size_t)(end-ptr));
			printf("AT command %s got reply:%s",at,reply);
		}
		//printf("*********p\r\n");
		rv = 0;
		goto out;
	}

	if(NULL != (end=strstr(ptr,AT_ERRSTR)))
	{
		rv = -5;
		goto out;
	}

out:
	memset(g_atcmd.xAtCmd,0,sizeof(g_atcmd.xAtCmd));
	memset(g_atcmd.xAtCmdReply,0,sizeof(g_atcmd.xAtCmdReply));
	if(rv<0)
		return -1;
	else
		return 0;
}
*/
int atcmd_send(char *at, uint32_t timeout,char *expect, char *error, char *reply,size_t size)
{
	EventBits_t		r_event;
	char			*ptr,*end;
	int				rv = 0;

	if(!at || !strlen(at))
		return -1;

	printf("send AT command:%s\r\n",at);
	snprintf(g_atcmd.xAtCmd,ATCMD_SIZE,"%s%s",at,AT_SUFFIX);
	memset(g_atcmd.xAtCmdReply,0,ATCMD_SIZE);

	if(comport_send(g_atcmd.comport, g_atcmd.xAtCmd, strlen(g_atcmd.xAtCmd)) != HAL_OK)
	{
		printf("Send AT command failed\r\n");
		rv = -2;
		goto out;
	}
	printf("Send AT command OK\r\n");

	r_event = xEventGroupWaitBits(Event_Handle,Receive_EVENT,pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout));
	if(!(r_event&Receive_EVENT))
	{
		rv = -3;
		goto out;
	}

	ptr = strstr(g_atcmd.xAtCmdReply,at);
	if(!ptr)
	{
		rv = -4;
		goto out;
	}

	ptr += strlen(at);
	if(NULL != (end=strstr(ptr,expect)))
	{
		if(reply && size>0)
		{
			end += strlen(AT_OKSTR);
			memset(reply, 0, size);
			strncpy(reply,ptr,(size_t)(end-ptr));
			printf("AT command %s got reply:%s",at,reply);
		}
		//printf("*********p\r\n");
		rv = 0;
		goto out;
	}

	if(NULL != (end=strstr(ptr,error)))
	{
		rv = -5;
		goto out;
	}

out:
	memset(g_atcmd.xAtCmd,0,sizeof(g_atcmd.xAtCmd));
	memset(g_atcmd.xAtCmdReply,0,sizeof(g_atcmd.xAtCmdReply));
	if(rv<0)
		return -1;
	else
		return 0;
}

int atcmd_check_OK(char *at,uint32_t timeout)
{
	int		rv = 0;

	rv = atcmd_send(at, timeout, AT_OKSTR, AT_ERRSTR, NULL, 0);
	if(rv<0)
	{
		return -1;
	}

	return 0;
}
int atcmd_check_value(char *at, uint32_t timeout,char *reply,size_t size)
{
	int			rv = 0,i=0;
	char		buf[ATBUF_SIZE];
	char		*ptr,*end;
	uint32_t	len;

	if(!at || !reply || size<=0)
	{
		rv = -1;
		goto out;
	}

	rv = atcmd_send(at,timeout,AT_OKSTR,AT_ERRSTR,buf,sizeof(buf));
	if(rv)
	{
		goto out;
	}

	ptr = strchr(buf,'+');
	if(!ptr)
	{
		rv = -2;
		goto out;
	}
	ptr++;

	ptr = strchr(buf,':');
	if(!ptr)
	{
		rv = -3;
		goto out;
	}
	ptr++;

	end = strstr(ptr,"\r\nOK");
	if(!end)
	{
		rv = -4;
		goto out;
	}
	memset(reply,0,size);

	if(!(len=strlen(ptr)))
	{
		rv = -5;
		goto out;
	}

	while(ptr != end)
	{
		if(*ptr!='\"')
			reply[i++] = *ptr;
		ptr++;
		rv = 0;
		goto out;
	}

out:
	memset(buf,0,sizeof(buf));
	if(rv<0)
		return -1;
	else
		return 0;
}

int parser_async_message(char *buf,char *keystr)
{
	char		*ptr,*end;
	int			bytes = 0;
	char		Asynbuf[128];

	if(!(ptr=strstr(buf,keystr)))
		return -1;

	if(!(end=strstr(ptr+strlen(keystr),"\r\n")))
		return -2;

	bytes = end-ptr+2;

	strncpy(Asynbuf,buf,(size_t)bytes);
	printf("^^^^^^^^Asynbuf=%s\r\n",Asynbuf);

	if(strstr(Asynbuf,"0622B8") && strstr(Asynbuf,"0101"))
	{
		printf("led on\n");
		HAL_GPIO_WritePin(GPIOB, green_led_Pin, GPIO_PIN_RESET);
	}

	else if(strstr(Asynbuf,"0100")&& strstr(Asynbuf,"0622B8"))
	{
		printf("led off\n");
		HAL_GPIO_WritePin(GPIOB, green_led_Pin, GPIO_PIN_SET);
	}

	//xEventGroupSetBits(Event_Handle,Aysn_EVENT);
	memset(Asynbuf,0,sizeof(Asynbuf));
	return 0;
}
