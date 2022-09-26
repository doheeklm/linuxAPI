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
#include "stctl/stctl.h"
#include "test_stat.h"
#include "oam_uda.h"
#include "sfm_alarm.h"
#include "oammmc.h"

#include "FW_mmc.h"

#define SERVER_PROCNAME				"MIPCSVR"
#define CLIENT_PROCNAME				"MIPCCLI"

#define INSERT						"Insert"
#define SELECT						"Select"
#define UPDATE						"Update"
#define DELETE						"Delete"
#define NUMTUPLES					"NUMTUPLES"

#define FLAG_RUN					100
#define FLAG_STOP					99

#define SIZE_NAME					32
#define SIZE_JOBTITLE				32
#define SIZE_TEAM					32
#define SIZE_PHONE					13

#define TABLE_NAME					"EmployeeInfos"
#define MSG_TYPE					"MsgType"
#define ALL							"{ALL}"
#define ID							"id"
#define NAME						"name"
#define JOBTITLE					"jobTitle"
#define TEAM						"team"
#define PHONE						"phone"

#define UDA_UPP_GNAME				"FW_TEST"
#define UDA_LOW_GNAME				"CNT"
#define UDA_ITEM_NAME				"CNT_EMPLOYEE_ALARM"

#define MODULE_CONF					"/home1/sepp/release/R1.0.0/etc/ini/OAM/sfwcs.ini"
#define MMT_PORT					"mmt_port"
#define MMT_ENABLE					"mmt_enable"

typedef enum
{
	SUCCESS = 1, INPUT_FAIL = 2,
	DAL_FAIL = -2, FGETS_FAIL = -3, TAP_FAIL = -4,
	NULL_FAIL = -5, MPGLOG_FAIL = -6, STAT_FAIL = -7,
	ID_NOT_EXIST = 30, NAME_NOT_EXIST = 31,
	UDA_FAIL = -8
} ReturnCode_t;

typedef struct REQUEST_s
{
	int		nMsgType;
	int		nId;
	char	szName		[SIZE_NAME + 1];
	char	szJobTitle	[SIZE_JOBTITLE + 1];
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
	char	szJobTitle	[SIZE_JOBTITLE + 1];
	char	szTeam		[SIZE_TEAM + 1];
	char	szPhone		[SIZE_PHONE + 1];
} SELECT_ONE_t;

int g_nFlag = FLAG_RUN;

DAL_CONN	*g_ptConn= NULL;
DAL_PSTMT	*g_ptPstmtInsert = NULL;
DAL_PSTMT	*g_ptPstmtSelectAll = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneById = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneByName = NULL;
DAL_PSTMT	*g_ptPstmtUpdate = NULL;
DAL_PSTMT	*g_ptPstmtDelete = NULL;
DAL_PSTMT	*g_ptPstmtNumtuples = NULL;

#endif /*_FW_INC_H_*/
