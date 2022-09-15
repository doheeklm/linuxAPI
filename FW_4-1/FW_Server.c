/* FW_4-1/FW_Server.c */
#include "FW_Inc.h"

int InitPreparedStatement();
void DestroyPreparedStatement();

int Insert			( struct REQUEST_s *ptRequest );
int SelectAll		( struct RESPONSE_s *ptResponse );
int SelectOne		( struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int Update			( struct REQUEST_s *ptRequest );
int Delete			( struct REQUEST_s *ptRequest );

void SignalHandler	( int nSig );
int GetOriginalData	( int nId, char *pszAttribute, char *pszOriginal, int nSize );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	iipc_msg_t tSendMsg;
	iipc_msg_t tRecvMsg;
	iipc_ds_t tIpc;
	iipc_key_t tKey;

	REQUEST_t *ptRequest = NULL;
	RESPONSE_t *ptResponse = NULL;
	
	int nRC = 0;
	int nExit = 0;

	/*
	 *	MPLOG
	 */
	nRC = MPGLOG_INIT( SERVER_PROCNAME, NULL,
			LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_SVC );
	if ( 0 > nRC )
	{
		printf( "%s MPGLOG_INIT() ERROR\n", __func__ );
		return MPGLOG_FAIL;
	}

	/*
	 *	TAP_IPC
	 */
	nRC = TAP_ipc_open( &tIpc, SERVER_PROCNAME );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_open() fail=%d", __func__, ipc_errno );
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCNAME );
	if ( IPC_NOPROC == tKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail=%d", __func__, ipc_errno );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	/*
	 *	STAT
	 */
	nRC = stgen_open( SERVER_PROCNAME, stctl_item_list, stctl_dtl_type_list );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: stgen_open() fail=%d(%s)", __func__, nRC, stctl_strerror(nRC) );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		return STAT_FAIL;
	}

	/*
	 *	dalConnect
	 */
	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		MPGLOG_ERR( "%s:: dalConnect() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = InitPreparedStatement();
	if ( SUCCESS != nRC )
	{
		nRC = dalDisconnect( g_ptConn );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		}
		return DAL_FAIL;
	}

	/*
	 *	Run Program (Server)
	 */
	while ( FLAG_RUN == g_nFlag )
	{
		memset( &tSendMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &tRecvMsg, 0x00, sizeof(iipc_msg_t) );

		/*
		 *	Receive Message
		 */	
		nRC = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );

		STGEN_DTLITEM_1COUNT( TCP_RCV_TOTAL_INV, SERVER_PROCNAME );
		//STGEN_ITEM_1COUNT( TCP_RCV_TOTAL_INV );

		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_msgrcv() fail=%d", __func__, ipc_errno );
			continue;
		}

		ptRequest = (REQUEST_t *)tRecvMsg.buf.msgq_buf;
		ptResponse = (RESPONSE_t *)tSendMsg.buf.msgq_buf;

		ptResponse->nMsgType = ptRequest->nMsgType;
		
		tSendMsg.u.h.dst = tRecvMsg.u.h.src;
		tSendMsg.u.h.src = tKey;
		tSendMsg.u.h.len = sizeof(struct RESPONSE_s);

		switch ( ptRequest->nMsgType )
		{
			case 1:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %s | %s: %s | %s: %s | %s: %s",
							MSG_TYPE, ptRequest->nMsgType,
							NAME, ptRequest->szName,
							JOBTITLE, ptRequest->szJobTitle,
							TEAM, ptRequest->szTeam,
							PHONE, ptRequest->szPhone );

				nRC = Insert( ptRequest );
				if ( SUCCESS != nRC )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 2:
			{
				MPGLOG_SVC( "[RECV] %s: %d",
							MSG_TYPE, ptRequest->nMsgType );

				nRC = SelectAll( ptResponse );
				if ( ID_NOT_EXIST == nRC || SUCCESS != nRC )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 3:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %d",
							MSG_TYPE, ptRequest->nMsgType,
							ID, ptRequest->nId );

				nRC = SelectOne( ptRequest, ptResponse );
				if ( ID_NOT_EXIST == nRC || SUCCESS != nRC )
				{
					ptResponse->nResult = 0;
					break;
				}
				
				ptResponse->nResult = 1;
			}
				break;
			case 4:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %d | %s: %s | %s: %s | %s: %s",
							MSG_TYPE, ptRequest->nMsgType,
							ID, ptRequest->nId,
							JOBTITLE, ptRequest->szJobTitle,
							TEAM, ptRequest->szTeam,
							PHONE, ptRequest->szPhone );

				nRC = Update( ptRequest );
				if ( ID_NOT_EXIST == nRC || SUCCESS != nRC )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 5:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %d",
							MSG_TYPE, ptRequest->nMsgType,
							ID, ptRequest->nId );
				
				nRC = Delete( ptRequest );
				if ( ID_NOT_EXIST == nRC || SUCCESS != nRC )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			default:
				break;
		}
		
		switch ( nRC )
		{
			case SUCCESS:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_SUCCESS, SERVER_PROCNAME );
				//STGEN_ITEM_1COUNT( RET_CODE_SUCCESS );
			}
				break;
			case NULL_FAIL:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_INVALID_PARAMETER, SERVER_PROCNAME );
				//STGEN_ITEM_1COUNT( RET_CODE_INVALID_PARAMETER );
			}
				break;
			case DAL_FAIL:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_SYSTEM_FAIL, SERVER_PROCNAME );
				//STGEN_ITEM_1COUNT( RET_CODE_SYSTEM_FAIL );
			}
				break;	
			case ID_NOT_EXIST:
			case NAME_NOT_EXIST:
			{
				STGEN_DTLITEM_1COUNT( RET_CODE_UNKNOWN_USER, SERVER_PROCNAME );
				//STGEN_ITEM_1COUNT( RET_CODE_UNKNOWN_USER );
			}
				break;
			default:
				break;
		}	

		ptResponse->nId = ptRequest->nId;
		
		if ( 2 == ptResponse->nMsgType )
		{
			MPGLOG_SVC( "[SEND] %s: %d | %s: %s | Result: %d | CntSelectAll: %d",
						MSG_TYPE, ptResponse->nMsgType,
						ID, ALL,
						ptResponse->nResult,
						ptResponse->nCntSelectAll );
		}
		else
		{
			MPGLOG_SVC( "[SEND] %s: %d | %s: %d | Result: %d",
						MSG_TYPE, ptResponse->nMsgType,
						ID, ptResponse->nId,
						ptResponse->nResult );
		}

		/*
		 *	Send Message
		 */
		nRC = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_msgsnd() fail=%d", __func__, ipc_errno );
			continue;
		}

		nExit = 0;
		printf( "서버 종료(1): " );
		nRC = scanf( "%d", &nExit );
		if ( 1 == nExit )
		{
			break;
		}
	}

	nRC = dalDisconnect( g_ptConn );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	
	nRC = TAP_ipc_close( &tIpc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		return TAP_FAIL;
	}

	stgen_close();

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d\n", nSig );

	exit( -1 );
}

