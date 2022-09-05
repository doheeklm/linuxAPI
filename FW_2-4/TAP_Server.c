/* TAP_Server.c */
#include "TAP_Inc.h"

DAL_CONN	*g_ptConn= NULL;
DAL_PSTMT	*g_ptPstmtInsert = NULL;
DAL_PSTMT	*g_ptPstmtSelectAll = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneById = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneByName = NULL;
DAL_PSTMT	*g_ptPstmtUpdate = NULL;
DAL_PSTMT	*g_ptPstmtDelete = NULL;
DAL_PSTMT	*g_ptPstmtTimer = NULL;

int InitPreparedStatement();
void DestroyPreparedStatement();

int Insert			( struct REQUEST_s *ptRequest );
int SelectAll		( struct RESPONSE_s *ptResponse );
int SelectOne		( struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int Update			( struct REQUEST_s *ptRequest );
int Delete			( struct REQUEST_s *ptRequest );

void LogErr			( const char* pszFuncName, int nErrno );
void SignalHandler	( int nSig );
void TimerHandler	( mptmr_t *ptTmr, void *pvArg, int nNumMiss );
int GetOriginalData	( int nId, char *pszAttribute, char *pszOriginal, int nSize );

void TimerHandler( mptmr_t *ptTmr, void *pvArg, int nNumMiss )
{
	if ( NULL == ptTmr )
		return;

	pvArg = pvArg;
	nNumMiss = nNumMiss;
	
	int nRet = 0;
	int nEmployeeCnt = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	//DONE query로 생성=> select NUMTUPLES from __SYS_TABLES__ where TABLE_NAME='EmployeeInfos';
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtTimer, &ptResult );
	if ( -1 == nRet )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return;
	}
	
	ptEntry = dalFetchFirst( ptResult );
	if ( NULL == ptEntry )
	{
		MPGLOG_ERR( "%s:: dalFetchFirst() fail=%d", __func__, dalErrno() );
		return;
	}
	
	nRet = dalGetIntByKey( ptEntry, NUMTUPLES, &nEmployeeCnt );
	if ( -1 == nRet )
	{
		MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
	}
	
	//DONE log에 저장
	MPGLOG_SVC( "Employee Count: %d", nEmployeeCnt );

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}
	
	return;
}

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
	
	mptmr_t *ptTmr = NULL;
	
	int nRet = 0;
	int nTmrId = 0;

	/*
	 *	MPLOG
	 */
	nRet = MPGLOG_INIT( SERVER_PROCESS, NULL,
			LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_DBG );
	if ( 0 > nRet )
	{
		printf( "%s MPGLOG_INIT() ERROR\n", __func__ );
		return MPGLOG_FAIL;
	}

	/*
	 *	TAP_IPC
	 */
	nRet = TAP_ipc_open( &tIpc, SERVER_PROCESS );
	if ( 0 > nRet )
	{
		LogErr( __func__, ipc_errno );
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		LogErr( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LogErr(  __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	/*
	 *	dalConnect
	 */
	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	/*
	 *	Init PreparedStatement
	 */
	//DONE MultiThread용 _MT 적용
	nRet = InitPreparedStatement();

	if ( SUCCESS != nRet )
	{
		nRet = dalDisconnect( g_ptConn );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
		}
		return DAL_FAIL;
	}

	/*
	 *	Init Timer
	 */
	ptTmr = mptmr_init( TIMER, 0, 100 );
	if ( NULL == ptTmr )
	{
		MPGLOG_ERR( "%s:: mptmr_init() fail", __func__ );
		goto end_of_function;
	}
	
	//DONE INIT mptmr_set_svc_flag 
	nRet = mptmr_set_svc_flag( ptTmr, &g_nTmrFlag );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mptmr_set_svc_flag() fail", __func__ );
		goto end_of_function_tmr;
	}

	//DONE Error Check
	nTmrId = mptmr_new_tmr_id( ptTmr );
	if ( 0 == nTmrId )
	{
		MPGLOG_ERR( "%s:: mptmr_new_tmr_id() fail", __func__ );
		goto end_of_function_tmr;
	}
	MPGLOG_DBG( "%s:: mptmr_new_tmr_id()=%d", __func__, nRet );

	nRet = mptmr_insert( ptTmr, nRet, MPTMR_INFINITE, 3 * 1000, TimerHandler, NULL, NULL );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mptmr_insert() fail", __func__ );
		goto end_of_function_tmr;
	}
	MPGLOG_DBG( "%s:: mptmrinsert()=%d", __func__, nRet );

	nRet = mptmr_start( ptTmr );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s: mptmr_start() fail", __func__ );
		goto end_of_function_tmr;
	}
	MPGLOG_DBG( "%s:: mptmr_start()=%d", __func__, nRet );

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
		nRet = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
			continue;
		}
	
		ptRequest = (REQUEST_t *)tRecvMsg.buf.msgq_buf;
		ptResponse = (RESPONSE_t *)tSendMsg.buf.msgq_buf;

		ptResponse->nMsgType = ptRequest->nMsgType;
		ptResponse->nId = ptRequest->nId;
		
		tSendMsg.u.h.dst = tRecvMsg.u.h.src;
		tSendMsg.u.h.src = tKey;
		tSendMsg.u.h.len = sizeof(struct RESPONSE_s);

		switch ( ptRequest->nMsgType )
		{
			case 1:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d | Name: %s | JobTitle: %s | Team: %s | Phone: %s",
				ptRequest->nMsgType, ptRequest->szName,
				ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone );

				nRet = Insert( ptRequest );
				if ( SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 2:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d", ptRequest->nMsgType );

				nRet = SelectAll( ptResponse );
				if ( ID_NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 3:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d", ptRequest->nMsgType, ptRequest->nId );

				nRet = SelectOne( ptRequest, ptResponse );
				if ( ID_NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}
		
				ptResponse->nResult = 1;
			}
				break;
			case 4:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d | JobTitle: %s | Team: %s | Phone: %s",
					ptRequest->nMsgType, ptRequest->nId, ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone );

				nRet = Update( ptRequest );
				if ( ID_NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 5:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d", ptRequest->nMsgType, ptRequest->nId );
				
				nRet = Delete( ptRequest );
				if ( ID_NOT_EXIST == nRet || SUCCESS != nRet )
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

		if ( ptResponse->nMsgType == 2 )
			MPGLOG_SVC( "[SEND] MsgType: %d | Result: %d | CntSelectAll: %d", ptResponse->nMsgType, ptResponse->nResult, ptResponse->nCntSelectAll );
		else
			MPGLOG_SVC( "[SEND] MsgType: %d | Result: %d", ptResponse->nMsgType, ptResponse->nResult );

		/*
		 *	Send Message
		 */
		nRet = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
			continue;
		}
	}

	nRet = mptmr_stop( ptTmr );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mptmr_stop() fail", __func__ );
	}
	MPGLOG_DBG( "%s:: mptmr_stop()=%d", __func__, nRet );

