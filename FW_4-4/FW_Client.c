/* FW_Client.c */
#include "FW_Header.h"

int g_nFlag = FLAG_RUN;

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler );
	tb_signal( SIGINT, SignalHandler );
	tb_signal( SIGQUIT, SignalHandler );
	tb_signal( SIGTERM, SignalHandler );

	REQUEST_t *ptRequestToServer = NULL;
	RESPONSE_t *ptResponseFromServer = NULL;

	SELECT_t tSelect;

	int i = 0;
	int nRet = 0;
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;

	iipc_ds_t	tIpc;
	iipc_key_t	tSrcKey = IPC_NOPROC;
	iipc_key_t	tDstKey = IPC_NOPROC;
	iipc_msg_t	tSendMsg;
	iipc_msg_t	tRecvMsg;
	
	/*
	 *	MPLOG
	 */
	nRet = MPGLOG_INIT( PROCNAME_CLIENT, NULL, LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG,
						LOG_LEVEL_DBG );
	if ( 0 > nRet )
	{
		printf( "MPGLOG_INIT() error\n" );
		return MPGLOG_FAIL;
	}	

	/*
	 *	IPC
	 */
	nRet = TAP_ipc_open( &tIpc, PROCNAME_CLIENT );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_open() fail=%d", __func__, ipc_errno );
		return IPC_FAIL;
	}	

	tDstKey = TAP_ipc_getkey( &tIpc, PROCNAME_SERVER );
	if ( IPC_NOPROC == tDstKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail=%d", __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		return IPC_FAIL;
	}

	tSrcKey = TAP_ipc_getkey( &tIpc, PROCNAME_CLIENT );
	if ( IPC_NOPROC == tSrcKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail=%d", __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		}
		return IPC_FAIL;
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
					"(5) Exit Program\n"
					"=================\n"
					"Input: " );

			pszRet = fgets( szPickMenu, sizeof(szPickMenu), stdin );
			if ( NULL == pszRet )
			{
				MPGLOG_ERR( "%s:: fgets() fail", __func__ );
				return INPUT_MENU_FAIL;
			}
			ClearStdin( szPickMenu );

			nPickMenu = atoi( szPickMenu );

		} while ( 1 > nPickMenu || 5 < nPickMenu );

		tSendMsg.buf.mtype = 0;

		tSendMsg.u.h.src = tSrcKey;
		tSendMsg.u.h.dst = tDstKey;
		tSendMsg.u.h.len = sizeof(struct REQUEST_s);
		
		tRecvMsg.u.h.src = tSrcKey;
		tRecvMsg.u.h.dst = tDstKey;
		tRecvMsg.u.h.len = sizeof(struct RESPONSE_s);

		ptRequestToServer = (REQUEST_t *)tSendMsg.buf.msgq_buf;	

		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert( ptRequestToServer );
				if ( RC_SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 2:
			{
				nRet = Select( ptRequestToServer );
				if ( RC_SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 3:
			{
				nRet = Update( ptRequestToServer );
				if ( RC_SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 4:
			{
				nRet = Delete( ptRequestToServer );
				if ( RC_SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 5:
			{
				goto end_of_function;
			}
				break;
			default:
				break;
		}

		/*
		 *	Send Message To Server
		 */
		nRet = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_msgsnd() fail=%d", __func__, ipc_errno );
			nRet = TAP_ipc_close( &tIpc );
			if ( 0 > nRet )
			{
				MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
			}
			return IPC_FAIL;
		}
	
		/*
		 *	Receive Message From Server
		 */
		nRet = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: TAP_ipc_msgrcv() fail=%d", __func__, ipc_errno );
			nRet = TAP_ipc_close( &tIpc );
			if ( 0 > nRet )
			{
				MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
			}
			return IPC_FAIL;
		}

		ptResponseFromServer = (RESPONSE_t *)tRecvMsg.buf.msgq_buf;	

		switch ( ptResponseFromServer->nMsgType )
		{
			case MTYPE_SELECTALL:
			case MTYPE_SELECTONE:
			{
				MPGLOG_DBG( "[RECV] mtype = %d | nRC = %d | nCntSelectAll = %d",
							ptResponseFromServer->nMsgType, ptResponseFromServer->nRC,
							ptResponseFromServer->nCntSelectAll );

				if ( RC_SUCCESS == ptResponseFromServer->nRC )
				{
					MPGLOG_SVC( "%4s %32s %32s %32s %11s",
							TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION,
							TABLE_ATT_TEAM, TABLE_ATT_PHONE );

					for ( i = 0; i < ptResponseFromServer->nCntSelectAll; i++ )
					{
						memcpy( &tSelect, ptResponseFromServer->szBuffer+(i*sizeof(tSelect)), sizeof(tSelect) );
						
						MPGLOG_SVC( "%4d %32s %32s %32s %11s",
								tSelect.nId, tSelect.szName, tSelect.szPosition,
								tSelect.szTeam, tSelect.szPhone );
					}	
				}
			}
				break;
			default:
			{
				MPGLOG_DBG( "[RECV] mtype = %d | nRC = %d",
						ptResponseFromServer->nMsgType, ptResponseFromServer->nRC );
			}
				break;
		}
	}

end_of_function:

	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_close() fail=%d", __func__, ipc_errno );
		return IPC_FAIL;
	}

	return 0;
}

void SignalHandler( int nSigno )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d", nSigno );

	exit( EXIT_FAILURE );
}

