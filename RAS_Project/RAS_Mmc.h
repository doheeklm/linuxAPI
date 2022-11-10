/* RAS_Mmc.h */
#ifndef _RAS_MMC_H_
#define _RAS_MMC_H_

#define ARG_NUM_IP			1
#define ARG_NUM_DESC		2
#define ARG_NUM_TIME		2
#define ARG_NUM_ID			1
#define ARG_NUM_NAME		2
#define ARG_NUM_GENDER		3
#define ARG_NUM_BIRTH		4

#define ARG_ID_TIME			63006
#define ARG_ID_IP			63007
#define ARG_ID_DESC			63008
#define ARG_ID_ID			63009
#define ARG_ID_NAME			63010
#define ARG_ID_GENDER		63011
#define ARG_ID_BIRTH		63012
#define ARG_ID_M			63013
#define ARG_ID_F			63014

#define ARG_DESC_IP			"CLI_IP"
#define ARG_DESC_DESC		"CLI_IP_DESC"
#define ARG_DESC_TIME		"PERIOD_TM"
#define ARG_DESC_M			"MALE"
#define ARG_DESC_F			"FEMALE"
#define ARG_DESC_ID			"USR_ID"
#define ARG_DESC_NAME		"USR_NAME"
#define ARG_DESC_GENDER		"USR_GENDER"
#define ARG_DESC_BIRTH		"USR_BIRTH"
#define ARG_DESC_BIRTH		"USR_BIRTH"

#define CMD_NUM_ADD_IP		1
#define CMD_NUM_DIS_IP		2
#define CMD_NUM_DEL_IP		3
#define CMD_NUM_ADD_TRC		1
#define CMD_NUM_DIS_TRC		2
#define CMD_NUM_DEL_TRC		3
#define CMD_NUM_DIS_INFO	1
#define CMD_NUM_DEL_INFO	2

#define CMD_DESC_ADD_IP		"add-cli-ip"
#define CMD_DESC_DIS_IP		"dis-cli-ip"
#define CMD_DESC_DEL_IP		"del-cli-ip"
#define CMD_DESC_ADD_TRC	"add-cli-ip-trc"
#define CMD_DESC_DIS_TRC	"dis-cli-ip-trc"
#define CMD_DESC_DEL_TRC	"del-cli-ip-trc"
#define CMD_DESC_DIS_INFO	"dis-usr-info"
#define CMD_DESC_DEL_INFO	"del-usr-info"

int MMC_Init();
void MMC_Destroy();

#endif /* _RAS_MMC_H_ */
