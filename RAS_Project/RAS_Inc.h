/* RAS_Inc.h */
#ifndef _RAS_INC_H_
#define _RAS_INC_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>
#include <stdio_ext.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
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

#include "RAS_Errmsg.h"
#include "RAS_Macro.h"
#include "RAS_Config.h"
#include "RAS_Log.h"
#include "RAS_Ipc.h"
#include "RAS_Mmchdl_CliIp.h"
#include "RAS_Mmchdl_CliIpTrc.h"
#include "RAS_Mmchdl_UsrInfo.h"
#include "RAS_Mmc.h"
#include "RAS_Regi.h"
//#include "RAS_Pstmt.h"
//#include "RAS_DB.h"
#include "RAS_Alarm.h"
#include "RAS_Stat.h"
#include "RAS_Thread.h"
//#include "RAS_Socket.h"
//#include "RAS_Http.h"
//#include "RAS_Json.h"
//#include "RAS_Trace.h"

#define PROCESS_NAME			"MIPCSVR"
#define DO_SVC_STOP				0
#define DO_SVC_START			1

#define SIZE_IP					15

#define TABLE_IP				"RAS_IP"
#define ATT_IP					"cli_ip"
#define ATT_DESC				"desc"

#define TABLE_INFO				"RAS_INFO"
#define ATT_ID					"id"
#define ATT_NAME				"name"
#define ATT_GENDER				"gender"
#define ATT_BIRTH				"birth"
#define ATT_ADDRESS				"address"

#define WORKER_THR_CNT			3

typedef struct
{
	pthread_t nThreadId;
	int nThreadEpollFd;
	char szIp[SIZEIP + 1];
} THREAD_t;

#if 0
#define SIZE_IP_DESC			32
#define SIZE_NAME				32
#define SIZE_BIRTH				6
#define SIZE_ADDRESS			256

#define STATUS_200				"OK"
#define STATUS_201				"Created"
#define STATUS_400				"Bad Request"
#define STATUS_404				"Not Found"
#define STATUS_405				"Method Not Allowed"
#define STATUS_500				"Internal Server Error"

#define HTTP_RESPONSE_MSG		"HTTP/1.1 %d %s\r\n" \
								"Content-Length:%d\r\n\r\n" \
								"%s"
#define HTTP_RESPONSE_MSG_GET	"HTTP/1.1 %d %s\r\n" \
								"Content-Type: application/json\r\n" \
								"Content-Length: %d\r\n\r\n" \
								"%s"
#define BODY_IN_JSON			"{\n    " \
								"\"%s\": %d,\n    " \
								"\"%s\": \"%s\",\n    " \
								"\"%s\": \"%s\",\n    " \
								"\"%s\": \"%s\",\n    " \
								"\"%s\": \"%s\"\n}"

#define REGI_KEY_DIR			"/CLI_IP_TRC"
#define REGI_MAN_SYS_ID			1
#define REGI_VALUE_SIZE			1024
#define REGI_KEY_DELIM			":"
#endif

#endif /* _RAS_INC_H_ */
