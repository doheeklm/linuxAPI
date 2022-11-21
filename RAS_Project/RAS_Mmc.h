/* RAS_Mmc.h */
#ifndef _RAS_MMC_H_
#define _RAS_MMC_H_

#define DFLT_TIME			60

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
#define ARG_ID_MALE			63013
#define ARG_ID_FEMALE		63014

#define ARG_STR_IP			"CLI_IP"
#define ARG_STR_DESC		"CLI_IP_DESC"
#define ARG_STR_TIME		"PERIOD_TM"
#define ARG_STR_MALE		"MALE"
#define ARG_STR_FEMALE		"FEMALE"
#define ARG_STR_ID			"USR_ID"
#define ARG_STR_NAME		"USR_NAME"
#define ARG_STR_GENDER		"USR_GENDER"
#define ARG_STR_BIRTH		"USR_BIRTH"
#define ARG_STR_BIRTH		"USR_BIRTH"

#define CMD_NUM_ADD_IP		1
#define CMD_NUM_DIS_IP		2
#define CMD_NUM_DEL_IP		3
#define CMD_NUM_ADD_TRC		1
#define CMD_NUM_DIS_TRC		2
#define CMD_NUM_DEL_TRC		3
#define CMD_NUM_DIS_INFO	1
#define CMD_NUM_DEL_INFO	2

#define CMD_STR_ADD_IP		"add-cli-ip"
#define CMD_STR_DIS_IP		"dis-cli-ip"
#define CMD_STR_DEL_IP		"del-cli-ip"
#define CMD_STR_ADD_TRC		"add-cli-ip-trc"
#define CMD_STR_DIS_TRC		"dis-cli-ip-trc"
#define CMD_STR_DEL_TRC		"del-cli-ip-trc"
#define CMD_STR_DIS_INFO	"dis-usr-info"
#define CMD_STR_DEL_INFO	"del-usr-info"

int MMC_Init();
void MMC_Destroy();

#endif /* _RAS_MMC_H_ */
