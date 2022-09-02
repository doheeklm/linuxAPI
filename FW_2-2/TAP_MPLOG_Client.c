/* TAP_MPLOG_Client.c */
#include "TAP_MPLOG_Inc.h"

void SignalHandler( int nSig );
void ClearStdin( char *pszTemp );

int	Insert		( struct REQUEST_s *ptRequest );
int Select		( struct REQUEST_s *ptRequest );
int Update		( struct REQUEST_s *ptRequest );
int Delete		( struct REQUEST_s *ptRequest );
void LOG_ERROR	( const char* pszFuncName, int nError );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	REQUEST_t *ptRequest = NULL;
	RESPONSE_t *ptResponse = NULL;
	
	iipc_msg_t tSendMsg;
	iipc_msg_t tRecvMsg;

	iipc_ds_t	tIpc;
	iipc_key_t	tSrcKey;
	iipc_key_t	tDstKey;

	int nRet = 0;
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;

	nRet = MPGLOG_INIT( CLIENT_PROCESS, NULL,
			LOG_MODE_DAILY |
			LOG_MODE_NO_DATE |
			LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_SVC );
	if ( 0 > nRet )
	{
		printf( "MPGLOG_INIT() error\n" );
		return MPGLOG_FAIL;
	}	

	nRet = TAP_ipc_open( &tIpc, CLIENT_PROCESS );
	if ( 0 > nRet )
	{
		LOG_ERROR( __func__, ipc_errno );
		return TAP_FAIL;
	}	

	tDstKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tDstKey )
	{
		LOG_ERROR( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	tSrcKey = TAP_ipc_getkey( &tIpc, CLIENT_PROCESS );
	if ( IPC_NOPROC == tSrcKey )
	{
		LOG_ERROR( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	while ( FLAG_RUN == g_nFlag )
	{
		nRet = 0;
		pszRet = NULL;
		nPickMenu = 0;
		memset( szPickMenu, 0x00, sizeof(szPickMenu) );
		memset( &tSendMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &tRecvMsg, 0x00, sizeof(iipc_msg_t) );

		do
		{
			printf( "=================\n"
					"(1) Insert Info\n"
					"(2) Select Info\n"
					"(3) Update Info\n"
					"(4) Delete Info\n"
					"(5) Exit program\n"
					"=================\n"
					"Input: " );

			pszRet = fgets( szPickMenu, sizeof(szPickMenu), stdin );
			if ( NULL == pszRet )
			{
				LOG_ERROR( __func__, dalErrno() );	
				return FGETS_FAIL;
			}
			ClearStdin( szPickMenu );

			nPickMenu = atoi( szPickMenu );
		} while ( nPickMenu < 1 || nPickMenu > 5);

		ptRequest = (REQUEST_t *)tSendMsg.buf.msgq_buf;	

		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert( ptRequest );
				if ( SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 2:
			{
				nRet = Select( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{	
					goto end_of_function;
				}
			}
				break;
			case 3:
			{
				nRet = Update( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 4:
			{
				nRet = Delete( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 5:
			{
				printf( "[ Menu ] Exit Program\n" );
				goto end_of_function;
			}
				break;
			default:
				break;
		} //switch

		tSendMsg.u.h.src = tSrcKey;
		tSendMsg.u.h.dst = tDstKey;
		tSendMsg.u.h.len = sizeof(struct REQUEST_s);

		MPGLOG_SVC( "[Send Msg to Server] Type: %d | Id: %d | "
				"Name: %s | JobTitle: %s | "
				"Team: %s | Phone: %s",
				ptRequest->nType, ptRequest->nId,
				ptRequest->szName, ptRequest->szJobTitle,
				ptRequest->szTeam, ptRequest->szPhone );

		/*
		 *	Send Message
		 */
		nRet = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			nRet = TAP_ipc_close( &tIpc );
			if ( 0 > nRet )
			{
				LOG_ERROR( __func__, ipc_errno );
			}
			return TAP_FAIL;
		}
	
		/*
		 *	Receive Message
		 */
		nRet = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LOG_ERROR( __func__, ipc_errno );
			nRet = TAP_ipc_close( &tIpc );
			if ( 0 > nRet )
			{
				LOG_ERROR( __func__, ipc_errno );
			}
			return TAP_FAIL;
		}

		ptResponse = (RESPONSE_t *)tRecvMsg.buf.msgq_buf;	
		
		MPGLOG_SVC( "[Recv Msg from Server] Type: %d | Result: %d",
					ptResponse->nType, ptResponse->nResult );

		//TODO 매핑
		//memcpy struct->char array

		SELECT_ALL_t tSelectAll;
		memset( &tSelectAll, 0x00, sizeof(tSelectAll) );

		switch ( ptResponse->nType )
		{
			case 1:
			case 4:
			case 5:
				break;
			case 2: //SELECTALL
			{
				printf( "Select All\n" );
				memcpy( &tSelectAll, ptResponse->szBuffer, sizeof(tSelectAll) );
				printf( "Id: %d\n", tSelectAll.nId );
			}
				break;
			case 3: //SELECTONE
				break;
			default:
				break;
		}

	} //while

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LOG_ERROR( __func__, ipc_errno );
		return TAP_FAIL;
	}

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d", nSig );

	exit( -1 );
}

void ClearStdin( char *pszTemp )
{
	if ( NULL == pszTemp )
	{
		return;
	}

	if ( pszTemp[ strlen(pszTemp) - 1 ] == '\n' )
	{
		pszTemp[ strlen(pszTemp) - 1 ] = '\0';
	}

	__fpurge( stdin );

	return;
}

int Insert( struct REQUEST_s *ptRequest )
{
	if ( NULL != ptRequest )
	{
		memset( ptRequest, 0x00, sizeof(struct REQUEST_s) );
	}
	
	char *pszRet = NULL;

	ptRequest->nType = 1;

	printf( "[%s] Name: ", __func__ );

	pszRet = fgets( ptRequest->szName, sizeof(ptRequest->szName), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szName );

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptRequest->szTeam, sizeof(ptRequest->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptRequest->szPhone, sizeof(ptRequest->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szPhone );
	
	return SUCCESS;
}

int Select( struct REQUEST_s *ptRequest )
{	
	if ( NULL != ptRequest )
	{
		memset( ptRequest, 0x00, sizeof(struct REQUEST_s) );
	}

	int nPickSelect = 0;
	char szPickSelect[8];
	char *pszRet = NULL;

	char szInputId[32];
	char szInputName[32];
	
	memset( szInputId, 0x00, sizeof(szInputId) );
	memset( szInputName, 0x00, sizeof(szInputName) );

	do
	{
		nPickSelect = 0;
		memset( szPickSelect, 0x00, sizeof(szPickSelect) );
	
		printf("=================\n"
			   "(1) Select All\n"
			   "(2) Select One\n"
			   "=================\n"
			   "Input: " );
	
		pszRet = fgets( szPickSelect, sizeof(szPickSelect), stdin );
		if ( NULL == pszRet )
		{
			LOG_ERROR( __func__, dalErrno() );
			return FGETS_FAIL;
		}
		ClearStdin( szPickSelect );

		nPickSelect = atoi( szPickSelect );

	} while ( nPickSelect != 1 && nPickSelect != 2 );

	switch ( nPickSelect )
	{
		case 1:
		{
			ptRequest->nType = 2;
		}
			break;
		case 2:
		{
			ptRequest->nType = 3;

			printf( "[%s] Input ID: ", __func__ );
			
			pszRet = fgets( szInputId, sizeof(szInputId), stdin );
			if ( NULL == pszRet )
			{
				LOG_ERROR( __func__, dalErrno() );
				return FGETS_FAIL;
			}
			ClearStdin( szInputId );

			if ( strlen(szInputId) == 0 )
			{
				printf( "[%s] Input Name: ", __func__ );

				pszRet = fgets( szInputName, sizeof(szInputName), stdin );
				if ( NULL == pszRet )
				{
					LOG_ERROR( __func__, dalErrno() );
					return FGETS_FAIL;
				}
				ClearStdin( szInputName );

				if ( strlen(szInputName) == 0 )
				{
					return INPUT_FAIL;
				}
				else if ( strlen(szInputName) > 0 )
				{
					strlcpy( ptRequest->szName, szInputName, sizeof(szInputName) );
					return SUCCESS;	
				}
			}
			else if ( strlen(szInputId) > 0 )
			{
				if ( atoi(szInputId) > 0 )
				{
					ptRequest->nId = atoi(szInputId);
					return SUCCESS;
				}
				else
				{
					return INPUT_FAIL;
				}
			}
		}
			break;
		default:
			break;
	}

	return SUCCESS;
}

int Update( struct REQUEST_s *ptRequest )
{
	if ( NULL != ptRequest )
	{
		memset( ptRequest, 0x00, sizeof(struct REQUEST_s) );
	}
	
	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	ptRequest->nType = 4;

	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szInput );

	if ( atoi( szInput ) > 0 )
	{
		ptRequest->nId = atoi( szInput );
	}
	else
	{
		return INPUT_FAIL;
	}

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptRequest->szTeam, sizeof(ptRequest->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptRequest->szPhone, sizeof(ptRequest->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szPhone );

	return SUCCESS;
}

int Delete( struct REQUEST_s *ptRequest )
{
	if ( NULL != ptRequest )
	{
		memset( ptRequest, 0x00, sizeof(struct REQUEST_s) );
	}

	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	ptRequest->nType = 5;

	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szInput );

	if ( atoi( szInput ) > 0 )
	{
		ptRequest->nId = atoi( szInput );
	}
	else
	{
		return INPUT_FAIL;
	}	

	return SUCCESS;
}

void LOG_ERROR( const char* pszFuncName, int nError )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nError );
	return;
}
