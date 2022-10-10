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

//Alarm
#define UDA_UPP_GNAME				"FW_TEST"
#define UDA_LOW_GNAME				"CNT"
#define UDA_ITEM_NAME				"CNT_EMPLOYEE_ALARM"

#define PROCESS_INI					"/home1/sepp/user/dhkim/FW_4-3/MIPCSVR.ini"
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

typedef enum
{
	MMC_SUCCESS = 0, MMC_FAIL = 1,
	SUCCESS = 0, INPUT_FAIL = -1, DAL_FAIL = -2,
	FGETS_FAIL = -3, IPC_FAIL = -4, NULL_FAIL = -5,
	MPGLOG_FAIL = -6, STAT_FAIL = -7, UDA_FAIL = -8,
	OAMMMC_FAIL = -9, MPCONF_FAIL = -10, SYSTEM_FAIL = -11,
	SQL_EXEC_FAIL = 30
} ReturnCode_t;

oammmc_arg_info_t atArgsAdd[] =
{
	{ 1, EMPL_NAME, "<EMPL_NAME>", OAMMMC_STR, EMPL_NAME_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 2, EMPL_POSITION, "<EMPL_POSITION>", OAMMMC_STR, EMPL_POSITION_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, EMPL_TEAM, "<EMPL_TEAM>", OAMMMC_STR, EMPL_TEAM_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, EMPL_PHONE, "<EMPL_PHONE>", OAMMMC_STR, EMPL_PHONE_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsDis[] =
{
	{ 1, EMPL_ID, "<EMPL_ID>", OAMMMC_INT, EMPL_ID_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 2, EMPL_NAME, "<EMPL_NAME>", OAMMMC_STR, EMPL_NAME_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, EMPL_POSITION, "<EMPL_POSITION>", OAMMMC_STR, EMPL_POSITION_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, EMPL_TEAM, "<EMPL_TEAM>", OAMMMC_STR, EMPL_TEAM_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 5, EMPL_PHONE, "<EMPL_PHONE>", OAMMMC_STR, EMPL_PHONE_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsChg[] =
{
	{ 1, EMPL_ID, "<EMPL_ID>", OAMMMC_INT, EMPL_ID_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 2, EMPL_NAME, "<EMPL_NAME>", OAMMMC_STR, EMPL_NAME_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, EMPL_POSITION, "<EMPL_POSITION>", OAMMMC_STR, EMPL_POSITION_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, EMPL_TEAM, "<EMPL_TEAM>", OAMMMC_STR, EMPL_TEAM_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 5, EMPL_PHONE, "<EMPL_PHONE>", OAMMMC_STR, EMPL_PHONE_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsDel[] =
{
	{ 1, EMPL_ID, "<EMPL_ID>", OAMMMC_INT, EMPL_ID_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

DAL_CONN *g_ptDalConn = NULL;
DAL_PSTMT *g_ptPstmtInsert = NULL;
DAL_PSTMT *g_ptPstmtSelectAll = NULL;
DAL_PSTMT *g_ptPstmtUpdate = NULL;
DAL_PSTMT *g_ptPstmtDelete = NULL;
DAL_PSTMT *g_ptPstmtNumTuples = NULL;

int g_nFlag = FLAG_RUN;

#endif /*_FW_INC_H_*/
