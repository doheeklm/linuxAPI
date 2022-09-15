/* FW_4-1/FW_Client.c */
#include "FW_Inc.h"

void SignalHandler( int nSig );
void ClearStdin( char *pszTemp );
void LogErr		( const char* pszFuncName, int nErrno );

int	Insert		( struct REQUEST_s *ptRequest );
int Select		( struct REQUEST_s *ptRequest );
int Update		( struct REQUEST_s *ptRequest );
int Delete		( struct REQUEST_s *ptRequest );

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

	SELECT_ALL_t tSelectAll;
	SELECT_ONE_t *ptSelectOne = NULL;

	int i = 0;
	int nRet = 0;
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;

	iipc_ds_t	tIpc;
	iipc_key_t	tSrcKey;
	iipc_key_t	tDstKey;
	iipc_msg_t tSendMsg;
	iipc_msg_t tRecvMsg;
	
	/*
	 *	MPLOG
	 */
	nRet = MPGLOG_INIT( CLIENT_PROCNAME, NULL,
			LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_DBG );
	if ( 0 > nRet )
	{
		printf( "MPGLOG_INIT() error\n" );
		return MPGLOG_FAIL;
	}	

	/*
	 *	TAP_IPC
	 */
	nRet = TAP_ipc_open( &tIpc, CLIENT_PROCNAME );
	if ( 0 > nRet )
	{
		LogErr( __func__, ipc_errno );
		return TAP_FAIL;
	}	

	tDstKey = TAP_ipc_getkey( &tIpc, SERVER_PROCNAME );
	if ( IPC_NOPROC == tDstKey )
	{
		LogErr( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	tSrcKey = TAP_ipc_getkey( &tIpc, CLIENT_PROCNAME );
	if ( IPC_NOPROC == tSrcKey )
	{
		LogErr( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	/*
	 *	Run Program (Client)
	 */
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
				LogErr( __func__, dalErrno() );	
				return FGETS_FAIL;
			}
			ClearStdin( szPickMenu );

			nPickMenu = atoi( szPickMenu );
		} while ( 1 > nPickMenu || 5 < nPickMenu );

		tSendMsg.u.h.src = tSrcKey;
		tSendMsg.u.h.dst = tDstKey;
		tSendMsg.u.h.len = sizeof(struct REQUEST_s);
		
		tRecvMsg.u.h.src = tSrcKey;
		tRecvMsg.u.h.dst = tDstKey;
		tRecvMsg.u.h.len = sizeof(struct RESPONSE_s);

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

				MPGLOG_SVC( "[SEND] %s: %d | %s: %s | %s: %s| %s: %s | %s: %s",
							MSG_TYPE, ptRequest->nMsgType,
							NAME, ptRequest->szName,
							JOBTITLE, ptRequest->szJobTitle,
							TEAM, ptRequest->szTeam,
							PHONE, ptRequest->szPhone );
			}
				break;
			case 2:
			{
				nRet = Select( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{	
					goto end_of_function;
				}

				if ( ptRequest->nMsgType == 2 )
				{
					MPGLOG_SVC( "[SEND] %s: %d | %s: %s",
								MSG_TYPE, ptRequest->nMsgType, ID, ALL );
				}
				else if ( ptRequest->nMsgType == 3 )	
				{
					MPGLOG_SVC( "[SEND] %s: %d | %s: %d",
								MSG_TYPE, ptRequest->nMsgType, ID, ptRequest->nId );
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

				MPGLOG_SVC( "[SEND] %s: %d | %s: %d | %s: %s | %s: %s | %s: %s | %s: %s",
							MSG_TYPE, ptRequest->nMsgType,
							ID, ptRequest->nId,
							NAME, ptRequest->szName,
							JOBTITLE, ptRequest->szJobTitle,
							TEAM, ptRequest->szTeam,
							PHONE, ptRequest->szPhone ); 
			}
				break;
			case 4:
			{
				nRet = Delete( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{
					goto end_of_function;
				}

				MPGLOG_SVC( "[SEND] %s: %d | %s: %d",
							MSG_TYPE, ptRequest->nMsgType, ID, ptRequest->nId );
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
		}

		/*
		 *	Send Message
		 */
		nRet = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
			nRet = TAP_ipc_close( &tIpc );
			if ( 0 > nRet )
			{
				LogErr( __func__, ipc_errno );
			}
			return TAP_FAIL;
		}
	
		/*
		 *	Receive Message
		 */
		nRet = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
			nRet = TAP_ipc_close( &tIpc );
			if ( 0 > nRet )
			{
				LogErr( __func__, ipc_errno );
			}
			return TAP_FAIL;
		}

		ptResponse = (RESPONSE_t *)tRecvMsg.buf.msgq_buf;	

		switch ( ptResponse->nMsgType )	
		{
			case 2:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %s | Result: %d | CntSelectAll: %d",
							MSG_TYPE, ptResponse->nMsgType, ID, ALL, ptResponse->nResult, ptResponse->nCntSelectAll );

				if ( 1 == ptResponse->nResult )
				{
					for ( i = 0; i < ptResponse->nCntSelectAll; i++ )
					{
						memcpy( &tSelectAll, ptResponse->szBuffer + ( i * sizeof(tSelectAll) ), sizeof(tSelectAll) );

						MPGLOG_SVC( "%s: %d | %s: %s",
									ID, tSelectAll.nId, NAME, tSelectAll.szName );
					}	
				}
			}
				break;
			case 3:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %d | Result: %d",
							MSG_TYPE, ptResponse->nMsgType, ID, ptResponse->nId, ptResponse->nResult );

				if ( 1 == ptResponse->nResult )
				{
					ptSelectOne = (SELECT_ONE_t *)ptResponse->szBuffer;

					MPGLOG_SVC( "%s: %d | %s: %s | %s: %s | %s: %s",
								ID, ptResponse->nId, JOBTITLE, ptSelectOne->szJobTitle,
								TEAM, ptSelectOne->szTeam, PHONE, ptSelectOne->szPhone );
				}	
			}
				break;
			default:
			{
				MPGLOG_SVC( "[RECV] %s: %d | %s: %d | Result: %d",
							MSG_TYPE, ptResponse->nMsgType, ID, ptResponse->nId, ptResponse->nResult );
			}
				break;
		}
	}

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LogErr( __func__, ipc_errno );
		return TAP_FAIL;
	}

	return 0;
}

