/* FW_4-4/FW_Header.h */
#ifndef _FW_HEADER_H_
#define _FW_HEADER_H_

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
#include <dal.h>			//Telcobase Dal
#include "mpipc.h"
#include <TAP_Ipc.h>		//TAP Ipc
#include <mplog.h>			//TAP MPLOG
#include "mpsignal.h"
#include "stctl/stctl.h"
#include "test_stat.h"		//OAM Stat
#include "oam_uda.h"		//OAM Alarm
#include "sfm_alarm.h"		//OAM Alarm
#include "oammmc.h"			//OAM
#include "oammmc_mmt.h"		//OAM Mmt
#include "oammmc_mml.h"		//OAM Mml
#include "mpenum.h"			//OAM
#include "trace.h"			//OAM Trace

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
#define TABLE_ATT_ID				"id"
#define TABLE_ATT_NAME				"name"
#define TABLE_ATT_POSITION			"position"
#define TABLE_ATT_TEAM				"team"
#define TABLE_ATT_PHONE				"phone"

#define UDA_UPP_GNAME				"FW_TEST"
#define UDA_LOW_GNAME				"CNT"
#define UDA_ITEM_NAME				"CNT_EMPLOYEE_ALARM"

#define PROCESS_INI					"/home1/sepp/user/dhkim/FW_4-4/MIPCSVR.ini"
#define MMT_PORT					"mmt_port"
#define MMT_ENABLE					"mmt_enable"
#define MML_ENABLE					"mml_enable"
#define MMT_CONN_MAX				"mmt_conn_max"
#define MMT_LOCAL_ONLY				"mmt_local_only"
#define MMT_IS_QUIET				"mmt_is_quiet"

//[SEPP-OMP] ~etc/ini/OAM/command.ut
#define MMC_ADD						"add-empl-info"
#define MMC_ADD_ID					7741
#define MMC_DIS						"dis-empl-info"
#define MMC_DIS_ID					7742
#define MMC_CHG						"chg-emp;-info"
#define MMC_CHG_ID					7743
#define MMC_DEL						"del-emp;-info"
#define MMC_DEL_ID					7744

//[SEPP-OMP] ~etc/ini/OAM/enum.ut
#define EMPL_ID						"EMPL_ID"
#define EMPL_ID_ID					63000
#define EMPL_NAME					"EMPL_NAME"
#define EMPL_NAME_ID				63001
#define EMPL_POSITION				"EMPL_POSITION"
#define EMPL_POSITION_ID			63002
#define EMPL_TEAM					"EMPL_TEAM"
#define EMPL_TEAM_ID				63003
#define EMPL_PHONE					"EMPL_PHONE"
#define EMPL_PHONE_ID				63004

extern DAL_CONN *g_ptDalConn;
extern DAL_PSTMT *g_ptPstmtInsert;
extern DAL_PSTMT *g_ptPstmtSelectAll;
extern DAL_PSTMT *g_ptPstmtDelete;
extern DAL_PSTMT *g_ptPstmtNumTuples;

typedef enum
{
	MMC_HANDLER_SUCCESS = 0,	MMC_FAIL = 1,		DAL_EXEC_ZERO = 2,
	SUCCESS = 0,				INPUT_FAIL = -1,	DAL_FAIL = -2,
	FGETS_FAIL = -3,			IPC_FAIL = -4,		NULL_FAIL = -5,
	MPGLOG_FAIL = -6,			STAT_FAIL = -7,		UDA_FAIL = -8,
	OAMMMC_FAIL = -9,			MPCONF_FAIL = -10,	SYSTEM_FAIL = -11,
	MMC_HANDLER_FAIL = -12
} ReturnCode_t;

void SignalHandler( int nSigno );

int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptIpcMsg, void *pvData );
void IPC_Destroy( mpipc_t *ptMpipc );

int MMC_Init( char *pszModule, oammmc_t *ptOammmc, mpipc_t *ptMpipc );
void MMC_Destroy( oammmc_t *ptOammmc );
int MMC_Handler_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );
int MMC_Handler_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );
int MMC_Handler_Chg( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );
int MMC_Handler_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );

int PSTMT_Init();
void PSTMT_Destroy();

int TRACE_Init();

#endif /*_FW_HEADER_H_*/
