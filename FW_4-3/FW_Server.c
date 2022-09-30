/* FW_4-3/FW_Server.c */
#include "FW_Inc.h"

int g_nFlag = FLAG_RUN;

int		oammmc_proc_tap_msg				(oammmc_t *mmc, iipc_msg_t *tap_mml_msg);

/*
int		Insert							( struct REQUEST_s *ptRequest);
int		SelectAll						( struct RESPONSE_s *ptResponse );
int		SelectOne						( struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int		Update							( struct REQUEST_s *ptRequest );
int		Delete							( struct REQUEST_s *ptRequest );
*/
void	SignalHandler					( int nSig );
//int	GetOldData						( int nId, char *pszAttribute, char *pszOldData, int nSizeOldData );
//int 	CntCurrentEmployee				( int *pnCntEmployee );
//void	SetAlarmStatus					( int nCntEmployee, int *pnAlarmStatus );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	mpsignal( SIGHUP, SignalHandler ); //1
	mpsignal( SIGINT, SignalHandler ); //2
	mpsignal( SIGQUIT, SignalHandler ); //3
	mpsignal( SIGTERM, SignalHandler ); //15

	int nRC = 0;
	//int nCntEmployee = 0;
	//int nAlarmStatus = 0;

	iipc_ds_t tIpc;
	memset( &tIpc, 0x00, sizeof(tIpc) );
	
	iipc_key_t tIpcKey;
	memset( &tIpcKey, 0x00, sizeof(tIpcKey) );
	
	iipc_msg_t tIpcRecvMsg;
	memset( &tIpcRecvMsg, 0x00, sizeof(tIpcRecvMsg) );

	oammmc_t *ptMmc = NULL;

	DAL_CONN *ptDal= NULL;
	DAL_PSTMT *ptPstmtInsert = NULL;
	DAL_PSTMT *ptPstmtSelectAll = NULL;
	DAL_PSTMT *ptPstmtSelectOne = NULL;
	DAL_PSTMT *ptPstmtUpdate = NULL;
	DAL_PSTMT *ptPstmtDelete = NULL;
	DAL_PSTMT *ptPstmtNumTuples = NULL;

	//char szAlarmModuleInfo[1024];
	//memset( szAlarmModuleInfo, 0x00, sizeof(szAlarmModuleInfo) );

	/*
	 *	MPLOG
	 */
	nRC = MPGLOG_INIT( MODULE, NULL, LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG, LOG_LEVEL_DBG );
	if ( 0 > nRC )
	{
		printf( "%s MPGLOG_INIT() fail\n", __func__ );
		exit( EXIT_FAILURE );
	}

	/*
	 *	IPC
	 */
	nRC = TAP_ipc_open( &tIpc, MODULE );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_open() fail=%d", __func__, ipc_errno );
		exit( EXIT_FAILURE );
	}

	tIpcKey = TAP_ipc_getkey( &tIpc, MODULE );
	if ( IPC_NOPROC == tIpcKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail=%d", __func__, ipc_errno );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		exit( EXIT_FAILURE );
	}

	/*
	 *	MMC
	 */
	nRC = MMC_Init( MODULE, ptMmc, &tIpc );
	if ( SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: MMC_Init() fail=%d", __func__, nRC );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		exit( EXIT_FAILURE );
	}

	/*
	 *	Prepared Statement
	 */
	nRC = PSTMT_Init( ptDal, ptPstmtInsert, ptPstmtSelectAll, ptPstmtSelectOne, ptPstmtUpdate, ptPstmtDelete, ptPstmtNumTuples );
	if ( SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: PSTMT_Init() fail=%d", __func__, nRC );
		nRC = dalDisconnect( ptDal );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		}
		MMC_Destroy( ptMmc );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		exit( EXIT_FAILURE );
	}

	/*
	 *	STAT
	 */
	/*nRC = stgen_open( MODULE, stctl_item_list, stctl_dtl_type_list );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: stgen_open() fail=%d(%s)", __func__, nRC, stctl_strerror(nRC) );
		nRC = dalDisconnect( g_ptConn );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		}
		MMC_Destroy( ptMmc );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		return SYSTEM_FAIL;
	}*/

	/*
	 *	Alarm
	 */
	/*SetAlarmStatus( nCntEmployee, &nAlarmStatus );	

	snprintf( szAlarmModuleInfo, sizeof(szAlarmModuleInfo),
			"UNIT NAME	: %s/%s\n"
			"ITEM NAME	: %s\n"
			"STATUS		: %d\n"
			"CNT EMP	: %d",
			UDA_UPP_GNAME, UDA_LOW_GNAME, UDA_ITEM_NAME, nAlarmStatus, nCntEmployee );
	szAlarmModuleInfo[ strlen(szAlarmModuleInfo) ] = '\0';

	nRC = oam_uda_crte_alarm( &tIpc, UDA_UPP_GNAME, UDA_LOW_GNAME, UDA_ITEM_NAME, nAlarmStatus, OAM_SFM_UDA_NOTI_OFF, szAlarmModuleInfo );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oam_uda_crte_alarm() fail=%d", __func__, nRC );
		stgen_close();
		nRC = dalDisconnect( g_ptConn );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		}
		MMC_Destroy( ptMmc );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		return SYSTEM_FAIL;
	}

	nRC = oam_uda_crte_alarm_noti( &tIpc, UDA_UPP_GNAME, UDA_LOW_GNAME );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oam_uda_crte_alarm_noti() fail=%d", __func__, nRC );
		goto _exit_failure;
	}
*/

	/*nRC = CntCurrentEmployee( &nCntEmployee );
	if ( SUCCESS != nRC )
	{
		goto _exit_failure;
	}*/

	//MPGLOG_DBG( "%s:: Cnt Current Employee=%d", __func__, nCntEmployee );

	/*
	 *	Run
	 */
	while ( FLAG_RUN == g_nFlag )
	{
		memset( &tIpcRecvMsg, 0x00, sizeof(tIpcRecvMsg) );

		/*
		 *	Receive Message
		 */
		nRC = TAP_ipc_msgrcv( &tIpc, &tIpcRecvMsg, IPC_BLOCK );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_msgrcv() fail=%d", __func__, ipc_errno );
			mpthr_sleep_msec(100);
			continue;
		}

		/*
		 *	OAMMMC 메시지인지 확인
		 */
		nRC = oammmc_is_my_tap_msg( ptMmc, tIpcRecvMsg.buf.mtype );
		if ( 1 == nRC )
		{
			//TODO oammmc_mpipc_hdlr를 사용행햐ㅏㅁ
			/*
			 *	실제 MML 메시지 처리
			 */
			nRC = oammmc_proc_tap_msg( ptMmc, &tIpcRecvMsg );
			if ( 1 == nRC ) //MPIPC_HDLR_RET_NOT_FOR_ME 값과 같음
			{
				printf( "not for me\n" );
				goto _exit_failure;
			}
			else if ( 0 == nRC ) //MPIPC_HDLR_RET_DONE
			{
				printf( "success\n" );
				break;
			}
			else if ( 0 > nRC ) //MPIPC_HDLR_RET_ERROR
			{
				printf( "mine but error occurred\n" );
				goto _exit_failure;
			}
		}

		//printf( "size of msg buf=%ld\n", sizeof(tIpcRecvMsg.buf.msgq_buf) );

	/*	SetAlarmStatus( nCntEmployee, &nAlarmStatus );

		snprintf( szAlarmModuleInfo, sizeof(szAlarmModuleInfo),
				"UNIT NAME	: %s/%s\n"
				"ITEM NAME	: %s\n"
				"STATUS		: %d\n"
				"CNT EMP	: %d",
				UDA_UPP_GNAME, UDA_LOW_GNAME,
				UDA_ITEM_NAME,
				nAlarmStatus,
				nCntEmployee );
		szAlarmModuleInfo[ strlen(szAlarmModuleInfo) ] = '\0';

		nRC = oam_uda_rpt_alarm_sts( &tIpc, UDA_UPP_GNAME, UDA_LOW_GNAME, UDA_ITEM_NAME, nAlarmStatus, szAlarmModuleInfo );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oam_uda_rpt_alarm_sts() fail=%d", __func__, nRC );
			return UDA_FAIL;
		}
		MPGLOG_DBG( "%s:: %s|%s|%s|%d", __func__, UDA_UPP_GNAME, UDA_LOW_GNAME, UDA_ITEM_NAME, nAlarmStatus );

		switch ( nRC )
		{
			case SUCCESS:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_SUCCESS, CLIENT_PROCNAME );
			}
				break;
			case NULL_FAIL:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_INVALID_PARAMETER, CLIENT_PROCNAME );
			}
				break;
			case DAL_FAIL:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_SYSTEM_FAIL, CLIENT_PROCNAME );
			}
				break;	
			case ID_NOT_EXIST:
			case NAME_NOT_EXIST:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_UNKNOWN_USER, CLIENT_PROCNAME );
			}
				break;
			default:
				break;
		}	
		
		nCntEmployee++;
		*/		
	}

	PSTMT_Destroy( ptPstmtInsert, ptPstmtSelectAll, ptPstmtSelectOne, ptPstmtUpdate, ptPstmtDelete, ptPstmtNumTuples );

	/*nRC = oam_uda_del_alarm( &tIpc, UDA_UPP_GNAME, UDA_LOW_GNAME, UDA_ITEM_NAME, OAM_SFM_UDA_NOTI_ON );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oam_uda_del_alarm() fail=%d", __func__, nRC );
	}

	stgen_close();
*/
	nRC = dalDisconnect( ptDal );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
	}
	
	MMC_Destroy( ptMmc );
	
	nRC = TAP_ipc_close( &tIpc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
	}

	exit( EXIT_SUCCESS );

