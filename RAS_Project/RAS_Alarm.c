/* RAS_Alarm.c */
#include "RAS_Inc.h"

extern mpipc_t *g_ptMpipc;

int ALARM_Init()
{
//	int nRC = 0;

//	int nCnt = 0;	
//	char szInfo[1024];
//	memset( szInfo, 0x00, sizeof(szInfo) );

	ALARM_SetStatus();
/*	
	snprintf( szInfo, sizeof(szInfo),
			"UNIT NAME: %s%s\n"
			"ITEM NAME: %s\n"
			"STATUS: %d\n"
			"CNT: %d",
			UPP_GNAME, LOW_GNAME,
			ITEM_NAME,
			g_nAlarmStatus,
			nCnt );

	nRC = oam_uda_crte_alarm( mpipc_tap_ipc(g_ptMpipc), UPP_GNAME, LOW_GNAME, ITEM_NAME,
			g_nAlarmStatus, OAM_SFM_UDA_NOTI_OFF, szInfo );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "oam_uda_crte_alarm fail <%d>", nRC );
		return RAS_rErrAlarmInit;
	}
*/	
	return RAS_rOK;
}

int ALARM_SetStatus()
{
	return 0;
}