void LogErr( const char* pszFuncName, int nErrno )
{
	MPGLOG_ERR( "%s:: errno[%d]\n", pszFuncName, nErrno );
	return;
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

	ptRequest->nMsgType = 1;

	printf( "[%s] Name: ", __func__ );

	pszRet = fgets( ptRequest->szName, sizeof(ptRequest->szName), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szName );

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptRequest->szTeam, sizeof(ptRequest->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptRequest->szPhone, sizeof(ptRequest->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
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
			LogErr( __func__, dalErrno() );
			return FGETS_FAIL;
		}
		ClearStdin( szPickSelect );

		nPickSelect = atoi( szPickSelect );

	} while ( nPickSelect != 1 && nPickSelect != 2 );

	switch ( nPickSelect )
	{
		case 1:
		{
			ptRequest->nMsgType = 2;
		}
			break;
		case 2:
		{
			ptRequest->nMsgType = 3;

			printf( "[%s] Input ID: ", __func__ );
			
			pszRet = fgets( szInputId, sizeof(szInputId), stdin );
			if ( NULL == pszRet )
			{
				LogErr( __func__, dalErrno() );
				return FGETS_FAIL;
			}
			ClearStdin( szInputId );

			if ( strlen(szInputId) == 0 )
			{
				printf( "[%s] Input Name: ", __func__ );

				pszRet = fgets( szInputName, sizeof(szInputName), stdin );
				if ( NULL == pszRet )
				{
					LogErr( __func__, dalErrno() );
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

	ptRequest->nMsgType = 4;

	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
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
		LogErr( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptRequest->szTeam, sizeof(ptRequest->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptRequest->szPhone, sizeof(ptRequest->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
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

	ptRequest->nMsgType = 5;

	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, dalErrno() );
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
