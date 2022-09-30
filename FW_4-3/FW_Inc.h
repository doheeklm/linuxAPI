/* FW_4-3/FW_Inc.h */
#ifndef _FW_INC_H_
#define _FW_INC_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio_ext.h>
#include <signal.h>
#include <unistd.h>

/* FRAMEWORK */
#include <dal.h>
#include <TAP_Ipc.h>
#include <mplog.h>
#include "mpsignal.h"
#include "stctl/stctl.h"
#include "test_stat.h"
#include "oam_uda.h"
#include "sfm_alarm.h"
#include "oammmc.h"
#include "oammmc_mmt.h"
#include "oammmc_mml.h"
#include "mpenum.h"

#include "FW_Mmc.h"
#include "FW_Pstmt.h"

#define MODULE						"MIPCSVR"

#define INSERT						"Insert"
#define SELECT						"Select"
#define UPDATE						"Update"
#define DELETE						"Delete"
#define NUMTUPLES					"NUMTUPLES"

#define FLAG_RUN					100
#define FLAG_STOP					99

#define SIZE_NAME					32
#define SIZE_POSITION				32
#define SIZE_TEAM					32
#define SIZE_PHONE					13

#define TABLE_NAME					"EmployeeInfos"
#define MSG_TYPE					"MsgType"
#define ALL							"{ALL}"

#define ID							"id"
#define NAME						"name"
#define POSITION					"position"
#define TEAM						"team"
#define PHONE						"phone"

#define UDA_UPP_GNAME				"FW_TEST"
#define UDA_LOW_GNAME				"CNT"
#define UDA_ITEM_NAME				"CNT_EMPLOYEE_ALARM"

#define PROCESS_INI					"./MIPCSVR.ini"
#define MMT_PORT					"mmt_port"
#define MMT_ENABLE					"mmt_enable"
#define MML_ENABLE					"mml_enable"
#define MMT_CONN_MAX				"mmt_conn_max"
#define MMT_LOCAL_ONLY				"mmt_local_only"
#define MMT_IS_QUIET				"mmt_is_quiet"

//[OMP]~etc/ini/OAM/command.ut
#define MSG_ID_ADD					7741
#define MSG_ID_DIS					7742
#define MSG_ID_CHG					7743
#define MSG_ID_DEL					7744

//[OMP]~etc/ini/OAM/enum.ut
#define ENUM_ID						63000
#define ENUM_NAME					63001
#define ENUM_POSITION				63002
#define ENUM_TEAM					63003
#define ENUM_PHONE					63004

typedef enum
{
	SUCCESS = 1, INPUT_FAIL = 2, DAL_FAIL = -2,
	FGETS_FAIL = -3, IPC_FAIL = -4, NULL_FAIL = -5,
	MPGLOG_FAIL = -6, STAT_FAIL = -7,
	ID_NOT_EXIST = 30, NAME_NOT_EXIST = 31,
	UDA_FAIL = -8, MMC_FAIL = -9, MPCONF_FAIL = -10
} ReturnCode_t;

typedef struct REQUEST_s
{
	int		nMsgType;
	int		nId;
	char	szName		[SIZE_NAME + 1];
	char	szPosition	[SIZE_POSITION + 1];
	char	szTeam		[SIZE_TEAM + 1];
	char	szPhone		[SIZE_PHONE + 1];
} REQUEST_t;

typedef struct RESPONSE_s
{
	int		nMsgType;
	int		nId;
	int		nResult;
	char	szBuffer	[2048];
	int		nCntSelectAll;
} RESPONSE_t;

typedef struct SELECT_ALL_s
{
	int		nId;
	char	szName		[SIZE_NAME + 1];
} SELECT_ALL_t;

typedef struct SELECT_ONE_s
{
	char	szName		[SIZE_NAME + 1];
	char	szPosition	[SIZE_POSITION + 1];
	char	szTeam		[SIZE_TEAM + 1];
	char	szPhone		[SIZE_PHONE + 1];
} SELECT_ONE_t;

#endif /*_FW_INC_H_*/
