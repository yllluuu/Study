/*
 * at_cmd.h
 *
 *  Created on: 2024年7月14日
 *      Author: 杨璐
 */

#ifndef INC_AT_CMD_H_
#define INC_AT_CMD_H_
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "stream_buffer.h"
#include "semphr.h"
#include "usart.h"

typedef struct atcmd_s
{
	comport_t    *comport;
	char		xAtCmd[256];
	char		xAtCmdReply[256];
}atcmd_t;

extern atcmd_t	g_atcmd;

#define ATBUF_SIZE	256	/* AT command UART receive buffer size */
#define ATCMD_SIZE  256		/* AT command/Reply message length */

#define STREAM_BUFFER_SIZE 1024
#define	TRIGGER_LEVEL      0

#define AT_OKSTR	"\r\nOK\r\n"
#define AT_ERRSTR	"\r\nERROR\r\n"
#define AT_SUFFIX	"\r\n"

#define Receive_EVENT		(0x01<<0)
#define Send_EVENT			(0x01<<1)

extern EventGroupHandle_t	Event_Handle;

extern SemaphoreHandle_t	xSemaphore;

extern int atcmd_pars(char *buf);
extern int atcmd_send(char *at, uint32_t timeout,char *expect, char *error,char *reply,size_t size);
extern int atcmd_check_OK(char *at,uint32_t timeout);
extern int atcmd_check_value(char *at, uint32_t timeout,char *reply,size_t size);
extern int parser_async_message(char *buf,char *keystr);

#endif /* INC_AT_CMD_H_ */
