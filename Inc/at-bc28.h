/*
 * NB-IoT_test.h
 *
 *  Created on: 2024年7月3日
 *      Author: 杨璐
 */

#ifndef INC_AT_BC28_H_
#define INC_AT_BC28_H_

enum
{
	STAT_INIT,
	STAT_PRESEND,
	STAT_CONF,
	STAT_RDY,
};

typedef struct NBiot_conf_s
{
	int		status;
	char	manufacturers[64];
	char	model[64];
	char	IMEI[64];
	char	SIM[64];
	char	CSQ[64];
}NBiot_conf_t;

extern NBiot_conf_t		NBconf;

extern int NB_RSET_OK();
extern int NB_HDW_OK();
extern int NB_CONF_OK();

#endif /* INC_AT_BC28_H_ */
