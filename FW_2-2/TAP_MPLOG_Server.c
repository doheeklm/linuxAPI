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

int Insert( struct SENDMSG_s *ptSendMsg );
int SelectAll( struct SENDMSG_s *ptSendMsg, char* pszBuffer );
int SelectOne( struct SENDMSG_s *ptSendMsg, char* pszBuffer );
int Update( struct SENDMSG_s *ptSendMsg );
int Delete( struct SENDMSG_s *ptSendMsg );
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
	iipc_ds_t tIpc;
	iipc_key_t tKey;


	//TODO cd etc -> dat ini 상용
	//./test.ini
	iipc_msg_t tMsg; //TODO tRecvMsg tSndMsg


	char szBuf[2048];

	SENDMSG_t *ptSendMsg = NULL;
	RECVMSG_t *ptRecvMsg = NULL;

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
		memset( &tMsg, 0x00, sizeof(iipc_msg_t) );
		memset( szBuf, 0x00, sizeof(szBuf) );
		
		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			continue;
		}
	
		ptSendMsg = (SENDMSG_t *)tMsg.buf.msgq_buf;

		MPGLOG_SVC( "[Recv Msg from Client] Type: %d | Id: %d | "
					"Name: %s | JobTitle: %s | Team: %s | Phone: %s",
					ptSendMsg->nType, ptSendMsg->nId,
					ptSendMsg->szName, ptSendMsg->szJobTitle,
					ptSendMsg->szTeam, ptSendMsg->szPhone );

		switch ( ptSendMsg->nType )
		{
			case 1:
			{
				nRet = Insert( ptSendMsg );
				
				ptRecvMsg = (RECVMSG_t *)tMsg.buf.msgq_buf;

				if ( SUCCESS != nRet )
				{
					ptRecvMsg->nResult = 0;

					break;
				}

				ptRecvMsg->nResult = 1;
			}
				break;
			case 2:
			{
				nRet = SelectAll( ptSendMsg, ptRecvMsg->szBuffer );
			//size TODO	
				ptRecvMsg = (RECVMSG_t *)tMsg.buf.msgq_buf;

				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptRecvMsg->nResult = 0;
					break;
				}
			
				ptRecvMsg->nResult = 1;
				strlcpy( ptRecvMsg->szBuffer, szBuf, sizeof(szBuf) );
			}
				break;
			case 3:
			{
				nRet = SelectOne( ptSendMsg, szBuf );
				
				ptRecvMsg = (RECVMSG_t *)tMsg.buf.msgq_buf;

				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptRecvMsg->nResult = 0;
					break;
				}
		
				ptRecvMsg->nResult = 1;
				strlcpy( ptRecvMsg->szBuffer, szBuf, sizeof(szBuf) );
			}
				break;
			case 4:
			{
				nRet = Update( ptSendMsg );
		
				ptRecvMsg = (RECVMSG_t *)tMsg.buf.msgq_buf;
				
				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptRecvMsg->nResult = 0;
					break;
				}

				ptRecvMsg->nResult = 1;
			}
				break;
			case 5:
			{
				nRet = Delete( ptSendMsg );

				ptRecvMsg = (RECVMSG_t *)tMsg.buf.msgq_buf;

				if ( NOT_EXIST == nRet || SUCCESS != nRet )
				{
					ptRecvMsg->nResult = 0;
					break;
				}

				ptRecvMsg->nResult = 1;
			}
				break;
			default:
				break;
		}

		ptRecvMsg->nType = ptSendMsg->nType;

		tMsg.u.h.dst = tMsg.u.h.src;
		tMsg.u.h.src = tKey;

		MPGLOG_SVC( "[Send Msg to Client] Type: %d | Result: %d\n"
					"%s",
					ptRecvMsg->nType, ptRecvMsg->nResult,
					ptRecvMsg->szBuffer );

		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
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