_exit_failure:
	PSTMT_Destroy( ptPstmtInsert, ptPstmtSelectAll, ptPstmtSelectOne, ptPstmtUpdate, ptPstmtDelete, ptPstmtNumTuples );
/*
	nRC = oam_uda_del_alarm( &tIpc, UDA_UPP_GNAME, UDA_LOW_GNAME, UDA_ITEM_NAME, OAM_SFM_UDA_NOTI_ON );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oam_uda_del_alarm() fail=%d", __func__, nRC );
	}

	stgen_close();
*/
	nRC = dalDisconnect( ptDal );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
	}
	
	MMC_Destroy( ptMmc );
	
	nRC = TAP_ipc_close( &tIpc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
	}
	
	exit( EXIT_FAILURE );
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d\n", nSig );

	exit( -1 );
}
/*
int Insert( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;		
	}

	int nRC = 0;

	nRC = dalSetStringByKey( g_ptPstmtInsert, NAME, ptRequest->szName );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, POSITION, ptRequest->szPosition );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, TEAM, ptRequest->szTeam );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, PHONE, ptRequest->szPhone );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalPreparedExec( g_ptConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		return ID_NOT_EXIST;
	}

	return SUCCESS;
}

int SelectAll( struct RESPONSE_s *ptResponse )
{
	int nRC = 0;
	int nCnt = 0;
	int nGetId = 0;
	char* pszGetName = NULL;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;
	SELECT_ALL_t tSelectAll;

	nRC = dalPreparedExec( g_ptConn, g_ptPstmtSelectAll, &ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		nRC = dalResFree( ptResult );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}
	
		MPGLOG_ERR( "%s:: no info exist", __func__ );
		return ID_NOT_EXIST;
	}

	for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
	{
		memset( &tSelectAll, 0x00, sizeof(tSelectAll) );
		
		nRC = dalGetIntByKey( ptEntry, ID, &nGetId );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
			goto error_return;
		}

		nRC = dalGetStringByKey( ptEntry, NAME, &pszGetName );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto error_return;
		}	

		tSelectAll.nId = nGetId;
		strlcpy( tSelectAll.szName, pszGetName, sizeof(tSelectAll.szName) );
	
		MPGLOG_DBG( "%s:: ID: %d | Name: %s", __func__, tSelectAll.nId, tSelectAll.szName );
	
		// 버퍼사이즈를 초과하여 중간에 메모리가 잘릴 가능성이 있으면 memcpy 하지 않고 break한다.
		if ( ( (nCnt + 1) * sizeof(tSelectAll) ) > sizeof(ptResponse->szBuffer) )
		{
			MPGLOG_DBG( "%s:: 현재까지 읽은 사이즈[%ld], 버퍼사이즈[%ld]:"
						"버퍼 사이즈 초과될 가능성이 있어 memcpy 하지 않음",
						__func__, nCnt * sizeof(tSelectAll), sizeof(ptResponse->szBuffer) );
			break;
		}

		memcpy( ptResponse->szBuffer + ( nCnt * sizeof(tSelectAll) ), &tSelectAll, sizeof(tSelectAll) );

		nCnt++;
	}

	ptResponse->nCntSelectAll = nCnt;

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}

	return DAL_FAIL;
}

int SelectOne( struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	char* pszPosition = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	struct SELECT_ONE_s tSelectOne;

	if ( ptRequest->nId > 0 )
	{
		nRC = dalSetIntByKey( g_ptPstmtSelectOneById, ID, ptRequest->nId );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRC = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRC )
		{
			nRC = dalResFree( ptResult );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				return DAL_FAIL;
			}
			
			MPGLOG_ERR( "%s:: id not exist", __func__ );
			return ID_NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL != ptEntry )
		{
			nRC = dalGetStringByKey( ptEntry, POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}	

			nRC = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}
		}
	}
	else if ( ptRequest->nId == 0 && strlen(ptRequest->szName) > 0 )
	{
		nRC = dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, ptRequest->szName );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRC = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneByName, &ptResult );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
			goto error_return;
		}
		else if ( 0 == nRC )
		{
			nRC = dalResFree( ptResult );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				return DAL_FAIL;
			}
		
			MPGLOG_ERR( "%s:: name not exist", __func__ );
			return NAME_NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL != ptEntry )
		{
			nRC = dalGetStringByKey( ptEntry, POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}	

			nRC = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}
		}
	}
	else if ( ptRequest->nId == 0 && strlen(ptRequest->szName) == 0 )
	{
		MPGLOG_ERR( "%s:: no input id/name", __func__ );
		return ID_NOT_EXIST;
	}

	strlcpy( tSelectOne.szPosition, pszPosition, sizeof(tSelectOne.szPosition) );
	strlcpy( tSelectOne.szTeam, pszTeam, sizeof(tSelectOne.szTeam) );
	strlcpy( tSelectOne.szPhone, pszPhone, sizeof(tSelectOne.szPhone) );

	memcpy( ptResponse->szBuffer, &tSelectOne, sizeof(tSelectOne) );

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}

	return DAL_FAIL;
}

int Update( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;

	if ( strlen(ptRequest->szName) == 0 )
	{
		nRC = GetOldData( ptRequest->nId, NAME, ptRequest->szName, sizeof(ptRequest->szName) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOldData() fail=%d", __func__, nRC );	
			return nRC;
		}	
	}

	if ( strlen(ptRequest->szPosition) == 0 )
	{
		nRC = GetOldData( ptRequest->nId, POSITION, ptRequest->szPosition, sizeof(ptRequest->szPosition) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOldData() fail=%d", __func__, nRC );	
			return nRC;
		}	
	}

	if ( strlen(ptRequest->szTeam) == 0 )
	{
		nRC = GetOldData( ptRequest->nId, TEAM, ptRequest->szTeam, sizeof(ptRequest->szTeam) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOldData() fail=%d", __func__, nRC );
			return nRC;
		}	
	}

	if ( strlen(ptRequest->szPhone) == 0 )
	{
		nRC = GetOldData( ptRequest->nId, PHONE, ptRequest->szPhone, sizeof(ptRequest->szPhone) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOldData() fail=%d", __func__, nRC );
			return nRC;
		}	
	}

	nRC = dalSetIntByKey( g_ptPstmtUpdate, ID, ptRequest->nId );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtUpdate, NAME, ptRequest->szName );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtUpdate, POSITION, ptRequest->szPosition );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, ptRequest->szTeam );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtUpdate, PHONE, ptRequest->szPhone );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalPreparedExec( g_ptConn, g_ptPstmtUpdate, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_DBG( "%s:: id not exist", __func__ );
		return ID_NOT_EXIST;
	}

	return SUCCESS;
}

int Delete( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;

	nRC = dalSetIntByKey( g_ptPstmtDelete, ID, ptRequest->nId );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );		
		return DAL_FAIL;
	}

	nRC = dalPreparedExec( g_ptConn, g_ptPstmtDelete, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_DBG( "%s:: id not exist", __func__ );
		return ID_NOT_EXIST;
	}

	return SUCCESS;
}

int GetOldData( int nId, char *pszAttribute, char *pszOldData, int nSizeOldData )
{
	if ( NULL == pszAttribute )
	{
		MPGLOG_ERR( "%s:: pszAttribute NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;
	char *pszTemp = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	if ( dalSetIntByKey( g_ptPstmtSelectOneById, ID, nId ) == -1 )
	{
		MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		nRC = dalResFree( ptResult );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}

		MPGLOG_SVC( "[%s] id not exist", __func__ );
		return ID_NOT_EXIST;
	}

	ptEntry = dalFetchFirst( ptResult );
	if ( NULL != ptEntry )
	{
		nRC = dalGetStringByKey( ptEntry, pszAttribute, &pszTemp );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			nRC = dalResFree( ptResult );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
			}
			return DAL_FAIL;
		}
		else
		{
			strlcpy( pszOldData, pszTemp, nSizeOldData );
		}
	}

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}
*/
/*
int CntCurrentEmployee( int *pnCntEmployee )
{
	int nRC = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	nRC = dalPreparedExec( g_ptConn, g_ptPstmtNumtuples, &ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	ptEntry = dalFetchFirst( ptResult );
	if ( NULL == ptEntry )
	{
		MPGLOG_ERR( "%s:: dalFetchFirst() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalGetIntByKey( ptEntry, NUMTUPLES, pnCntEmployee );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void SetAlarmStatus( int nCntEmployee, int *pnAlarmStatus )
{
	if ( 0 <= nCntEmployee && 9 >= nCntEmployee )
	{
		*pnAlarmStatus = NORMAL;
	}
	else if ( 10 <= nCntEmployee && 20 >= nCntEmployee )
	{
		*pnAlarmStatus = MINOR;
	}
	else if ( 21 <= nCntEmployee && 30 >= nCntEmployee )
	{
		*pnAlarmStatus = MAJOR;
	}
	else if ( 31 <= nCntEmployee )	
	{
		*pnAlarmStatus = CRITICAL;
	}
	else
	{
		*pnAlarmStatus = -1;
	}

	MPGLOG_DBG( "%s:: nCntEmployee=%d, nAlarmStatus=%d", __func__, nCntEmployee, *pnAlarmStatus );
}
*/
