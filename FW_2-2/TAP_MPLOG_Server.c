/* TAP_MPLOG_Server.c */
#include "TAP_MPLOG_Inc.h"

DAL_CONN	*g_ptConn= NULL;
DAL_PSTMT	*g_ptPstmtInsert = NULL;
DAL_PSTMT	*g_ptPstmtSelectAll = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneById = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneByName = NULL;
DAL_PSTMT	*g_ptPstmtUpdate = NULL;
DAL_PSTMT	*g_ptPstmtDelete = NULL;

void SignalHandler( int nSig );
int InitPreparedStatement();
void DestroyPreparedStatement();

int Insert			( struct REQUEST_s *ptRequest );
int SelectAll		( char* pszBuf );
int SelectOne		( struct REQUEST_s *ptRequest, char *pszBuf, int nBufSize );
int Update			( struct REQUEST_s *ptRequest );
int Delete			( struct REQUEST_s *ptRequest );
int GetOriginalData	( int nId, char *pszAttribute, char *pszOriginal, int nSize );
void LOG_ERROR		( const char* pszFuncName, int nError );

//NOTE
//1. 파라미터 포인터 NULL은 MPGLOG_ERR 처리
//2. Request Response 구조체 이름 변경
//3. strncat -> strlcat 변경 (strlcat 사용시 필요한 버퍼 사이즈는 파라미터로 넘겨주기)
//4. TODO SelectAll를 tSendMsg의 버퍼에 memcpy -> 버퍼 문제로 Client가 Server의 Response Msg를 읽지못함

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRet = 0;
	iipc_msg_t tSendMsg;
	iipc_msg_t tRecvMsg;
	iipc_ds_t tIpc;
	iipc_key_t tKey;

	char szBuf[2048];

	REQUEST_t *ptRequest = NULL;
	RESPONSE_t *ptResponse = NULL;

	nRet = MPGLOG_INIT( SERVER_PROCESS, NULL,
			LOG_MODE_DAILY |
			LOG_MODE_NO_DATE |
			LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_SVC );
	if ( 0 > nRet )
	{
		printf( "MPGLOG_INIT() error\n" );
		return MPGLOG_FAIL;
	}	
	nRet = TAP_ipc_open( &tIpc, SERVER_PROCESS );
	if ( 0 > nRet )
	{
		LOG_ERROR( __func__, ipc_errno );
		nRet = dalDisconnect( g_ptConn );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		LOG_ERROR( __func__, ipc_errno );
		goto end_of_function;
	}

	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = InitPreparedStatement();
	if ( SUCCESS != nRet )
	{
		nRet = dalDisconnect( g_ptConn );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
		}
		return DAL_FAIL;
	}

	while ( FLAG_RUN == g_nFlag )
	{
		memset( &tSendMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &tRecvMsg, 0x00, sizeof(iipc_msg_t) );
		memset( szBuf, 0x00, sizeof(szBuf) );
		
		nRet = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			continue;
		}
	
		ptRequest = (REQUEST_t *)tRecvMsg.buf.msgq_buf;
		ptResponse = (RESPONSE_t *)tSendMsg.buf.msgq_buf;

		MPGLOG_SVC( "[Recv Msg from Client] Type: %d | Id: %d | "
					"Name: %s | JobTitle: %s | Team: %s | Phone: %s",
					ptRequest->nType, ptRequest->nId,
					ptRequest->szName, ptRequest->szJobTitle,
					ptRequest->szTeam, ptRequest->szPhone );

		switch ( ptRequest->nType )
		{
			case 1:
			{
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
				nRet = SelectAll( ptResponse->szBuffer );
				
				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 3:
			{
				nRet = SelectOne( ptRequest, ptResponse->szBuffer, sizeof(ptResponse->szBuffer) );
				
				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}
		
				ptResponse->nResult = 1;
			}
				break;
			case 4:
			{
				nRet = Update( ptRequest );
		
				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}

				ptResponse->nResult = 1;
			}
				break;
			case 5:
			{
				nRet = Delete( ptRequest );

				if ( NOT_EXIST == nRet || SUCCESS != nRet )
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

		ptResponse->nType = ptRequest->nType;

		tSendMsg.u.h.dst = tSendMsg.u.h.src;
		tSendMsg.u.h.src = tKey;
/*
		SELECT_ALL_t *ptTemp = NULL;

		ptTemp = (SELECT_ALL_t *)ptResponse->szBuffer;			
		printf("%d | %s\n", ptTemp->nId, ptTemp->szName );		
*/

		MPGLOG_SVC( "[Send Msg to Client] Type: %d | Result: %d",
					ptResponse->nType, ptResponse->nResult );

		nRet = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			continue;
		}
	}