void ClearStdin( char *pszTemp )
{
	if ( NULL == pszTemp ) { return; }

	if ( pszTemp[ strlen(pszTemp) - 1 ] == '\n' )
	{
		pszTemp[ strlen(pszTemp) - 1 ] = '\0';
	}

	__fpurge( stdin );

	return;
}

int Insert( struct REQUEST_s *ptRequestToServer )
{
	char *pszRet = NULL;

	ptRequestToServer->nMsgType = MTYPE_INSERT;
	
	printf( "[%s] name = ", __func__ );

	pszRet = fgets( ptRequestToServer->szName, sizeof(ptRequestToServer->szName), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );	
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szName );

	printf( "[%s] position = ", __func__ );
	pszRet = fgets( ptRequestToServer->szPosition, sizeof(ptRequestToServer->szPosition), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szPosition );

	printf( "[%s] team = ", __func__ );
	pszRet = fgets( ptRequestToServer->szTeam, sizeof(ptRequestToServer->szTeam), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szTeam );

	printf( "[%s] phone = ", __func__ );
	pszRet = fgets( ptRequestToServer->szPhone, sizeof(ptRequestToServer->szPhone), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szPhone );

	MPGLOG_DBG( "[SEND] mtype = %d | name = %s | position = %s | team = %s | phone = %s",
				ptRequestToServer->nMsgType,
				ptRequestToServer->szName, ptRequestToServer->szPosition,
				ptRequestToServer->szTeam, ptRequestToServer->szPhone );

	return RC_SUCCESS;
}

int Select( struct REQUEST_s *ptRequestToServer )
{
	char *pszRet = NULL;

	char szId[32];
	memset( szId, 0x00, sizeof(szId) );

	printf( "[%s] Enter to select all, or select id = ", __func__ );	
	
	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( szId );

	if ( 0 == strlen(szId) )
	{
		ptRequestToServer->nMsgType = MTYPE_SELECTALL;
		ptRequestToServer->nId = 0;

		MPGLOG_DBG( "[SEND] mtype = %d | id = ALL", ptRequestToServer->nMsgType );
	}
	else if ( 0 < strlen(szId) )
	{
		ptRequestToServer->nMsgType = MTYPE_SELECTONE;
		ptRequestToServer->nId = atoi(szId);

		MPGLOG_DBG( "[SEND] mtype = %d | id = %d", ptRequestToServer->nMsgType, ptRequestToServer->nId );
	}

	return RC_SUCCESS;
}

int Update( struct REQUEST_s *ptRequestToServer )
{
	char *pszRet = NULL;
	
	char szId[32];
	memset( szId, 0x00, sizeof(szId) );

	printf( "[%s] id = ", __func__ );

	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( szId );

	ptRequestToServer->nMsgType = MTYPE_UPDATE;
	ptRequestToServer->nId = atoi(szId);

	printf( "[%s] name = ", __func__ );
	pszRet = fgets( ptRequestToServer->szName, sizeof(ptRequestToServer->szName), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szName );

	printf( "[%s] position = ", __func__ );
	pszRet = fgets( ptRequestToServer->szPosition, sizeof(ptRequestToServer->szPosition), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szPosition );

	printf( "[%s] team = ", __func__ );
	pszRet = fgets( ptRequestToServer->szTeam, sizeof(ptRequestToServer->szTeam), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szTeam );

	printf( "[%s] phone = ", __func__ );
	pszRet = fgets( ptRequestToServer->szPhone, sizeof(ptRequestToServer->szPhone), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( ptRequestToServer->szPhone );

	MPGLOG_DBG( "[SEND] mtype = %d | id = %d | name = %s | position = %s | team = %s | phone = %s",
				ptRequestToServer->nMsgType, ptRequestToServer->nId,
				ptRequestToServer->szName, ptRequestToServer->szPosition,
				ptRequestToServer->szTeam, ptRequestToServer->szPhone );

	return RC_SUCCESS;
}

int Delete( struct REQUEST_s *ptRequestToServer )
{
	char *pszRet = NULL;

	char szId[32];
	memset( szId, 0x00, sizeof(szId) );
	
	printf( "[%s] id = ", __func__ );

	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( NULL == pszRet )
	{
		MPGLOG_ERR( "%s:: fgets() fail", __func__ );
		return INPUT_FAIL;
	}
	ClearStdin( szId );

	ptRequestToServer->nMsgType = MTYPE_DELETE;
	ptRequestToServer->nId = atoi(szId);

	MPGLOG_DBG( "[SEND] mtype = %d | id = %d", ptRequestToServer->nMsgType, ptRequestToServer->nId );

	return RC_SUCCESS;
}