end_of_function_tmr:
	nRet = mptmr_cancel_all( ptTmr );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mptmr_cancel_all() fail", __func__ );
	}
	MPGLOG_DBG( "%s:: mptmr_cancel_all()=%d", __func__, nRet );

	mptmr_destroy( ptTmr );

end_of_function:	
	DestroyPreparedStatement();

	nRet = dalDisconnect( g_ptConn );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}
	
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LogErr(  __func__, ipc_errno );
		return TAP_FAIL;
	}

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d\n", nSig );

	//DONE mptmr_set_svc_flag set
	g_nTmrFlag = TMR_FLAG_STOP;
	
	exit( -1 );
}

int InitPreparedStatement()
{
	char szQuery[256];
	
	//INSERT
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s);", TABLE_NAME, NAME, JOBTITLE, TEAM, PHONE, NAME, JOBTITLE, TEAM, PHONE );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtInsert = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtInsert )
	{
		LogErr(  __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s from %s;", ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectAll = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY ID
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneById = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneById )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY NAME
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneByName = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneByName )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;", TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtUpdate = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtDelete = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//TIMER
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s from __SYS_TABLES__ where TABLE_NAME='%s'", NUMTUPLES, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';
	
	g_ptPstmtTimer = dalPreparedStatement_MT( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtTimer )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void DestroyPreparedStatement()
{
	int nRet = 0;

	nRet = dalDestroyPreparedStmt( g_ptPstmtInsert );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneById );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneByName );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtUpdate );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtTimer );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
	}

	return;
}

int Insert( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s Parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	nRet = dalSetStringByKey( g_ptPstmtInsert, NAME, ptRequest->szName );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, ptRequest->szJobTitle );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, TEAM, ptRequest->szTeam );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, PHONE, ptRequest->szPhone );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return ID_NOT_EXIST;
	}

	return SUCCESS;
}

