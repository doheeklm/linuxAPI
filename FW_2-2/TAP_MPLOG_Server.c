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

int Insert( struct INFO_s *ptInfo );
int SelectAll( struct INFO_s *ptInfo );
int SelectOne( struct INFO_s *ptInfo );
int Update( struct INFO_s *ptInfo );
int Delete( struct INFO_s *ptInfo );
int GetOriginalData( int nId, char *pszAttribute, char *pszOriginal, int nSize );
int LOG_ERROR( const char* pszFuncName, int nError );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRet = 0;
	int *pnSendMsg = NULL;
	INFO_t *ptInfo = NULL;
	iipc_ds_t tIpc;
	iipc_key_t tKey;
	iipc_msg_t tMsg;

	//TODO Select All 을 위한 메세지 버퍼 구조체 수정

	nRet = MPGLOG_INIT( SERVER_PROCESS, NULL,
			LOG_MODE_DAILY |
			LOG_MODE_REDIRECT_STDOUT |
			LOG_MODE_REDIRECT_STDERR |
			LOG_MODE_NO_DATE |
			LOG_MODE_LEVEL_TAG |
			LOG_MODE_FILE_LINE,
			LOG_LEVEL_DBG );
	if ( 0 > nRet )
	{
		printf( "MPGLOG_INIT() error\n" );
		return MPGLOG_FAIL;
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

	while ( FLAG_RUN == g_nFlag )
	{
		ptInfo = NULL;
		memset( &tMsg, 0x00, sizeof(iipc_msg_t) );

		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			continue;
		}
		
		ptInfo = (INFO_t *)tMsg.buf.msgq_buf;
	
		switch ( ptInfo->nDB )
		{
			case 1:
			{
				nRet = Insert( ptInfo );
				pnSendMsg = (int *)tMsg.buf.msgq_buf;
				if ( SUCCESS != nRet )
				{
					*pnSendMsg = 0;
					MPGLOG_SVC( "[Send Msg to Client] %d\n", *pnSendMsg );
					break;
				}	
				*pnSendMsg = 1;
				MPGLOG_SVC( "[Send Msg to Client] %d\n", *pnSendMsg );
			}
				break;
			case 2:
			{
				nRet = SelectAll( ptInfo );

				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					break;
				}
			
				g_ptSelectAll = (SELECTALL_t *)tMsg.buf.msgq_buf;
				
				MPGLOG_SVC( "[Send Msg to Client]\nId: %d\nName: %s\n",
						g_ptSelectAll->nId, g_ptSelectAll->szName );
			}
				break;
			case 3:
			{
				nRet = SelectOne( ptInfo );

				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					break;
				}

				MPGLOG_SVC( "[Send Msg to Client]\nDB: %d\nId: %d\n"
						"Name: %s\nJobTitle: %s\n"
						"Team: %s\nPhone: %s\n",
						ptInfo->nDB, ptInfo->nId,
						ptInfo->szName, ptInfo->szJobTitle,
						ptInfo->szTeam, ptInfo->szPhone );
			}
				break;
			case 4:
			{
				nRet = Update( ptInfo );
				pnSendMsg = (int *)tMsg.buf.msgq_buf;			
				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					*pnSendMsg = 0;
					MPGLOG_SVC( "[Send Msg to Client] %d\n", *pnSendMsg );
					break;
				}
				*pnSendMsg = 1;
				MPGLOG_SVC( "[Send Msg to Client] %d\n", *pnSendMsg );
			}
				break;
			case 5:
			{
				nRet = Delete( ptInfo );
				pnSendMsg = (int *)tMsg.buf.msgq_buf;
				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					*pnSendMsg = 0;
					MPGLOG_SVC( "[Send Msg to Client] %d\n", *pnSendMsg );
					break;
				}
				*pnSendMsg = 1;
				MPGLOG_SVC( "[Send Msg to Client] %d\n", *pnSendMsg );
			}
				break;
			default:
				break;
		}

		tMsg.u.h.dst = tMsg.u.h.src;
		tMsg.u.h.src = tKey;
	
		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			continue;
		}
	}

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LOG_ERROR(  __func__, ipc_errno );
		return TAP_FAIL;
	}
	
	nRet = dalDisconnect( g_ptConn );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
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

