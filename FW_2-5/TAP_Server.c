/* FW_2-5/TAP_Server.c */
#include "TAP_Inc.h"

void LogErr			( const char* pszFuncName, int nErrno );
void SignalHandler	( int nSig );
static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList );

int GetConfig		( char *pszIP, int *pnPort, int nSizeIP );
int SetUniqueId		( int *pnId );
int CheckIdExist	( int nId );

int Insert			( struct REQUEST_s *ptRequest );
int Update			( struct REQUEST_s *ptRequest );
int Delete			( struct REQUEST_s *ptRequest );
int GetOriginalData	( int nId, char *pszAttribute, char *pszBuf, int nSizeBuf );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	iipc_msg_t tRecvMsg;
	iipc_msg_t tSendMsg;
	iipc_ds_t tIpc;
	iipc_key_t tKey;
	
	REQUEST_t *ptRequest = NULL;
	RESPONSE_t *ptResponse = NULL;

	int nRet = 0;
	int nPort = 0;
	char szIP[SIZE_IP];
	memset( szIP, 0x00, sizeof(szIP) );

	/*
	 *	MPLOG
	 */
	nRet = MPGLOG_INIT( SERVER_PROCESS, NULL,
			LOG_MODE_DAILY | LOG_MODE_NO_DATE |	LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_DBG );
	if ( 0 > nRet )
	{
		printf( "%s MPGLOG_INIT() ERROR", __func__ );
		return MPGLOG_FAIL;
	}

	/*
	 *	TAP_IPC 
	 */
	nRet = TAP_ipc_open( &tIpc, SERVER_PROCESS );
	if ( 0 > nRet )
	{
		LogErr( __func__, ipc_errno );
		return TAP_IPC_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		LogErr( __func__, ipc_errno );
		goto end_of_function;
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
	 *	Run Program (Server)
	 */	
	while ( FLAG_RUN == g_nFlag )
	{
		memset( &tRecvMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &tSendMsg, 0x00, sizeof(iipc_msg_t) );

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
			case 4:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d | Name: %s | JobTitle: %s | Team: %s | Phone: %s",
							ptRequest->nMsgType, ptRequest->nId, ptRequest->szName,
						ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone );

				nRet = Update( ptRequest );
				if ( SUCCESS != nRet )
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
				if ( SUCCESS != nRet )
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

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LogErr(  __func__, ipc_errno );
		return TAP_IPC_FAIL;
	}
	
	return 0;
}

void LogErr( const char* pszFuncName, int nErrno )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nErrno );
	return;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d\n", nSig );

	exit( -1 );
}

static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList )
{
	if ( NULL != ptSectList )
	{
		mpconf_list_free( ptSectList );
	}

	if ( NULL != ptItemList )
	{
		mpconf_list_free( ptItemList );
	}

	if ( NULL != ptValueList )
	{
		mpconf_list_free( ptValueList );
	}
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

int SetUniqueId( int *pnId )
{
	int nRet = 0;
	int nCnt = 0;
	int nDuplicateFlag = 0;

	srand( time(NULL) );

	do
	{
		nCnt++;
	
		if ( nCnt > MAX_CNT_ID )
		{
			MPGLOG_ERR( "%s:: 아이디를 더이상 생성할 수 없습니다.", __func__ );
			return OVER_MAX_FAIL;
		}

		*pnId = rand() % MAX_CNT_ID + 1; //ID 1~1000

		nRet = CheckIdExist( *pnId );
		if ( ID_EXIST == nRet )
		{
			nDuplicateFlag = 1;
			continue;
		}
			
		nDuplicateFlag = 0;

	} while ( nDuplicateFlag == 1 );

	MPGLOG_DBG( "%s:: create random id [%d]", __func__, *pnId );

	return SUCCESS;
}

int CheckIdExist( int nId ) 
{
	int nRet = 0;

	char szKey[TAP_REGI_KEY_SIZE];
	char szBuf[1024];
	
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szBuf, 0x00, sizeof(szBuf) );
	
	nRet = TAP_Registry_udp_enum_key_node( KEY_DIR, strlen(KEY_DIR), szBuf, sizeof(szBuf), SYSTEM_ID );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_enum_key_node() fail=%d", __func__, nRet );
		return TAP_REGI_FAIL;
	}

	pszToken = strtok_r( szBuf, DELIM, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		MPGLOG_DBG( "%s", pszToken );

		if ( nId == atoi(pszToken) )
		{
			MPGLOG_DBG( "%s:: ID_EXIST return", __func__ );
			return ID_EXIST;
		}

		pszToken = strtok_r( NULL, DELIM, &pszDefaultToken );
	}

	MPGLOG_DBG( "%s:: ID_NOT_EXIST return", __func__ );
	return ID_NOT_EXIST;
}

int Insert( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	char szKey[TAP_REGI_KEY_SIZE];
	char szValue[SIZE_VALUE];

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szValue, 0x00, sizeof(szValue) );

	nRet = SetUniqueId( &(ptRequest->nId) );
	if ( SUCCESS != nRet )
	{
		return FUNC_FAIL;
	}

	snprintf( szKey, sizeof(szKey), "%s%d", KEY_DIR, ptRequest->nId );	
	
	/*
	 *	Create Registry Key
	 */
	nRet = TAP_Registry_udp_key_create( szKey, strlen(szKey), TAP_REGISTRY_FILE, SYSTEM_ID );
	if ( TAP_REGI_ALREADY_EXIST == nRet )
	{
		MPGLOG_DBG("%s:: Create Key", __func__ );	
	}
	else if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_key_create() fail=%d", __func__, nRet );
		return TAP_REGI_FAIL;
	}

	memcpy( szValue, ptRequest, sizeof(struct REQUEST_s) );

	/*
	 *	Set Value
	 */
	nRet = TAP_Registry_udp_set_value( szKey, strlen(szKey), szValue, sizeof(struct REQUEST_s), SYSTEM_ID );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_set_value() fail=%d, key=%s, value=%s", __func__, nRet, szKey, szValue );
		return TAP_REGI_FAIL;
	}

	return SUCCESS;
}

