/* RAS_Alarm.c */
#include "RAS_Inc.h"

extern mpipc_t *g_ptMpipc;
extern DB_t g_tDBMain;
extern int g_nUser;
extern int g_nAlarmStatus;

int ALARM_Init()
{
	int nRC = 0;

	char szModuleInfo[1024];
	memset( szModuleInfo, 0x00, sizeof(szModuleInfo) );

	nRC = ALARM_CountUser();
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrFail;
	}

	ALARM_SetStatus();

	ALARM_SET_MODULE_INFO( szModuleInfo, sizeof(szModuleInfo),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, g_nUser );

	ALARM_CREATE( mpipc_tap_ipc(g_ptMpipc),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, szModuleInfo, nRC );

	ALARM_CREATE_NOTI( mpipc_tap_ipc(g_ptMpipc), UPP_GNAME, LOW_GNAME, nRC );

	return RAS_rOK;

end_of_function:
	return nRC;
}

int ALARM_CountUser()
{
	int nRC = 0;
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	DB_PREPARED_EXEC( g_tDBMain, g_tDBMain.patPstmt[PSTMT_NUMTUPLE_INFO], &ptRes, nRC );
	if ( NULL != ptEntry )
	{
		DB_GET_INT_BY_KEY( ptEntry, NUMTUPLE, &g_nUser, nRC );
	}

	LOG_DBG_F( "User %d", g_nUser );
	
	DB_FREE( ptRes );
	return RAS_rOK;

end_of_function:
	DB_FREE( ptRes );
	return nRC;
}

void ALARM_SetStatus()
{
	if ( 0 <= g_nUser && 11 > g_nUser )
	{
		g_nAlarmStatus = NORMAL;
	}
	else if ( 11 <= g_nUser && 21 > g_nUser )
	{
		g_nAlarmStatus = MINOR;
	}
	else if ( 21 <= g_nUser && 31 > g_nUser )
	{
		g_nAlarmStatus = MAJOR;
	}
	else if ( 31 <= g_nUser )
	{
		g_nAlarmStatus = CRITICAL;
	}
	else
	{
		g_nAlarmStatus = -1;
	}

	LOG_DBG_F( "Alarm Status %d", g_nAlarmStatus );
}

int ALARM_Report()
{
	int nRC = 0;
	char szModuleInfo[1024];
	memset( szModuleInfo, 0x00, sizeof(szModuleInfo) );

	ALARM_SetStatus();

	ALARM_SET_MODULE_INFO( szModuleInfo, sizeof(szModuleInfo),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, g_nUser );

	ALARM_REPORT_STATUS( mpipc_tap_ipc(g_ptMpipc),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, szModuleInfo, nRC );
	
	return RAS_rOK;

end_of_function:
	return nRC;
}