int Insert( struct SENDMSG_s *ptSendMsg )
{
	if ( NULL == ptSendMsg )
	{
	//TODO LOG ERR
		printf( "%s Parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	nRet = dalSetStringByKey( g_ptPstmtInsert, NAME, ptSendMsg->szName );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, ptSendMsg->szJobTitle );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, TEAM, ptSendMsg->szTeam );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, PHONE, ptSendMsg->szPhone );
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

int SelectAll( struct SENDMSG_s *ptSendMsg, char* pszBuffer )
{
	if ( NULL == ptSendMsg )
	{
		MPGLOG_ERR( "%s parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	int nGetId = 0;
	char* pszGetName = NULL;

	char szTemp[256];
	memset( szTemp, 0x00, sizeof(szTemp) );

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

	SELECTALL_t tS;

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

		tS.nId[nIndex] = nGetId;
		nIndex++;
		
		snprintf( szTemp, sizeof(szTemp), "Id: %d | Name: %s\n", nGetId, pszGetName );
		szTemp[ strlen(szTemp) ] = '\0';

		//TODO Size check
		//TODO strlcat
		//TODO size overflow -갯수 제한
		strncat( pszBuffer, szTemp, sizeof(szTemp) );
	
//nIndex* size
//			memcpy(szBuffer + x , tS, sizeof( SELECTALL_t ) );

	}

	

	pszBuffer[ strlen(pszBuffer) ] = '\0';

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

int SelectOne( struct SENDMSG_s *ptSendMsg, char* pszBuffer )
{
	if ( NULL == ptSendMsg )
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

	if ( ptSendMsg->nId > 0 )
	{
		nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, ptSendMsg->nId );
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
	else if ( ptSendMsg->nId == 0 && strlen(ptSendMsg->szName) != 0 )
	{
		nRet = dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, ptSendMsg->szName );
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
	else if ( ptSendMsg->nId == 0 && strlen(ptSendMsg->szName) == 0 )
	{
		MPGLOG_SVC( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	snprintf( szTemp, sizeof(szTemp), "JobTitle: %s\n", pszJobTitle );
	strncat( pszBuffer, szTemp, sizeof(szTemp) );

	snprintf( szTemp, sizeof(szTemp), "Team: %s\n", pszTeam );
	strncat( pszBuffer, szTemp, sizeof(szTemp) );

	snprintf( szTemp, sizeof(szTemp), "Phone: %s\n", pszPhone );
	strncat( pszBuffer, szTemp, sizeof(szTemp) );

	pszBuffer[ strlen(pszBuffer) ] = '\0';

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

int Update( struct SENDMSG_s *ptSendMsg )
{
	if ( NULL == ptSendMsg )
	{
		MPGLOG_ERR( "%s parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	if ( strlen(ptSendMsg->szJobTitle) == 0 )
	{
		nRet = GetOriginalData( ptSendMsg->nId, JOBTITLE, ptSendMsg->szJobTitle, sizeof(ptSendMsg->szJobTitle) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptSendMsg->szTeam) == 0 )
	{
		nRet = GetOriginalData( ptSendMsg->nId, TEAM, ptSendMsg->szTeam, sizeof(ptSendMsg->szTeam) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	if ( strlen(ptSendMsg->szPhone) == 0 )
	{
		nRet = GetOriginalData( ptSendMsg->nId, PHONE, ptSendMsg->szPhone, sizeof(ptSendMsg->szPhone) );
		if ( SUCCESS != nRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return DAL_FAIL;
		}	
	}

	nRet = dalSetIntByKey( g_ptPstmtUpdate, ID, ptSendMsg->nId );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, ptSendMsg->szJobTitle );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, ptSendMsg->szTeam );
	if ( -1 == nRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, PHONE, ptSendMsg->szPhone );
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

int Delete( struct SENDMSG_s *ptSendMsg )
{
	int nRet = 0;

	nRet = dalSetIntByKey( g_ptPstmtDelete, ID, ptSendMsg->nId );
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
