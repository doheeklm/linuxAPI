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
#include <dal.h>
#include "mpipc.h"
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
#include "trace.h"
#include "TAP_Registry.h"
#include "TAP_Registry_udp.h"
#include "TAP_Registry_P.h"			//$TAP_HOME/include

#include "mpdal.h"

/* MODULE */
#define PROCNAME_SERVER				"MIPCSVR"
#define PROCNAME_CLIENT				"MIPCCLI"

/* DB SQL */
#define INSERT						"Insert"
#define SELECT						"Select"
#define UPDATE						"Update"
#define DELETE						"Delete"
#define NUMTUPLES					"NUMTUPLES"
#define TABLE_NAME					"EmployeeInfos"
#define TABLE_ATT_ID				"id"
#define TABLE_ATT_NAME				"name"
#define TABLE_ATT_POSITION			"position"
#define TABLE_ATT_TEAM				"team"
#define TABLE_ATT_PHONE				"phone"

/* MSGTYPE */
#define MTYPE_INSERT				1
#define MTYPE_SELECTALL				21
#define MTYPE_SELECTONE				22
#define MTYPE_UPDATE				3
#define MTYPE_DELETE				4

/* SIGNAL */
#define FLAG_RUN					100
#define FLAG_STOP					99

/* INFO SIZE */
#define SIZE_ID						4  //ex: 1000
#define SIZE_NAME					32
#define SIZE_POSITION				32
#define SIZE_TEAM					32
#define SIZE_PHONE					11 //ex: 01012345678
#define SIZE_IP						15 //ex: 000.000.000.000

/* ALARM */
#define UDA_UPP_GNAME				"FW_TEST"
#define UDA_LOW_GNAME				"CNT"
#define UDA_ITEM_NAME				"CNT_EMPLOYEE_ALARM"

/* CONFIG */
#define CONF_PATH					"/home1/sepp/etc/ini/config.ut"
#define CONF_SECTION_NAME			"MP1"
#define CONF_ITEM_NAME_IP			"ip_address"
#define CONF_ITEM_NAME_PORT			"PORT"

/* REGISTRY */
#define REGI_KEY_DIR				"/EMPLOYEE_TRACE"
#define REGI_MAN_SYSTEM_ID			1
#define REGI_VALUE_SIZE				1024
#define REGI_KEY_DELIM				":"

/* MMC */
#define PROCESS_INI					"/home1/sepp/user/dhkim/FW_4-4/MIPCSVR.ini"
#define MMT_PORT					"mmt_port"
#define MMT_ENABLE					"mmt_enable"
#define MML_ENABLE					"mml_enable"
#define MMT_CONN_MAX				"mmt_conn_max"
#define MMT_LOCAL_ONLY				"mmt_local_only"
#define MMT_IS_QUIET				"mmt_is_quiet"

/* [SEPP-OMP] ~etc/ini/OAM/command.ut */
#define MMC_ADD_INFO				"add-empl-info"
#define MMC_ADD_INFO_ID				7741
#define MMC_DIS_INFO				"dis-empl-info"
#define MMC_DIS_INFO_ID				7742
#define MMC_CHG_INFO				"chg-empl-info"
#define MMC_CHG_INFO_ID				7743
#define MMC_DEL_INFO				"del-empl-info"
#define MMC_DEL_INFO_ID				7744
#define MMC_ADD_TRC					"add-empl-trace"
#define MMC_ADD_TRC_ID				7745
#define MMC_DIS_TRC					"dis-empl-trace"
#define MMC_DIS_TRC_ID				7746
#define MMC_DEL_TRC					"del-empl-trace"
#define MMC_DEL_TRC_ID				7747

/* [SEPP-OMP] ~etc/ini/OAM/enum.ut */
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
#define EMPL_TRC_KEY				"EMPL_TRC_KEY"
#define EMPL_TRC_KEY_ID				63005
#define PERIOD_TM					"PERIOD_TM"
#define PERIOD_TM_ID				63006

