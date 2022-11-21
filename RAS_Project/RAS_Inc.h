/* RAS_Inc.h */
#ifndef _RAS_INC_H_
#define _RAS_INC_H_

#include <stdio.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>
#include <dal.h>
#include "mpipc.h"
#include <TAP_Ipc.h>
#include <mplog.h>
#include "mpsignal.h"
#include "stctl/stctl.h"
#include "oam_uda.h"
#include "sfm_alarm.h"
#include "oammmc.h"
#include "oammmc_mmt.h"
#include "oammmc_mml.h"
#include "mpenum.h"
#include "trace.h"
#include "TAP_Registry.h"
#include "TAP_Registry_udp.h"
#include "TAP_Registry_P.h"
#include "mpconf.h"
#include "mpdal.h"

#define PROCESS_NAME			"MIPCSVR"
#define STOP_SVC				0
#define START_SVC				1
#define ON						1
#define OFF						0
#define MAX_WORKER_CNT			3
#define MAX_TRC_CNT				1000
#define EMPTY_STRING			""
#define SIZE_HEADER				1024
#define SIZE_METHOD				16
#define SIZE_PATH				128
#define SIZE_BODY				1024
#define SIZE_MSG				2048
#define SIZE_IP					15
#define SIZE_TM					5
#define SIZE_NAME				32
#define SIZE_GENDER				6
#define SIZE_BIRTH				6
#define SIZE_ADDRESS			256

typedef struct WORKER_s
{
	pthread_t	nThreadId;
	int			nEpollFd;
	char		szClientIp	[SIZE_IP + 1];
} WORKER_t;

typedef struct HTTP_REQUEST_s
{
	char		szHeader	[SIZE_HEADER + 1];
	char		szMethod	[SIZE_METHOD + 1];
	char		szPath		[SIZE_PATH + 1];
	int			nContentLength;
	char		szBody		[SIZE_BODY + 1];
} HTTP_REQUEST_t;

typedef struct HTTP_RESPONSE_s
{
	int			nStatusCode;
	int			nContentLength;
	char		szBody		[SIZE_BODY + 1];
	char		szMsg		[SIZE_MSG + 1];
} HTTP_RESPONSE_t;

typedef struct TRC_s
{
	char		szClientIp	[SIZE_IP + 1];
	char		szPeriodTm	[SIZE_TM + 1];;
	int			nCnt;
} TRC_t;

typedef struct USER_s
{
	int			nId;
	char		szName		[SIZE_NAME + 1];
	char		szGender	[SIZE_GENDER + 1];
	char		szBirth		[SIZE_BIRTH + 1];
	char		szAddress	[SIZE_ADDRESS + 1];
} USER_t;

#include "RAS_Err.h"
#include "RAS_Macro.h"
#include "RAS_Config.h"
#include "RAS_Log.h"
#include "RAS_Ipc.h"
#include "RAS_DB.h"
#include "RAS_Util.h"
#include "RAS_Regi.h"
#include "RAS_Mmc.h"
#include "RAS_Mmchdl_Ip.h"
#include "RAS_Mmchdl_Trc.h"
#include "RAS_Mmchdl_Info.h"
#include "RAS_Alarm.h"
#include "RAS_Stat.h"
#include "RAS_Worker.h"
#include "RAS_Socket.h"
#include "RAS_Event.h"
#include "RAS_Http.h"
#include "RAS_Method.h"
#include "RAS_Trace.h"

#endif /* _RAS_INC_H_ */
