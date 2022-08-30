/* TAP_MPLOG_Inc.h */

#ifndef _TAP_MPLOG_INC_H_
#define _TAP_MPLOG_INC_H_

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

#define SERVER_PROCESS	"MIPCSVR" //TAP IPC open시 사용하는 Process name
#define CLIENT_PROCESS	"MIPCCLI"

#define INSERT			"Insert"
#define SELECT			"Select"
#define UPDATE			"Update"
#define DELETE			"Delete"

#define FLAG_RUN		100
#define FLAG_STOP		99

#define SIZE_NAME		32
#define SIZE_JOBTITLE	32
#define SIZE_TEAM		32
#define SIZE_PHONE		13
#define MAX_SELECT		20

#define TABLE_NAME		"EmployeeInfos"
#define ID				"id"
#define NAME			"name"
#define JOBTITLE		"jobTitle"
#define TEAM			"team"
#define PHONE			"phone"

typedef enum
{
	NOT_EXIST = 0, SUCCESS = 1, INPUT_FAIL = 2,
	DAL_FAIL = -2, FGETS_FAIL = -3, TAP_FAIL = -4,
	NULL_FAIL = -5, MPGLOG_FAIL = -6
} ReturnCode_t;

//TODO REQUEST RESPONSE
typedef struct SENDMSG_s //Client->Server
{
	int		nType;
	int		nId;
	char	szName		[SIZE_NAME + 1];
	char	szJobTitle	[SIZE_JOBTITLE + 1];
	char	szTeam		[SIZE_TEAM + 1];
	char	szPhone		[SIZE_PHONE + 1];

} SENDMSG_t;

typedef struct RECVMSG_s //Server->Client
{
	int		nType;
	int		nResult;
	char	szBuffer	[2048];
} RECVMSG_t;

int g_nFlag = FLAG_RUN;

#endif /*_TAP_MPLOG_INC_H_*/
