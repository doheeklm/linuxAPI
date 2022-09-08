/* FW_2-5/TAP_Inc.h */

#ifndef _TAP_INC_H_
#define _TAP_INC_H_

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
#include <mpconf.h>
#include "TAP_Registry.h"
#include "TAP_Registry_udp.h"
#include "TAP_Registry_P.h"		//cd $TAP_HOME/include

#define SERVER_PROCESS	"MIPCSVR"
#define CLIENT_PROCESS	"MIPCCLI"

#define INSERT			"Insert"
#define SELECT			"Select"
#define UPDATE			"Update"
#define DELETE			"Delete"

#define FLAG_RUN		100
#define FLAG_STOP		99

#define SIZE_ID			 4 + 1
#define SIZE_NAME		32 + 1
#define SIZE_JOBTITLE	32 + 1
#define SIZE_TEAM		32 + 1
#define SIZE_PHONE		13 + 1
#define SIZE_IP			15 + 1

#define MAX_CNT_ID		1000

#define TABLE_NAME		"EmployeeInfos"
#define ID				"ID"
#define NAME			"NAME"
#define JOBTITLE		"JOBTITLE"
#define TEAM			"TEAM"
#define PHONE			"PHONE"

#define CONFIG_PATH		"/home1/sepp/etc/ini/config.ut"
#define SECTION_NAME	"MP1"
#define ITEM_NAME_IP	"ip_address"
#define ITEM_NAME_PORT	"PORT"

#define KEY_DIR			"/EMPLOYEE_INFOS/"
#define SYSTEM_ID		1
#define SIZE_VALUE		1024

#define DELIM			":"

char *g_pszConfigPath = "/home1/sepp/user/dhkim/FW_2-3/TAP_Config.ini";

typedef enum
{
	NOT_EXIST = 0,			SUCCESS = 1,		INPUT_FAIL = 2,
	ID_EXIST = 3,			ID_NOT_EXIST = 4,
	
	FGETS_FAIL = -3,	TAP_IPC_FAIL = -4,
	NULL_FAIL = -5,			MPGLOG_FAIL = -6,	MPCONF_FAIL = -7,
	OVER_MAX_FAIL = -8,		FUNC_FAIL = -9	

	//TAP_REGI_FAIL = -33 (define in TAP_Registry.h)
} ReturnCode_t;

typedef struct REQUEST_s
{
	int		nMsgType;
	int		nId;
	char	szName		[SIZE_NAME];
	char	szJobTitle	[SIZE_JOBTITLE];
	char	szTeam		[SIZE_TEAM];
	char	szPhone		[SIZE_PHONE];

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
	char	szName		[SIZE_NAME];
} SELECT_ALL_t;

typedef struct SELECT_ONE_s
{
	char	szName		[SIZE_NAME];
	char	szJobTitle	[SIZE_JOBTITLE];
	char	szTeam		[SIZE_TEAM];
	char	szPhone		[SIZE_PHONE];
} SELECT_ONE_t;

int g_nFlag = FLAG_RUN;

#endif /*_TAP_INC_H_*/