end_of_function:
	nRet = dalDisconnect( g_ptConn );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}
	
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LOG_ERROR(  __func__, ipc_errno );
		return TAP_FAIL;
	}

	MPGLOG_DESTROY();

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
		LOG_ERROR(  __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s from %s;", ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectAll = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY ID
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneById = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneById )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY NAME
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneByName = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneByName )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;", TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtUpdate = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtDelete = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		LOG_ERROR( __func__, dalErrno() );
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
		LOG_ERROR( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneById );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneByName );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtUpdate );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
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
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, ptRequest->szJobTitle );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, TEAM, ptRequest->szTeam );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, PHONE, ptRequest->szPhone );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return NOT_EXIST;
	}

	return SUCCESS;
}

int SelectAll( char* pszBuf )//, int nBufSize )
{
	int nRet = 0;
	int nCnt = 0;
	int nGetId = 0;
	char* pszGetName = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	char szTemp[256];
	memset( szTemp, 0x00, sizeof(szTemp) );
	
	SELECT_ALL_t tSelectAll;
	memset( &tSelectAll, 0x00, sizeof(tSelectAll) );

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectAll, &ptResult );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		nRet = dalResFree( ptResult );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}

		MPGLOG_ERR( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
	{
		nRet = dalGetIntByKey( ptEntry, ID, &nGetId );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			goto error_return;
		}

		nRet = dalGetStringByKey( ptEntry, NAME, &pszGetName );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			goto error_return;
		}	

		tSelectAll.nId = nGetId;
		strlcpy( tSelectAll.szName, pszGetName, sizeof(tSelectAll.szName) );
	
		printf( "[%d] ID: %d | Name: %s\n", nCnt, tSelectAll.nId, tSelectAll.szName );

		memcpy( pszBuf + (nCnt * sizeof(tSelectAll)), &tSelectAll, sizeof(tSelectAll) );

		nCnt++;
	}

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
	}

	return DAL_FAIL;
}

int SelectOne( struct REQUEST_s *ptRequest, char* pszBuf, int nBufSize )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s parameter NULL\n", __func__ );
		return NULL_FAIL;
	}

	int nRet = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	char* pszJobTitle = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	char szTemp[256];
	memset( szTemp, 0x00, sizeof(szTemp) );

	if ( ptRequest->nId > 0 )
	{
		nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, ptRequest->nId );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				return DAL_FAIL;
			}

			MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
			return NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL != ptEntry )
		{
			nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				goto error_return;
			}

			nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				goto error_return;
			}	

			nRet = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				goto error_return;
			}
		}
	}
	else if ( ptRequest->nId == 0 && strlen(ptRequest->szName) != 0 )
	{
		nRet = dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, ptRequest->szName );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneByName, &ptResult );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			goto error_return;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				return DAL_FAIL;
			}

			MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
			return NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL != ptEntry )
		{
			nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				goto error_return;
			}

			nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				goto error_return;
			}	

			nRet = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				goto error_return;
			}
		}
	}
	else if ( ptRequest->nId == 0 && strlen(ptRequest->szName) == 0 )
	{
		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	snprintf( szTemp, sizeof(szTemp), "JobTitle: %s\n", pszJobTitle );
	strlcat( pszBuf, szTemp, nBufSize );

	snprintf( szTemp, sizeof(szTemp), "Team: %s\n", pszTeam );
	strlcat( pszBuf, szTemp, nBufSize );

	snprintf( szTemp, sizeof(szTemp), "Phone: %s\n", pszPhone );
	strlcat( pszBuf, szTemp, nBufSize );

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
	}

	return DAL_FAIL;
}

int Update( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	if ( strlen(ptRequest->szJobTitle) == 0 )
	{
		nRet = GetOriginalData( ptRequest->nId, JOBTITLE, ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptRequest->szTeam) == 0 )
	{
		nRet = GetOriginalData( ptRequest->nId, TEAM, ptRequest->szTeam, sizeof(ptRequest->szTeam) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptRequest->szPhone) == 0 )
	{
		nRet = GetOriginalData( ptRequest->nId, PHONE, ptRequest->szPhone, sizeof(ptRequest->szPhone) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	nRet = dalSetIntByKey( g_ptPstmtUpdate, ID, ptRequest->nId );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, ptRequest->szJobTitle );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, ptRequest->szTeam );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, PHONE, ptRequest->szPhone );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtUpdate, NULL );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	return SUCCESS;
}

int Delete( struct REQUEST_s *ptRequest )
{
	int nRet = 0;

	nRet = dalSetIntByKey( g_ptPstmtDelete, ID, ptRequest->nId );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtDelete, NULL );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
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
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		nRet = dalResFree( ptResult );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}

		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	ptEntry = dalFetchFirst( ptResult );
	if ( NULL != ptEntry )
	{
		nRet = dalGetStringByKey( ptEntry, pszAttribute, &pszTemp );
		if ( -1 == nRet )
		{
			LOG_ERROR( __func__, dalErrno() );

			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				LOG_ERROR( __func__, dalErrno() );
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
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void LOG_ERROR( const char* pszFuncName, int nError )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nError );
	return;
}
