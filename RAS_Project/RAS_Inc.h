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
#include <sys/syscall.h>
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

#include "RAS_Err.h"
#include "RAS_Macro.h"
#include "RAS_Config.h"
#include "RAS_Log.h"
#include "RAS_Ipc.h"
#include "RAS_Util.h"
#include "RAS_Regi.h"
#include "RAS_Mmc.h"
#include "RAS_Mmchdl_Ip.h"
#include "RAS_Mmchdl_Trc.h"
#include "RAS_Mmchdl_Info.h"
#include "RAS_DB.h"
#include "RAS_Alarm.h"
#include "RAS_Stat.h"
#include "RAS_Worker.h"
#include "RAS_Socket.h"
#include "RAS_Event.h"
#include "RAS_Http.h"
#include "RAS_Method.h"
#include "RAS_Trace.h"

#define PROCESS_NAME			"MIPCSVR"
#define STOP_SVC				0
#define START_SVC				1
#define ON						1
#define OFF						0
#define MAX_WORKER_CNT			3
#define SIZE_ZERO				0

#endif /* _RAS_INC_H_ */
