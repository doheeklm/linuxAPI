/* TAP_MPLOG_Client.c */
#include "TAP_MPLOG_Inc.h"

void SignalHandler( int nSig );
void ClearStdin( char *pszTemp );

int	Insert( struct INFO_s *ptInfo );
int Select( struct INFO_s *ptInfo );
int Update( struct INFO_s *ptInfo );
int Delete( struct INFO_s *ptInfo );

int LOG_ERROR( const char* pszFuncName, int nError );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	INFO_t *ptInfo = NULL;

	iipc_msg_t	tMsg;
	iipc_ds_t	tIpc;
	iipc_key_t	tSrcKey;
	iipc_key_t	tDstKey;

	int nRet = 0;
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;
	int *pnRecvMsg = NULL;

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
		pnRecvMsg = NULL;
		nPickMenu = 0;
		memset( szPickMenu, 0x00, sizeof(szPickMenu) );

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
	
		ptInfo = (INFO_t *)tMsg.buf.msgq_buf;	

		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert( ptInfo );
				if ( SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 2:
			{
				nRet = Select( ptInfo );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{	
					goto end_of_function;
				}
			}
				break;
			case 3:
			{
				nRet = Update( ptInfo );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 4:
			{
				nRet = Delete( ptInfo );
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
		}

		tMsg.u.h.src = tSrcKey;
		tMsg.u.h.dst = tDstKey;
		tMsg.u.h.len = sizeof(struct INFO_s);

		MPGLOG_SVC( "[Send Msg] Length: %d\n", tMsg.u.h.len );

		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
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

		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
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

		switch ( g_nDB )
		{
			case 1: //Insert
			case 4: //Update
			case 5: //Delete
			{
				pnRecvMsg = (int *)tMsg.buf.msgq_buf;
				printf( "[Recv Msg from Server] %d\n", *pnRecvMsg );
			}
				break;
			case 2: //SelectAll
			{
				g_ptSelectAll = (SELECTALL_t *)tMsg.buf.msgq_buf;

				printf( "[Recv Msg from Server]\n"
						"Id: %3d\n"
						"Name: %s'n",
						g_ptSelectAll->nId, g_ptSelectAll->szName );
			}
				break;
			case 3: //SelectOne
			{	
				ptInfo = (INFO_t *)tMsg.buf.msgq_buf;

				if ( ptInfo->nId > 0 )
				{
					printf( "[Recv Msg from Server]\n"
							"DB: %d\n"
							"Id: %d\n"
							"JobTitle: %s\n"
							"Team: %s\n"
							"Phone: %s\n",
							ptInfo->nDB, ptInfo->nId, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone );
				}
				else 
				{
					printf( "[Recv Msg from Server]\n"
							"DB: %d\n"
							"Name: %s\n"
							"JobTitle: %s\n"
							"Team: %s\n"
							"Phone: %s\n",
							ptInfo->nDB, ptInfo->szName, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone );
				}
			}
				break;
			default:
				break;
		}
	}

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

	MPGLOG_SVC( "Signal: %d\n", nSig );

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

int Insert( struct INFO_s *ptInfo )
{
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}
	
	char *pszRet = NULL;

	ptInfo->nDB = 1;
	g_nDB = 1;

	printf( "[%s] Name: ", __func__ );

	pszRet = fgets( ptInfo->szName, sizeof(ptInfo->szName), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szName );

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptInfo->szJobTitle, sizeof(ptInfo->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptInfo->szTeam, sizeof(ptInfo->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptInfo->szPhone, sizeof(ptInfo->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szPhone );
	
	return SUCCESS;
}

int Select( struct INFO_s *ptInfo )
{	
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
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
			ptInfo->nDB = 2;
			g_nDB = 2;
		}
			break;
		case 2:
		{
			ptInfo->nDB = 3;
			g_nDB = 3;

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
					strlcpy( ptInfo->szName, szInputName, sizeof(szInputName) );
				}
			}
			else if ( strlen(szInputId) > 0 )
			{
				if ( atoi(szInputId) > 0 )
				{
					ptInfo->nId = atoi(szInputId);	
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

int Update( struct INFO_s *ptInfo )
{
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}
	
	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	ptInfo->nDB = 4;
	g_nDB = 4;

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
		ptInfo->nId = atoi( szInput );
	}
	else
	{
		return INPUT_FAIL;
	}

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptInfo->szJobTitle, sizeof(ptInfo->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptInfo->szTeam, sizeof(ptInfo->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptInfo->szPhone, sizeof(ptInfo->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LOG_ERROR( __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szPhone );

	return SUCCESS;
}

int Delete( struct INFO_s *ptInfo )
{
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}

	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	ptInfo->nDB = 5;
	g_nDB = 5;

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
		ptInfo->nId = atoi( szInput );
	}
	else
	{
		return INPUT_FAIL;
	}	

	return SUCCESS;
}

int LOG_ERROR( const char* pszFuncName, int nError )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nError );

	return 0;
}