int SelectAll( struct RESPONSE_s *ptResponse )
{
	int nRet = 0;
	int nCnt = 0;
	int nGetId = 0;
	char* pszGetName = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;
	
	SELECT_ALL_t tSelectAll;

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectAll, &ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		nRet = dalResFree( ptResult );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}
	
		MPGLOG_ERR( "%s:: NO INFO EXIST", __func__ );
		return ID_NOT_EXIST;
	}

	for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
	{
		memset( &tSelectAll, 0x00, sizeof(tSelectAll) );
		
		nRet = dalGetIntByKey( ptEntry, ID, &nGetId );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}

		nRet = dalGetStringByKey( ptEntry, NAME, &pszGetName );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}	

		tSelectAll.nId = nGetId;
		strlcpy( tSelectAll.szName, pszGetName, sizeof(tSelectAll.szName) );
	
		MPGLOG_DBG( "ID: %d | Name: %s", tSelectAll.nId, tSelectAll.szName );
	
		/* 버퍼사이즈를 초과하여 중간에 메모리가 잘릴 가능성이 있으면 memcpy 하지 않고 break */
		if ( ( (nCnt + 1) * sizeof(tSelectAll) ) > sizeof(ptResponse->szBuffer) )
		{
			MPGLOG_DBG( "현재까지 읽은 사이즈[%ld], 버퍼사이즈[%ld]: 버퍼 사이즈 초과될 가능성이 있어 memcpy 하지 않음", nCnt * sizeof(tSelectAll), sizeof(ptResponse->szBuffer) );
			break;
		}

		memcpy( ptResponse->szBuffer + ( nCnt * sizeof(tSelectAll) ), &tSelectAll, sizeof(tSelectAll) );

		nCnt++;
	}

	ptResponse->nCntSelectAll = nCnt;

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
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

	int nRet = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	char* pszJobTitle = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	struct SELECT_ONE_s tSelectOne;

	if ( ptRequest->nId > 0 )
	{
		nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, ptRequest->nId );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				LogErr( __func__, dalErrno() );
				return DAL_FAIL;
			}
			
			MPGLOG_ERR( "%s:: ID NOT EXIST", __func__ );
			return ID_NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL == ptEntry )
		{
			MPGLOG_ERR( "%s:: dalFetchFirst fail=%d", __func__, dalErrno() );
			goto error_return;
		}

		nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}

		nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}	

		nRet = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}
	}
	else if ( ptRequest->nId == 0 && strlen(ptRequest->szName) > 0 )
	{
		nRet = dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, ptRequest->szName );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneByName, &ptResult );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				LogErr( __func__, dalErrno() );
				return DAL_FAIL;
			}
		
			MPGLOG_ERR( "%s:: NAME_NOT_EXIST", __func__ );
			return NAME_NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL == ptEntry )
		{
			MPGLOG_ERR( "%s:: dalFetchFirst() fail=%d", __func__, dalErrno() );
			goto error_return;
		}
	
		nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}

		nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}	

		nRet = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			goto error_return;
		}
	}
	else if ( ptRequest->nId == 0 && strlen(ptRequest->szName) == 0 )
	{
		MPGLOG_ERR( "%s:: NO INPUT ID/NAME", __func__ );
		return ID_NOT_EXIST;
	}

	strlcpy( tSelectOne.szJobTitle, pszJobTitle, sizeof(tSelectOne.szJobTitle) );
	strlcpy( tSelectOne.szTeam, pszTeam, sizeof(tSelectOne.szTeam) );
	strlcpy( tSelectOne.szPhone, pszPhone, sizeof(tSelectOne.szPhone) );

	memcpy( ptResponse->szBuffer, &tSelectOne, sizeof(tSelectOne) );

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
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

	int nRet = 0;

	if ( strlen(ptRequest->szJobTitle) == 0 )
	{
		nRet = GetOriginalData( ptRequest->nId, JOBTITLE, ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle) );
		if ( SUCCESS != nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptRequest->szTeam) == 0 )
	{
		nRet = GetOriginalData( ptRequest->nId, TEAM, ptRequest->szTeam, sizeof(ptRequest->szTeam) );
		if ( SUCCESS != nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptRequest->szPhone) == 0 )
	{
		nRet = GetOriginalData( ptRequest->nId, PHONE, ptRequest->szPhone, sizeof(ptRequest->szPhone) );
		if ( SUCCESS != nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	nRet = dalSetIntByKey( g_ptPstmtUpdate, ID, ptRequest->nId );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, ptRequest->szJobTitle );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, ptRequest->szTeam );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, PHONE, ptRequest->szPhone );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtUpdate, NULL );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
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

	int nRet = 0;

	nRet = dalSetIntByKey( g_ptPstmtDelete, ID, ptRequest->nId );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtDelete, NULL );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
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
		MPGLOG_ERR( "pszAttribute NULL\n" );
		return NULL_FAIL;
	}

	int nRet = 0;
	char *pszTemp = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	if ( dalSetIntByKey( g_ptPstmtSelectOneById, ID, nId ) == -1 )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		nRet = dalResFree( ptResult );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );
			return DAL_FAIL;
		}

		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return ID_NOT_EXIST;
	}

	ptEntry = dalFetchFirst( ptResult );
	if ( NULL != ptEntry )
	{
		nRet = dalGetStringByKey( ptEntry, pszAttribute, &pszTemp );
		if ( -1 == nRet )
		{
			LogErr( __func__, dalErrno() );

			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				LogErr( __func__, dalErrno() );
			}
			return DAL_FAIL;
		}
		else
		{
			strlcpy( pszOriginal, pszTemp, nSize );
		}
	}

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LogErr( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void LogErr( const char* pszFuncName, int nError )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nError );
	return;
}
