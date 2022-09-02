/* TAP_Server.c */
#include "TAP_Inc.h"

void SignalHandler	( int nSig );
void LogErr			( const char* pszFuncName, int nErrno );
static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList );

int SetUniqueId		( int *pnId );
int CheckIdExist	( int nId );

int Insert			( struct REQUEST_s *ptRequest );
int SelectAll		( struct RESPONSE_s *ptResponse );
int SelectOne		( struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int Update			( struct REQUEST_s *ptRequest );
int Delete			( struct REQUEST_s *ptRequest );

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
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		LogErr( __func__, ipc_errno );
		nRet = TAP_ipc_close( &tIpc );
		if ( 0 > nRet )
		{
			LogErr(  __func__, ipc_errno );
		}
		return TAP_FAIL;
	}
	
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
			case 2:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d", ptRequest->nMsgType );

				nRet = SelectAll( ptResponse );
				
				if ( SUCCESS != nRet )
				{
					ptResponse->nResult = 0;
					break;
				}
				
				ptResponse->nResult = 1;
			}
				break;
			case 3:
			{
				MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d", ptRequest->nMsgType, ptRequest->nId );

				nRet = SelectOne( ptRequest, ptResponse );
			
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
				MPGLOG_SVC( "[RECV] MsgType: %d | Id: %d | JobTitle: %s | Team: %s | Phone: %s",
						ptRequest->nMsgType, ptRequest->nId, ptRequest->szJobTitle, ptRequest->szTeam, ptRequest->szPhone );

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

		if ( ptResponse->nMsgType == 2 )
			MPGLOG_SVC( "[SEND] MsgType: %d | Result: %d | CntSelectAll: %d", ptResponse->nMsgType, ptResponse->nResult, ptResponse->nCntSelectAll );
		else
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

	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LogErr(  __func__, ipc_errno );
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

void LogErr( const char* pszFuncName, int nErrno )
{
	MPGLOG_ERR( "%s errno[%d]\n", pszFuncName, nErrno );
	return;
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

int SetUniqueId( int *pnId )
{
	srand( time(NULL) );

	int i = 0;
	int nDuplicateFlag = 0;
	int nCnt = 0;

	mpconf_list_t *ptSectList = NULL;

	do
	{	
		nCnt++;

		if ( nCnt > MAX_CNT_ID )
		{
			MPGLOG_ERR( "%s:: no more unique id", __func__ );
			return OVER_MAX_FAIL;
		}

		*pnId = rand() % MAX_CNT_ID + 1; // if 10, 1~10. if 1000, 1~1000

		ptSectList = mpconf_get_sect_list( NULL, g_pszConfigPath );
		if ( NULL == ptSectList )
		{
			MPGLOG_ERR( "%s:: mpconf_get_sect_list() fail, path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}

		for ( i = 0; i < ptSectList->name_num; i++ )
		{
			if ( *pnId == atoi( ptSectList->name[i] ) )
			{
				nDuplicateFlag = 1;	
				continue;
			}
		}

		nDuplicateFlag = 0;

	} while ( nDuplicateFlag == 1 );

	MPGLOG_DBG( "%s:: create random id [%d]", __func__, *pnId );

	ListFree( ptSectList, NULL, NULL );

	return SUCCESS;
}

int CheckIdExist( int nId ) 
{
	int i = 0;
	
	mpconf_list_t *ptSectList = NULL;

	ptSectList = mpconf_get_sect_list( NULL, g_pszConfigPath );
	if ( NULL == ptSectList )
	{
		MPGLOG_ERR( "%s:: mpconf_get_sect_list() fail, path=%s", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}

	for ( i = 0; i < ptSectList->name_num; i++ )
	{
		if ( nId == atoi( ptSectList->name[i] ) )
		{
			return ID_EXIST;
		}
	}
	
	ListFree( ptSectList, NULL, NULL );

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
	char szId[SIZE_ID + 1];
	
	memset( szId, 0x00, sizeof(szId) );

	nRet = SetUniqueId( &(ptRequest->nId) );
	if ( SUCCESS != nRet )
	{
		return FUNC_FAIL;
	}

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, NAME, ptRequest->szName );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}

	nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, JOBTITLE, ptRequest->szJobTitle );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}

	nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, TEAM, ptRequest->szTeam );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}
	
	nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, PHONE, ptRequest->szPhone );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}
	
	return SUCCESS;
}