int InitPreparedStatement()
{
	char szQuery[256];
	
	//INSERT
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s);", TABLE_NAME, NAME, JOBTITLE, TEAM, PHONE, NAME, JOBTITLE, TEAM, PHONE );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtInsert = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtInsert )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s from %s;", ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectAll = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY ID
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneById = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneById )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY NAME
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneByName = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneByName )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;", TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtUpdate = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtDelete = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void DestroyPreparedStatement()
{
	int nRC = 0;

	nRC = dalDestroyPreparedStmt( g_ptPstmtInsert );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtSelectOneById );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtSelectOneByName );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtUpdate );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	return;
}

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

	nRC = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, ptRequest->szJobTitle );
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
	
		MPGLOG_DBG( "ID: %d | Name: %s", tSelectAll.nId, tSelectAll.szName );
	
		// 버퍼사이즈를 초과하여 중간에 메모리가 잘릴 가능성이 있으면 memcpy 하지 않고 break한다.
		if ( ( (nCnt + 1) * sizeof(tSelectAll) ) > sizeof(ptResponse->szBuffer) )
		{
			MPGLOG_DBG( "현재까지 읽은 사이즈[%ld], 버퍼사이즈[%ld]: 버퍼 사이즈 초과될 가능성이 있어 memcpy 하지 않음", nCnt * sizeof(tSelectAll), sizeof(ptResponse->szBuffer) );
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

	char* pszJobTitle = NULL;
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
			nRC = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
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
			nRC = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
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

	strlcpy( tSelectOne.szJobTitle, pszJobTitle, sizeof(tSelectOne.szJobTitle) );
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

	if ( strlen(ptRequest->szJobTitle) == 0 )
	{
		nRC = GetOriginalData( ptRequest->nId, JOBTITLE, ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOriginalData() fail=%d", __func__, nRC );	
			return FUNC_FAIL;
		}	
	}

	if ( strlen(ptRequest->szTeam) == 0 )
	{
		nRC = GetOriginalData( ptRequest->nId, TEAM, ptRequest->szTeam, sizeof(ptRequest->szTeam) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOriginalData() fail=%d", __func__, nRC );
			return FUNC_FAIL;
		}	
	}

	if ( strlen(ptRequest->szPhone) == 0 )
	{
		nRC = GetOriginalData( ptRequest->nId, PHONE, ptRequest->szPhone, sizeof(ptRequest->szPhone) );
		if ( SUCCESS != nRC )
		{
			MPGLOG_ERR( "%s:: GetOriginalData() fail=%d", __func__, nRC );
			return FUNC_FAIL;
		}	
	}

	nRC = dalSetIntByKey( g_ptPstmtUpdate, ID, ptRequest->nId );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, ptRequest->szJobTitle );
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
		MPGLOG_DBG( "%s:: NO INPUT ID EXIST", __func__ );
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
		MPGLOG_DBG( "%s:: NO INPUT ID EXIST", __func__ );
		return ID_NOT_EXIST;
	}

	return SUCCESS;
}

int GetOriginalData( int nId, char *pszAttribute, char *pszOriginal, int nSize )
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

		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRC );
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
			strlcpy( pszOriginal, pszTemp, nSize );
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