extern DAL_CONN *g_ptDalConn;
extern DAL_PSTMT *g_ptPstmtInsert;
extern DAL_PSTMT *g_ptPstmtSelectAll;
extern DAL_PSTMT *g_ptPstmtDelete;
extern DAL_PSTMT *g_ptPstmtNumTuples;
extern mpipc_t *g_ptMpipc;

typedef enum
{
	MMC_HANDLER_SUCCESS		= 0, //must be 0
	RC_SUCCESS				= 1,
	REGI_SUCCESS			= 2,
	REGI_KEY_EXIST			= 3,
	REGI_KEY_NOT_EXIST		= 4,
	DAL_EXEC_ZERO			= -1,
	DAL_FAIL				= -2,
	INPUT_FAIL				= -3,
	DB_FAIL					= -4,
	IPC_FAIL				= -5,
	NULL_FAIL				= -6,
	MPGLOG_FAIL				= -7,
	STAT_FAIL				= -8,	
	UDA_FAIL				= -9,
	OAMMMC_FAIL				= -10,
	MPCONF_FAIL				= -11,
	SYSTEM_FAIL				= -12,
	MMC_HANDLER_FAIL		= -13,
	RC_FAIL					= -14,
	INPUT_MENU_FAIL			= -15,
	REGI_FAIL				= -16,
	DATA_OVERFLOW_FAIL		= -17
} ReturnCode_e;

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
	int		nRC;
	char	szBuffer	[2048];
	int		nCntSelectAll;
} RESPONSE_t;

typedef struct SELECT_s
{
	int		nId;
	char	szName		[SIZE_NAME + 1];
	char	szPosition	[SIZE_POSITION + 1];
	char	szTeam		[SIZE_TEAM + 1];
	char	szPhone		[SIZE_PHONE + 1];
} SELECT_t;

/* FW_Server.c, FW_Client.c */
void SignalHandler( int nSigno );

/* FW_Ipc.c */
int		IPC_Handler( mpipc_t *g_ptMpipc, iipc_msg_t *ptRecvMsg, void *pvData );
void	IPC_Destroy( mpipc_t *g_ptMpipc );

/* FW_Mmc.c */
int		MMC_Init( char *pszModule, oammmc_t *ptOammmc, mpipc_t *g_ptMpipc );
int		MMC_Handler_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int		MMC_Handler_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int		MMC_Handler_Chg( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int		MMC_Handler_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int		MMC_Handler_AddTrace( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int		MMC_Handler_DisTrace( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int		MMC_Handler_DelTrace( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
void	MMC_Destroy( oammmc_t *ptOammmc );
void	MMC_PrintLine( oammmc_t *ptOammmc );

/* FW_Pstmt.c */
int		PSTMT_Init();
void	PSTMT_Destroy();

/* FW_DB.c */
int		DB_CheckDuplicate( char *pszPhone );
int		DB_Insert( struct REQUEST_s *ptRequestFromClient );
int		DB_Select( struct REQUEST_s *ptRequestFromClient,
		struct RESPONSE_s *ptResponseToClient );
int		DB_Update( struct REQUEST_s *ptRequestFromClient );
int		DB_Delete( struct REQUEST_s *ptRequestFromClient );

/* FW_Util.c */
int		UTIL_GetConfig( char *pszIp, int *pnPort, int nSizeIp );
void	UTIL_FreeList( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList,
		mpconf_list_t *ptValueList );

/* FW_Regi.c */
int		REGI_CheckKey( int nId );

/* FW_Trace.c */
int		TRACE_MakeTrace( struct REQUEST_s *ptReqeustFromClient );

/* FW_Client.c */
void	ClearStdin( char *pszTemp );
int		Insert( struct REQUEST_s *ptRequestToServer );
int		Select( struct REQUEST_s *ptRequestToServer );
int		Update( struct REQUEST_s *ptRequestToServer );
int		Delete( struct REQUEST_s *ptRequestToServer );

#endif /*_FW_HEADER_H_*/