int Insert( struct INFO_s *ptInfo )
{
	if ( NULL == ptInfo )
	{
		printf( "%s ptInfo NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	nRet = dalSetStringByKey( g_ptPstmtInsert, NAME, ptInfo->szName );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, ptInfo->szJobTitle );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, TEAM, ptInfo->szTeam );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, PHONE, ptInfo->szPhone );
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

int SelectAll( struct INFO_s *ptInfo )
{
	if ( NULL == ptInfo )
	{
		MPGLOG_ERR( "%s ptInfo NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	int nIndex = 0;
	int nGetId = 0;
	char* pszGetName = NULL;

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
		if ( nIndex >= MAX_SELECT )
		{
			break;
		}

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

		g_ptSelectAll->nId = nGetId;
		strlcpy( g_ptSelectAll->szName, pszGetName, sizeof(g_ptSelectAll->szName) );

		printf( "Id: %d | Name: %s\n", g_ptSelectAll->nId, g_ptSelectAll->szName );
		
		nIndex++;
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

int SelectOne( struct INFO_s *ptInfo )
{
	if ( NULL == ptInfo )
	{
		MPGLOG_ERR( "%s ptInfo NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	char* pszJobTitle = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	if ( ptInfo->nId > 0 )
	{
		nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, ptInfo->nId );
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

		strlcpy( ptInfo->szJobTitle, pszJobTitle, sizeof(ptInfo->szJobTitle) );
		strlcpy( ptInfo->szTeam, pszTeam, sizeof(ptInfo->szTeam) );
		strlcpy( ptInfo->szPhone, pszPhone, sizeof(ptInfo->szPhone) );
	}
	else if ( ptInfo->nId == 0 && strlen(ptInfo->szName) != 0 )
	{
		nRet = dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, ptInfo->szName );
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

		strlcpy( ptInfo->szJobTitle, pszJobTitle, sizeof(ptInfo->szJobTitle) );
		strlcpy( ptInfo->szTeam, pszTeam, sizeof(ptInfo->szTeam) );
		strlcpy( ptInfo->szPhone, pszPhone, sizeof(ptInfo->szPhone) );
	}
	else if ( ptInfo->nId == 0 && strlen(ptInfo->szName) == 0 )
	{
		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
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

int Update( struct INFO_s *ptInfo )
{
	if ( NULL == ptInfo )
	{
		MPGLOG_ERR( "%s ptInfo NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	if ( strlen(ptInfo->szJobTitle) == 0 )
	{
		nRet = GetOriginalData( ptInfo->nId, JOBTITLE, ptInfo->szJobTitle, sizeof(ptInfo->szJobTitle) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptInfo->szTeam) == 0 )
	{
		nRet = GetOriginalData( ptInfo->nId, TEAM, ptInfo->szTeam, sizeof(ptInfo->szTeam) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptInfo->szPhone) == 0 )
	{
		nRet = GetOriginalData( ptInfo->nId, PHONE, ptInfo->szPhone, sizeof(ptInfo->szPhone) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	nRet = dalSetIntByKey( g_ptPstmtUpdate, ID, ptInfo->nId );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, ptInfo->szJobTitle );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, ptInfo->szTeam );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, PHONE, ptInfo->szPhone );
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

int Delete( struct INFO_s *ptInfo )
{
	int nRet = 0;

	nRet = dalSetIntByKey( g_ptPstmtDelete, ID, ptInfo->nId );
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

int LOG_ERROR( const char* pszFuncName, int nError )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nError );

	return 0;
}