int Update( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	char szKey[TAP_REGI_KEY_SIZE];
	char szReValue[SIZE_VALUE];
	char szCopy[256];

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szReValue, 0x00, sizeof(szReValue) );
	memset( szCopy, 0x00, sizeof(szCopy) );
	
	if ( 0 == strlen(ptRequest->szName) )
	{
		nRet = GetOriginalData( ptRequest->nId, NAME, szCopy, sizeof(szCopy) );
		if ( SUCCESS != nRet )
		{
			return FUNC_FAIL;
		}
		strlcpy( ptRequest->szName, szCopy, sizeof(ptRequest->szName) );
	}

	memset( szCopy, 0x00, sizeof(szCopy) );
	if ( 0 == strlen(ptRequest->szJobTitle) )
	{
		nRet = GetOriginalData( ptRequest->nId, JOBTITLE, szCopy, sizeof(szCopy) );
		if ( SUCCESS != nRet )
		{
			return FUNC_FAIL;
		}
		strlcpy( ptRequest->szJobTitle, szCopy, sizeof(ptRequest->szJobTitle) );
	}

	memset( szcopy, 0x00, sizeof(szcopy) );
	if ( 0 == strlen(ptRequest->szTeam) )
	{
		nRet = GetOriginalData( ptRequest->nId, TEAM, szCopy, sizeof(szCopy) );
		if ( SUCCESS != nRet )
		{
			return FUNC_FAIL;
		}
		strlcpy( ptRequest->szTeam, szCopy, sizeof(ptRequest->szTeam) );
	}

	memset( szcopy, 0x00, sizeof(szcopy) );
	if ( 0 == strlen(ptRequest->szPhone) )
	{
		nRet = GetOriginalData( ptRequest->nId, PHONE, szCopy, sizeof(szCopy) );
		if ( SUCCESS != nRet )
		{
			return FUNC_FAIL;
		}
		strlcpy( ptRequest->szPhone, szCopy, sizeof(ptRequest->szPhone) );
	}

	snprintf( szKey, sizeof(szKey), "%s%d", KEY_DIR, ptRequest->nId );

	memcpy( szReValue, ptRequest, sizeof(struct REQUEST_s) );

	nRet = TAP_Registry_udp_replace_value( szKey, strlen(szKey), szReValue, sizeof(struct REQUEST_s), SYSTEM_ID );
	if ( 0 > nRet )
	{
		if ( TAP_REGI_NOT_FOUND == nRet )
		{
			MPGLOG_DBG( "%s:: TAP_REGI_NOT_FOUND", __func__ );
			return ID_NOT_EXIST;
		}
		
		MPGLOG_ERR( "%s:: TAP_Registry_udp_replace_value() fail=%d", __func__, nRet );
		return TAP_REGI_FAIL;
	}

	return SUCCESS;
}

int Delete( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;
	char szKey[TAP_REGI_KEY_SIZE];
	memset( szKey, 0x00, sizeof(szKey) );

	snprintf( szKey, sizeof(szKey), "%s%d", KEY_DIR, ptRequest->nId );	

	nRet = TAP_Registry_udp_key_delete( szKey, strlen(szKey), TAP_REGISTRY_UDP_REMOVE, SYSTEM_ID );
	if ( 0 > nRet )
	{
		if ( TAP_REGI_NOT_FOUND == nRet )
		{
			MPGLOG_DBG( "%s:: TAP_REGI_NOT_FOUND", __func__ );
			return ID_NOT_EXIST;
		}
		MPGLOG_ERR( "%s:: TAP_Registry_udp_key_delete() fail=%d", __func__, nRet );
		return TAP_REGI_FAIL;
	}

	return SUCCESS;
}

int GetOriginalData	( int nId, char *pszAttribute, char *pszBuf, int nSizeBuf )
{
	int nRet = 0;
	char szKey[TAP_REGI_KEY_SIZE];
	char szValue[SIZE_VALUE];

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szValue, 0x00, sizeof(szValue) );

	snprintf( szKey, sizeof(szKey), "%s%d", KEY_DIR, nId );

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

	if ( 0 == strcmp( pszAttribute, NAME ) )
	{
		strlcpy( pszBuf, ptTemp->szName, nSizeBuf );
	}
	else if ( 0 == strcmp( pszAttribute, JOBTITLE ) )
	{
		strlcpy( pszBuf, ptTemp->szJobTitle, nSizeBuf );
	}
	else if ( 0 == strcmp( pszAttribute, TEAM ) )
	{
		strlcpy( pszBuf, ptTemp->szTeam, nSizeBuf );
	}
	else if ( 0 == strcmp( pszAttribute, PHONE ) )
	{
		strlcpy( pszBuf, ptTemp->szPhone, nSizeBuf );
	}
	else
	{
		return FUNC_FAIL;
	}

	return SUCCESS;
}
