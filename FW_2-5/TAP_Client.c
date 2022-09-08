/* FW_2-5/TAP_Client.c */
#include "TAP_Inc.h"

void ClearStdin		( char *pszTemp );

void LogErr			( const char* pszFuncName, int nErrno );
void SignalHandler	( int nSig );
static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList );

int GetConfig		( char *pszIP, int *pnPort, int nSizeIP );
int CheckIdExist	( int nId );

int	Insert			( struct REQUEST_s *ptRequest );
int Select			( struct REQUEST_s *ptRequest );
int Update			( struct REQUEST_s *ptRequest );
int Delete			( struct REQUEST_s *ptRequest );
int GetAllIdAndName ();

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

	int nRet = 0;
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;
	
	iipc_ds_t	tIpc;
	iipc_key_t	tSrcKey;
	iipc_key_t	tDstKey;
	iipc_msg_t	tSendMsg;
	iipc_msg_t	tRecvMsg;

	int nPort = 0;
	char szIP[SIZE_IP];
	memset( szIP, 0x00, sizeof(szIP) );

	/*
	 *	MPLOG
	 */
	nRet = MPGLOG_INIT( CLIENT_PROCESS, NULL,
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
	nRet = TAP_ipc_open( &tIpc, CLIENT_PROCESS );
	if ( 0 > nRet )
	{
		LogErr( __func__, ipc_errno );
		return TAP_IPC_FAIL;
	}	

	tDstKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tDstKey )
	{
		LogErr( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
		}
		return TAP_IPC_FAIL;
	}

	tSrcKey = TAP_ipc_getkey( &tIpc, CLIENT_PROCESS );
	if ( IPC_NOPROC == tSrcKey )
	{
		LogErr( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
		}
		return TAP_IPC_FAIL;
	}

	/*
	 *	MPCONF
	 */
	if ( SUCCESS != GetConfig( szIP, &nPort, sizeof(szIP) ) )
	{
		MPGLOG_ERR( "%s:: GetConfig() fail", __func__ );
		goto end_of_function;	
	}

	/*
	 *	TAP_Registry
	 */
	nRet = TAP_Registry_udp_open( szIP, nPort, '0', SYSTEM_ID );
	if ( -1 == nRet )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_open() fail=%d", __func__, nRet );
		goto end_of_function;
	}

	nRet = TAP_Registry_udp_manager_check_alive( SYSTEM_ID );
	if ( -1 == nRet )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_manager_check_alive() fail=%d", __func__, nRet );
		goto end_of_function;
	}
	MPGLOG_DBG( "Error Message [%s]", TAP_REG_GET_ERROR_CONTENT(nRet) );

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
				LogErr( __func__, errno );
				return FGETS_FAIL;
			}
			ClearStdin( szPickMenu );

			nPickMenu = atoi( szPickMenu );
		} while ( nPickMenu < 1 || nPickMenu > 5);

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

				MPGLOG_SVC( "[SEND] MsgType: %d | Name: %s | JobTitle: %s | Team: %s | Phone: %s",
						ptRequest->nMsgType, ptRequest->szName,
						ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone );
			}
				break;
			case 2:
			{
				nRet = Select( ptRequest );
				if ( ID_NOT_EXIST == nRet || SUCCESS == nRet )
				{
					continue;
				}
				else 
				{	
					goto end_of_function;
				}
			}
			case 3:
			{
				nRet = Update( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{
					goto end_of_function;
				}

				MPGLOG_SVC( "[SEND] MsgType: %d | Id: %d | Name: %s | JobTitle: %s | Team: %s | Phone: %s",
						ptRequest->nMsgType, ptRequest->nId, ptRequest->szName,
						ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone ); 
			}
				break;
			case 4:
			{
				nRet = Delete( ptRequest );
				if ( SUCCESS != nRet && INPUT_FAIL != nRet )
				{
					goto end_of_function;
				}

				MPGLOG_SVC( "[SEND] MsgType: %d | Id: %d", ptRequest->nMsgType, ptRequest->nId );
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
			return TAP_IPC_FAIL;
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
			return TAP_IPC_FAIL;
		}

		ptResponse = (RESPONSE_t *)tRecvMsg.buf.msgq_buf;	

		MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d | Result: %d", ptResponse->nMsgType, ptResponse->nId, ptResponse->nResult );
	}

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LogErr( __func__, ipc_errno );
		return TAP_IPC_FAIL;
	}

	return 0;
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

static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList )
{
	if ( NULL != ptSectList )
		mpconf_list_free( ptSectList );

	if ( NULL != ptItemList )
		mpconf_list_free( ptItemList );

	if ( NULL != ptValueList )
		mpconf_list_free( ptValueList );
}

int GetConfig( char *pszIP, int *pnPort, int nSizeIP )
{
	int i = 0;

	mpconf_list_t *ptItemList = NULL;
	mpconf_list_t *ptValueList = NULL;

	ptValueList = mpconf_get_value_list( NULL, CONFIG_PATH, SECTION_NAME, &ptItemList );
	if ( NULL == ptValueList )
	{
		MPGLOG_ERR( "%s:: mpconf_get_value_list() fail", __func__ );
		return MPCONF_FAIL;
	}

	for ( i = 0; i < ptItemList->name_num; i++ )
	{
		if ( 0 == strcmp( ptItemList->name[i], ITEM_NAME_IP ) )
		{
			strlcpy( pszIP, ptValueList->name[i], nSizeIP ); 
		}
		else if ( 0 == strcmp( ptItemList->name[i], ITEM_NAME_PORT ) )
		{
			*pnPort = atoi( ptValueList->name[i] );
		}
	}

	MPGLOG_DBG( "%s::[%s]IP=%s|PORT=%d", __func__, SECTION_NAME, pszIP, *pnPort );
	
	ListFree( NULL, ptItemList, ptValueList );

	return SUCCESS;
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
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szName );

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptRequest->szTeam, sizeof(ptRequest->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptRequest->szPhone, sizeof(ptRequest->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szPhone );

	MPGLOG_DBG( "%d|%d|%s|%s|%s|%s", ptRequest->nMsgType, ptRequest->nId,
		ptRequest->szName, ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone );	

	return SUCCESS;
}

