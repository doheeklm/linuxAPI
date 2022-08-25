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
int Select( struct INFO_s *ptInfo );
int Update( struct INFO_s *ptInfo );
int Delete( struct INFO_s *ptInfo );

//TODO id check

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
	/*
	mplog_t *tMplog = NULL;
	tMplog = MPLOG_INIT( SERVER_PROCESS, "./log.txt", LOG_MODE_FILE_LINE, LOG_LEVEL_SVC );
	if ( NULL == tMplog )
	{
		printf( " mplog_init() error\n" );
		return -1;
	}	
*/
	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	if ( SUCCESS != InitPreparedStatement() )
	{
		exit( -1 );
	}

	nRet = TAP_ipc_open( &tIpc, SERVER_PROCESS );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_open() errno[%d]\n", __func__, ipc_errno );
		
		nRet = dalDisconnect( g_ptConn );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		fprintf( stderr, "%s TAP_ipc_getkey() errno[%d]\n", __func__, ipc_errno );
		goto end_of_function;
	}

	while ( FLAG_RUN == g_nFlag )
	{
		ptInfo = NULL;
		memset( &tMsg, 0x00, sizeof(iipc_msg_t) );

		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgrcv() errno[%d]\n", __func__, ipc_errno );
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
					}
					
					*pnSendMsg = 1;
					printf( "\n[SERVER SEND] %d\n", *pnSendMsg );
				}
				break;
			case 2:
				{
					nRet = Select( ptInfo );
					
					if ( NOT_EXIST == nRet || SUCCESS != nRet )
					{
						pnSendMsg = (int *)tMsg.buf.msgq_buf;
						*pnSendMsg = 0;
						printf( "\n[SERVER SEND] %d\n", *pnSendMsg );
						break;
					}

					//TODO MPLOG_SVC()
					printf( "\n[SERVER SEND]\n"
							"DB: %d\n"
							"Id: %d\n"
							"Name: %s\n"
							"JobTitle: %s\n"
							"Team: %s\n"
							"Phone: %s\n",
							ptInfo->nDB, ptInfo->nId, ptInfo->szName, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone );
				}
				break;
			case 3:
				{
					nRet = Update( ptInfo );
					if ( SUCCESS != nRet && NOT_EXIST == nRet )
					{
						goto end_of_function;
					}
				}
				break;
			case 4:
				{
					nRet = Delete( ptInfo );
					if ( SUCCESS != nRet && NOT_EXIST == nRet )
					{
						goto end_of_function;	
					}
				}
				break;
			default:
				break;
		}

		
		tMsg.u.h.dst = tMsg.u.h.src;
		tMsg.u.h.src = tKey;
	
		//Select All -> struct [10] id name ]

		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgsnd() errno[%d]\n", __func__, ipc_errno );
			continue;
		}
	}

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_close() errno[%d]\n", __func__, ipc_errno );
		return -1;
	}
	
	nRet = dalDisconnect( g_ptConn );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return -1;
	}

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	printf( "\n[SignalHandler] %d\n", nSig );

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
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s from %s;", ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectAll = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY ID
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneById = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneById )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY NAME
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneByName = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneByName )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;", TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtUpdate = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtDelete = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
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
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneById );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneByName );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtUpdate );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
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
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, ptInfo->szJobTitle );
	if ( -1 == nRet )
	{
		printf("error\n");
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, TEAM, ptInfo->szTeam );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, PHONE, ptInfo->szPhone );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		printf( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	return SUCCESS;
}

int Select( struct INFO_s *ptInfo )
{
	if ( NULL == ptInfo )
	{
		printf( "%s ptInfo NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	int nId = 0;
	char* pszName = NULL;
	char* pszJobTitle = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	if ( ptInfo->nId == 0 && strlen(ptInfo->szName) == 0 )
	{
		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectAll, &ptResult );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return DAL_FAIL;
			}

			printf( "[%s] %d Tuple\n", __func__, nRet );
			return NOT_EXIST;
		}

		printf( "\n " );
		for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
		{
			nRet = dalGetIntByKey( ptEntry, ID, &nId );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}

			nRet = dalGetStringByKey( ptEntry, NAME, &pszName );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}	
	
			printf( "[%s] ID: %3d | Name: %s\n", __func__, nId, pszName );
		}

	}
	else if ( ptInfo->nId != 0 && strlen(ptInfo->szName) == 0 )
	{
		nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, ptInfo->nId );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		
		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return DAL_FAIL;
			}

			printf( "[%s] %d Tuple\n", __func__, nRet );
			return NOT_EXIST;
		}

		ptEntry = dalFetchFirst( ptResult );
		if ( NULL != ptEntry )
		{
			nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}

			nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}	

			nRet = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
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
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}

		nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneByName, &ptResult );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRet )
		{
			nRet = dalResFree( ptResult );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return DAL_FAIL;
			}

			printf( "[%s] %d Tuple\n", __func__, nRet );
			return NOT_EXIST;
		}
		
		ptEntry = dalFetchFirst( ptResult );
		if ( NULL != ptEntry )
		{
			nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}

			nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}	
			
			nRet = dalGetStringByKey( ptEntry, PHONE, &pszPhone );
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				goto error_return;
			}
		}

		strlcpy( ptInfo->szJobTitle, pszJobTitle, sizeof(ptInfo->szJobTitle) );
		strlcpy( ptInfo->szTeam, pszTeam, sizeof(ptInfo->szTeam) );
		strlcpy( ptInfo->szPhone, pszPhone, sizeof(ptInfo->szPhone) );
	}

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	
	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	
	return FUNCTION_FAIL;
}

int Update( struct INFO_s *ptInfo )
{
	if ( NULL == ptInfo )
	{
		printf( "%s ptInfo NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	nRet = dalSetIntByKey( g_ptPstmtUpdate, ID, ptInfo->nId );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, ptInfo->szJobTitle );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, ptInfo->szTeam );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, PHONE, ptInfo->szPhone );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtUpdate, NULL );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		printf( "[%s] %d Tuple\n", __func__, nRet );
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
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalPreparedExec( g_ptConn, g_ptPstmtDelete, NULL );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		printf( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}
	
	return SUCCESS;
}
