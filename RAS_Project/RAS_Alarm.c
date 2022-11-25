/* RAS_Alarm.c */
#include "RAS_Inc.h"

extern mpipc_t			*g_ptMpipc;
extern DB_t				g_tDBMain;
extern int				g_nUser;
extern int				g_nAlarmStatus;
extern pthread_mutex_t	g_tMutex;

int ALARM_Init()
{
	int nRC = 0;
	
	char szModuleInfo[1024];
	memset( szModuleInfo, 0x00, sizeof(szModuleInfo) );

	nRC = ALARM_CountUser();
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	ALARM_SetStatus();

	ALARM_SET_MODULE_INFO( szModuleInfo, sizeof(szModuleInfo),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, g_nUser );

	//LOG_DBG_F( "Module Info\n%s", szModuleInfo );
	
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

	DB_PREPARED_EXEC( g_tDBMain, g_tDBMain.patPstmt[PSTMT_NUMTUPLES_INFO], &ptRes, nRC );
	
	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		DB_GET_INT_BY_KEY( ptEntry, NUMTUPLES, &g_nUser, nRC );
	}
	else
	{
		nRC = RAS_rErrDBFetch;
		goto end_of_function;
	}

	DB_FREE( ptRes );
	return RAS_rOK;

end_of_function:
	DB_FREE( ptRes );
	return nRC;
}

void ALARM_SetStatus()
{
	if ( 0 <= g_nUser && ALARM_NORMAL >= g_nUser )
	{
		g_nAlarmStatus = NORMAL;
	}
	else if ( ALARM_NORMAL < g_nUser && ALARM_MINOR >= g_nUser )
	{
		g_nAlarmStatus = MINOR;
	}
	else if ( ALARM_MINOR < g_nUser && ALARM_MAJOR >= g_nUser )
	{
		g_nAlarmStatus = MAJOR;
	}
	else if ( ALARM_MAJOR < g_nUser )
	{
		g_nAlarmStatus = CRITICAL;
	}
	else
	{
		g_nAlarmStatus = -1;
	}
}

int ALARM_Report()
{
	int nRC = 0;
	char szModuleInfo[1024];
	memset( szModuleInfo, 0x00, sizeof(szModuleInfo) );

	pthread_mutex_lock( &g_tMutex );

	ALARM_SetStatus();
	
	ALARM_SET_MODULE_INFO( szModuleInfo, sizeof(szModuleInfo),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, g_nUser );

	ALARM_REPORT_STATUS( mpipc_tap_ipc(g_ptMpipc),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, g_nAlarmStatus, szModuleInfo, nRC );

	LOG_SVC_F( "Count User <%d> Alarm Status <%d>", g_nUser, g_nAlarmStatus );	

	pthread_mutex_unlock( &g_tMutex );

	return RAS_rOK;

end_of_function:
	return nRC;
}
