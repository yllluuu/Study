/*
 * NB-IoT_test.c
 *
 *  Created on: Jul 2, 2024
 *      Author: 杨璐
 */
#include "at-bc28.h"
#include "usart.h"
#include "string.h"
#include "at_cmd.h"

NBiot_conf_t		NBconf;

int bc28_check_at()
{
	if(atcmd_check_OK("AT", 500)<0)
	{
		printf("AT command test failed,try again...\r\n");
		return -1;
	}
	else
		printf("AT command is normal.\r\n");
	return 0;
}

int bc28_get_mnf(char *reply_buf)
{
	if(atcmd_send("AT+CGMI", 200,AT_OKSTR,AT_ERRSTR,reply_buf,sizeof(reply_buf))<0)
	{
		printf("View module manufacturers failed,try again...\r\n");
		return -1;
	}
	else
	{
		strncpy(NBconf.manufacturers,reply_buf,strlen(reply_buf));
		printf("View module manufacturers OK.\r\n");
	}
	return 0;
}

int bc28_get_model(char *reply_buf)
{
	if(atcmd_send("AT+CGMM", 200,AT_OKSTR,AT_ERRSTR,reply_buf,sizeof(reply_buf))<0)
	{
		printf("View module model failed,try again...\r\n");
		return -1;
	}
	else
	{
		strncpy(NBconf.model,reply_buf,strlen(reply_buf));
		printf("View module model OK.\r\n");
	}
	return 0;
}

int bc28_check_imei(char *reply_buf)
{
	if(atcmd_check_value("AT+CGSN=1",200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("Check module IMEI number is not normal\r\n");
		return -1;
	}
	strncpy(NBconf.IMEI,reply_buf,strlen(reply_buf));
	printf("Check module IMEI number is normal\r\n");

	return 0;
}

int bc28_check_simcd(char *reply_buf)
{
	if(atcmd_send("AT+CIMI",200,AT_OKSTR,AT_ERRSTR,reply_buf,sizeof(reply_buf))<0)
	{
		printf("SIM card does not exist\r\n");
		return -1;
	}
	strncpy(NBconf.SIM,reply_buf,strlen(reply_buf)-6);
	printf("SIM card exists\r\n");

	return 0;
}

int bc28_set_autocnt()
{
	if(atcmd_check_OK("AT+NCONFIG=AUTOCONNECT,TRUE",200)<0)
	{
		printf("Auto connect failed\r\n");
		return -1;
	}
	printf("Auto connect OK\r\n");

	return 0;
}

int bc28_check_CFUN(char *reply_buf)
{
	if(atcmd_check_value("AT+CFUN?", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("The radio is not turn on\r\n");
		return -1;
	}
	printf("The radio is turned on\r\n");

	return 0;
}

int bc28_check_CSQ(char *reply_buf)
{
	if(atcmd_check_value("AT+CSQ", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("The module signal test failed,try again...\r\n");
		return -1;
	}
	strncpy(NBconf.CSQ,reply_buf,strlen(reply_buf));
	printf("The module signal test is normal\r\n");

	return 0;
}

int bc28_set_attach_net()
{
	if(atcmd_check_OK("AT+CGATT=1", 200)<0)
	{
		printf("The module attachment network test failed,try again...\r\n");
		return -1;
	}
	printf("The module attachment network test is normal\r\n");

	return 0;
}

int bc28_check_attach_net(char *reply_buf)
{
	if(atcmd_check_value("AT+CGATT?", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("The module attachment network test failed,try again...\r\n");
		return -1;
	}
	printf("The module attachment network test is normal\r\n");

	return 0;
}

int bc28_check_reg_status(char *reply_buf)
{
	if(atcmd_check_value("AT+CEREG?", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("The network registration status is abnormal,try again...\r\n");
		return -1;
	}
	printf("The network registration status is normal\r\n");

	return 0;
}

int bc28_check_ip(char *reply_buf)
{
	if(atcmd_check_value("AT+CGPADDR", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("Obtaining an IP address is abnormal,try again...\r\n");
		return -1;
	}
	printf("Obtaining an IP address is normal\r\n");

	return 0;
}

int bc28_set_ip_port(char *reply_buf)
{
	if(atcmd_send("AT+NCDP=221.229.214.202,5683\r\n",200,AT_OKSTR,AT_ERRSTR,reply_buf,sizeof(reply_buf))<0)
	{
		printf("The module fails to connect to the cloud platform failed, try again...\r\n");
		return -1;
	}
	printf("The module connect to the cloud platform successfully.\r\n");

	return 0;
}

int bc28_check_ip_port(char *reply_buf)
{
	if(atcmd_check_value("AT+NCDP?", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("The cloud platform is incorrectly configured, try again...\r\n");
		return -1;
	}
	printf("The cloud platform is correctly configured.\r\n");

	return 0;
}

int bc28_check_iot(char *reply_buf)
{
	if(atcmd_check_value("AT+NMSTATUS?", 200,reply_buf,sizeof(reply_buf))<0)
	{
		printf("Failed to register the telecom cloud platform,try again...\r\n");
		return -1;
	}
	printf("Successfully registered on the telecom cloud platform\r\n");

	return 0;
}

int NB_RSET_OK()
{
	if(bc28_check_at()<0)
		return -1;

	return 0;
}

int NB_HDW_OK()
{
	char 		reply_buf[256];

	if(bc28_get_mnf(reply_buf)<0)
		return -1;

	if(bc28_get_model(reply_buf)<0)
		return -1;

	if(bc28_check_imei(reply_buf)<0)
		return -1;

	if(bc28_check_simcd(reply_buf))
		return -1;

	return 0;
}

int NB_CONF_OK()
{
	char 		reply_buf[256];

	if(bc28_set_autocnt()<0)
		return -1;

	if(bc28_check_CFUN(reply_buf)<0)
		return -1;

	if(bc28_check_CSQ(reply_buf)<0)
		return -1;

	if(bc28_set_attach_net()<0)
		return -1;

	if(bc28_check_attach_net(reply_buf)<0)
		return -1;

	if(bc28_check_reg_status(reply_buf)<0)
		return -1;

	if(bc28_check_ip(reply_buf)<0)
		return -1;

	if(bc28_set_ip_port(reply_buf)<0)
		return -1;

	if(bc28_check_ip_port(reply_buf)<0)
		return -1;

	if(bc28_check_iot(reply_buf)<0)
		return -1;

	return 0;
}