int Select( struct REQUEST_s *ptRequest )
{	
	if ( NULL != ptRequest )
	{
		memset( ptRequest, 0x00, sizeof(struct REQUEST_s) );
	}

	int nRet = 0;
	int nPickSelect = 0;
	char szPickSelect[8];
	char *pszRet = NULL;

	char szInputId[32];
	char szInputName[32];
	char szKey[TAP_REGI_KEY_SIZE]; //64
	char szValue[SIZE_VALUE];
	
	memset( szInputId, 0x00, sizeof(szInputId) );
	memset( szInputName, 0x00, sizeof(szInputName) );
	memset( szKey, 0x00, sizeof(szKey) );
	memset( szValue, 0x00, sizeof(szValue) );
	
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
			LogErr( __func__, errno );
			return FGETS_FAIL;
		}
		ClearStdin( szPickSelect );

		nPickSelect = atoi( szPickSelect );

	} while ( nPickSelect != 1 && nPickSelect != 2 );

	switch ( nPickSelect )
	{
		case 1:
		{
			GetAllIdAndName();	
		}
			break;
		case 2:
		{
			printf( "[%s] Input ID: ", __func__ );

			pszRet = fgets( szInputId, sizeof(szInputId), stdin );
			if ( NULL == pszRet )
			{
				LogErr( __func__, errno );
				return FGETS_FAIL;
			}
			ClearStdin( szInputId );

			ptRequest->nId = atoi(szInputId);
			MPGLOG_DBG( "%s:: ID[%d]", __func__, ptRequest->nId );

			snprintf( szKey, sizeof(szKey), "%s%d", KEY_DIR, ptRequest->nId );	
		
			/*
			 * Get Value
			 */
			nRet = TAP_Registry_udp_query_value( szKey, strlen(szKey), szValue, sizeof(struct REQUEST_s), SYSTEM_ID );
			if ( 0 > nRet )
			{
				if ( TAP_REGI_NOT_FOUND == nRet )
				{
					MPGLOG_DBG( "%s:: TAP_REGI_NOT_FOUND", __func__ );	
					return ID_NOT_EXIST;
				}
				MPGLOG_ERR( "%s:: TAP_Registry_udp_set_value() fail=%d, key=%s", __func__, nRet, szKey );
				return TAP_REGI_FAIL;
			}
			
			struct REQUEST_s *ptTemp = NULL;
			ptTemp = (struct REQUEST_s *)szValue;

			MPGLOG_DBG( "%s:: Name: %s | JobTitle: %s | Team: %s | Phone: %s",
				__func__, ptTemp->szName, ptTemp->szJobTitle, ptTemp->szTeam, ptTemp->szPhone );
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
		LogErr( __func__, errno );
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
	
	printf( "[%s] Name: ", __func__ );
	pszRet = fgets( ptRequest->szName, sizeof(ptRequest->szName), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szName );

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptRequest->szJobTitle, sizeof(ptRequest->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptRequest->szTeam, sizeof(ptRequest->szTeam), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
		return FGETS_FAIL;
	}
	ClearStdin( ptRequest->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptRequest->szPhone, sizeof(ptRequest->szPhone), stdin );
	if ( NULL == pszRet )
	{
		LogErr( __func__, errno );
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
		LogErr( __func__, errno );
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

int GetAllIdAndName()
{
	int i = 0;
	int nRet = 0;
	int nIndex = 0;

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	char szKey[TAP_REGI_KEY_SIZE]; //64
	char szValue[SIZE_VALUE];
	int anListId[10];
	
	memset( szValue, 0x00, sizeof(szValue) );
	memset( anListId, 0x00, sizeof(anListId) );

	nRet = TAP_Registry_udp_enum_key_node( KEY_DIR, strlen(KEY_DIR), szValue, sizeof(szValue), SYSTEM_ID );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_enum_key_node() fail=%d", __func__, nRet );
		return TAP_REGI_FAIL;
	}

	pszToken = strtok_r( szValue, DELIM, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		if ( nIndex >= 10 )
		{
			return OVER_MAX_FAIL;
		}

		anListId[nIndex] = atoi(pszToken);		
		nIndex++;
		
		pszToken = strtok_r( NULL, DELIM, &pszDefaultToken );
	}

	for ( i = 0; i < nIndex; i++ )
	{
		memset( szKey, 0x00, sizeof(szKey) );
		snprintf( szKey, sizeof(szKey), "%s%d", KEY_DIR, anListId[i] );
		
		nRet = TAP_Registry_udp_query_value( szKey, strlen(szKey), szValue, sizeof(struct REQUEST_s), SYSTEM_ID );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: TAP_Registry_udp_set_value() fail=%d, key=%s", __func__, nRet, szKey );
			return TAP_REGI_FAIL;
		}	

		struct REQUEST_s *ptTemp = NULL;
		ptTemp = (struct REQUEST_s *) szValue;

		MPGLOG_DBG( "Id: %d | Name: %s", anListId[i], ptTemp->szName );
	}

	return SUCCESS;
}