int SelectAll( struct RESPONSE_s *ptResponse )
{
	int i = 0, j = 0;
	int nCnt = 0;

	SELECT_ALL_t tSelectAll;
	memset( &tSelectAll, 0x00, sizeof(SELECT_ALL_t) );

	mpconf_list_t *ptSectList = NULL;
	mpconf_list_t *ptItemList = NULL;
	mpconf_list_t *ptValueList = NULL;

	ptSectList = mpconf_get_sect_list( NULL, g_pszConfigPath );
	if ( NULL == ptSectList )
	{
		MPGLOG_ERR( "%s:: mpconf_get_sect_list() fail, path=%s", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}

	for ( i = 0; i < ptSectList->name_num; i++ )
	{
		ptValueList = mpconf_get_value_list( NULL, g_pszConfigPath, ptSectList->name[i], &ptItemList );
		if ( NULL == ptValueList )
		{
			MPGLOG_ERR( "%s:: mpconf_get_value_list() fail, path=%s", __func__, g_pszConfigPath );
		
			ListFree( ptSectList, ptItemList, ptValueList );
	
			return MPCONF_FAIL;
		}

		for ( j = 0; j < ptItemList->name_num; j++ )
		{
			if ( strcmp( NAME, ptItemList->name[j] ) == 0 )
			{
				/*
				 *	Section 문자열값 => ID
				 *	Item의 Value값=> NAME
				 */
				tSelectAll.nId = atoi( ptSectList->name[i] );
				
				strlcpy( tSelectAll.szName, ptValueList->name[j], sizeof(tSelectAll.szName) );

				MPGLOG_DBG( "[%s] %s=%s", ptSectList->name[i], ptItemList->name[j], ptValueList->name[j] ); 
			
				/* 버퍼사이즈를 초과하여 중간에 메모리가 잘릴 가능성이 있으면 memcpy 하지 않고 break */
				if ( ( (nCnt + 1) * sizeof(tSelectAll) ) > sizeof(ptResponse->szBuffer) )
				{
					MPGLOG_DBG( "현재까지 읽은 사이즈[%ld], 버퍼사이즈[%ld]: 버퍼 사이즈 초과될 가능성이 있어 memcpy 하지 않음", nCnt * sizeof(tSelectAll), sizeof(ptResponse->szBuffer) );
					break;
				}

				memcpy( ptResponse->szBuffer + ( nCnt * sizeof(tSelectAll) ), &tSelectAll, sizeof(tSelectAll) ); 
				
				nCnt++;
			}
		}
	}
	
	ptResponse->nCntSelectAll = nCnt;

	ListFree( ptSectList, ptItemList, ptValueList );

	return SUCCESS;
}

int SelectOne( struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;		
	}

	if ( ID_NOT_EXIST == CheckIdExist( ptRequest->nId ) )
	{
		return CLIENT_INPUT_FAIL;
	}

	int nRet = 0;
	char szBuf[256];
	char szDefaultBuf[256];
	char szId[SIZE_ID + 1];
	
	struct SELECT_ONE_s tSelectOne;

	memset( szId, 0x00, sizeof(szId) );
	memset( szBuf, 0x00, sizeof(szBuf) );
	memset( szDefaultBuf, 0x00, sizeof(szDefaultBuf) );
	memset( &tSelectOne, 0x00, sizeof(struct SELECT_ONE_s) );

	if ( ptRequest->nId > 0 )
	{
		snprintf( szId, sizeof(szId), "%d", ptRequest->nId );
		
		nRet = mpconf_get_str( NULL, g_pszConfigPath, szId, NAME, szBuf, sizeof(szBuf), szDefaultBuf );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str() fail, path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}

		strlcpy( tSelectOne.szName, szBuf, sizeof(tSelectOne.szName) );

		memset( szBuf, 0x00, sizeof(szBuf) );
		nRet = mpconf_get_str( NULL, g_pszConfigPath, szId, JOBTITLE, szBuf, sizeof(szBuf), szDefaultBuf );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str() fail, path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}

		strlcpy( tSelectOne.szJobTitle, szBuf, sizeof(tSelectOne.szJobTitle) );

		memset( szBuf, 0x00, sizeof(szBuf) );
		nRet = mpconf_get_str( NULL, g_pszConfigPath, szId, TEAM, szBuf, sizeof(szBuf), szDefaultBuf );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str() fail, path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}

		strlcpy( tSelectOne.szTeam, szBuf, sizeof(tSelectOne.szTeam) );

		memset( szBuf, 0x00, sizeof(szBuf) );
		nRet = mpconf_get_str( NULL, g_pszConfigPath, szId, PHONE, szBuf, sizeof(szBuf), szDefaultBuf );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str() fail, path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}
	
		strlcpy( tSelectOne.szPhone, szBuf, sizeof(tSelectOne.szPhone) );
	}
	else if ( ptRequest->nId == 0 )
	{
		MPGLOG_SVC( "%s:: NO INPUT ID for SELECT ONE", __func__ );
		return NOT_EXIST;
	}
	
	memcpy( ptResponse->szBuffer, &tSelectOne, sizeof(tSelectOne) );	

	return SUCCESS;
}

int Update( struct REQUEST_s *ptRequest )
{
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter NULL\n", __func__ );
		return NULL_FAIL;		
	}

	if ( ID_NOT_EXIST == CheckIdExist( ptRequest->nId ) )
	{	
		return CLIENT_INPUT_FAIL;
	}

	int nRet = 0;
	char szId[SIZE_ID + 1];
	
	memset( szId, 0x00, sizeof(szId) );
	
	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	if ( strlen(ptRequest->szJobTitle) > 0 )
	{
		nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, JOBTITLE, ptRequest->szJobTitle );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}
	}

	if ( strlen(ptRequest->szTeam) > 0 )
	{
		nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, TEAM, ptRequest->szTeam );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}
	}

	if ( strlen(ptRequest->szPhone) > 0 )
	{
		nRet = mpconf_set_str( NULL, g_pszConfigPath, szId, PHONE, ptRequest->szPhone );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str(), path=%s", __func__, g_pszConfigPath );
			return MPCONF_FAIL;
		}
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

	if ( ID_NOT_EXIST == CheckIdExist( ptRequest->nId ) )
	{	
		return CLIENT_INPUT_FAIL;
	}

	int nRet = 0;
	char szId[6];
	
	memset( szId, 0x00, sizeof(szId) );

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	nRet = mpconf_del_sect( NULL, g_pszConfigPath, szId );
	if ( 0 > nRet )
	{
		MPGLOG_ERR( "%s:: mpconf_del_sect() fail, path=%s\n", __func__, g_pszConfigPath );
		return MPCONF_FAIL;
	}
	
	return SUCCESS;
}
