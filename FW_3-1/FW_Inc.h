/* FW_3-1/FW_Inc.h */
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
#include <mpconf.h>
#include "util.h"

#define SERVER_PROCESS	"MIPCSVR"
#define CLIENT_PROCESS	"MIPCCLI"

#define INSERT			"Insert"
#define SELECT			"Select"
#define UPDATE			"Update"
#define DELETE			"Delete"

#define FLAG_RUN		100
#define FLAG_STOP		99

#define SIZE_ID			4
#define SIZE_NAME		32
#define SIZE_JOBTITLE	32
#define SIZE_TEAM		32
#define SIZE_PHONE		13

#define MAX_CNT_ID		1000

#define TABLE_NAME		"EmployeeInfos"
#define ID				"ID"
#define NAME			"NAME"
#define JOBTITLE		"JOBTITLE"
#define TEAM			"TEAM"
#define PHONE			"PHONE"

#define SIZE_BUCKET		20
#define SIZE_TEMP		1024

char *g_pszFile = "/home1/sepp/user/dhkim/FW_3-1/FW_Config.ini";
char *g_pszBackup = "/home1/sepp/user/dhkim/FW_3-1/FW_Backup.ini";

typedef enum
{
	NOT_EXIST = 0, SUCCESS = 1, INPUT_FAIL = 2,
	FGETS_FAIL = -3, TAP_FAIL = -4, NULL_FAIL = -5,
	MPGLOG_FAIL = -6, MPCONF_FAIL = -7, OVER_MAX_FAIL = -8,
	FUNC_FAIL = -9, LIST_FAIL = -10, HASH_FAIL = -11,
	ID_EXIST = 30, ID_NOT_EXIST = 31,
	FILE_EMPTY = 32, HASH_EMPTY = 33,
	INDEX_NOT_FOUND = -2002
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
	int		nCntSelectAll;
	char	szBuffer	[2048];
} RESPONSE_t;

typedef struct ID_NAME_s
{
	int		nId;
	char	szName		[SIZE_NAME + 1];
} ID_NAME_t;

typedef struct INFO_s
{
	char	szName		[SIZE_NAME + 1];
	char	szJobTitle	[SIZE_JOBTITLE + 1];
	char	szTeam		[SIZE_TEAM + 1];
	char	szPhone		[SIZE_PHONE + 1];
} INFO_t;

int g_nFlag = FLAG_RUN;

#endif /*_FW_INC_H_*/
