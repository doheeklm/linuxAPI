/* FW_3-1/FW_Server.c */
#include "FW_Inc.h"

void SignalHandler	( int nSig );
void LogErr			( const char* pszFuncName, int nErrno );
static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList );
int SetUniqueId		( int *pnId );

int Insert			( hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int SelectAll		( hash_table_t *ptHash, struct RESPONSE_s *ptResponse );
int SelectOne		( hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int Update			( hash_table_t *ptHash, struct REQUEST_s *ptRequest );
int Delete			( hash_table_t *ptHash, struct REQUEST_s *ptRequest );

int HASH_LoadFile	( hash_table_t *ptHash );
int HASH_SaveInFile	( hash_table_t *ptHash );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRet = 0;
	int nExit = 0;

	iipc_msg_t tRecvMsg;
	iipc_msg_t tSendMsg;
	iipc_ds_t tIpc;
	iipc_key_t tKey;
	
	REQUEST_t *ptRequest = NULL;
	RESPONSE_t *ptResponse = NULL;

	hash_table_t tHash;
	memset( &tHash, 0x00, sizeof(tHash) );

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
	 *	Hash
	 */
	tHash = HASH_INIT( MEM_DEFAULT,		//m_type
					   FLAG_OW,			//flag
					   STRCMP,			//cmp
					   NULL,			//dhook
					   SIZE_BUCKET,		//bsize
					   DEFAULT_HASHF ); //hashf
	if ( NULL == tHash )
	{
		MPGLOG_ERR( "%s:: HASH_INIT fail", __func__ );
		return HASH_FAIL;
	}	

	nRet = HASH_LoadFile( &tHash );
	if ( SUCCESS != nRet && FILE_EMPTY != nRet )
	{
		MPGLOG_ERR( "%s:: HASH_LoadFile fail", __func__ );
		return HASH_FAIL;
	}

	/*
	 *	Run Program (Server)
	 */
	while ( FLAG_RUN == g_nFlag )
	{
		memset( &tRecvMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &tSendMsg, 0x00, sizeof(iipc_msg_t) );

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

				nRet = Insert( &tHash, ptRequest, ptResponse );
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

				nRet = SelectAll( &tHash, ptResponse );
				if ( SUCCESS != nRet && HASH_EMPTY != nRet )
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

				nRet = SelectOne( &tHash, ptRequest, ptResponse );
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

				nRet = Update( &tHash, ptRequest );
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

				nRet = Delete( &tHash, ptRequest );
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
	
		if ( 2 == ptResponse->nMsgType )
		{
			MPGLOG_SVC( "[SEND] MsgType: %d | Result: %d", ptResponse->nMsgType, ptResponse->nResult );
		}
		else
		{
			MPGLOG_SVC( "[SEND] MsgType: %d | Id: %d | Result: %d", ptResponse->nMsgType, ptResponse->nId, ptResponse->nResult );
		}

		nRet = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			LogErr( __func__, ipc_errno );
			continue;
		}

		nExit = 0;
		printf( "서버 종료(1): " );
		nRet = scanf( "%d", &nExit );
		if ( 1 == nExit )
		{
			break;
		}

		HASH_DUMP( tHash, NULL );
	}

	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		LogErr(  __func__, ipc_errno );
		return TAP_FAIL;
	}

	nRet = HASH_SaveInFile( &tHash );
	if ( SUCCESS != nRet && HASH_EMPTY != nRet )
	{
		MPGLOG_ERR( "%s:: HASH_SaveInFile() fail", __func__ );
		return FUNC_FAIL;
	}

	HASH_RELEASE( tHash );
	
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

int SetUniqueId( int *pnId )
{
	srand( time(NULL) );

	int i = 0;
	int nCnt = 0;
	int nDuplicateFlag = 0;
	mpconf_list_t *ptSectList = NULL;

	do
	{	
		nCnt++;

		if ( nCnt > MAX_CNT_ID )
		{
			MPGLOG_ERR( "%s:: no more unique id", __func__ );
			return OVER_MAX_FAIL;
		}

		*pnId = rand() % MAX_CNT_ID + 1;

		ptSectList = mpconf_get_sect_list( NULL, g_pszFile );
		if ( NULL == ptSectList )
		{
			MPGLOG_ERR( "%s:: mpconf_get_sect_list() fail, path=%s", __func__, g_pszFile );
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

int Insert( hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}
	
	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter(ptRequest) NULL", __func__ );
		return NULL_FAIL;		
	}

	if ( NULL == ptResponse )
	{
		MPGLOG_ERR( "%s:: parameter(ptResponse) NULL", __func__ );
		return NULL_FAIL;
	}

	int nRet = 0;

	char szId[SIZE_ID + 1];
	memset( szId, 0x00, sizeof(szId) );
	
	INFO_t tInfo;
	memset( &tInfo, 0x00, sizeof(tInfo) );

	nRet = SetUniqueId( &(ptRequest->nId) );
	if ( SUCCESS != nRet )
	{
		return FUNC_FAIL;
	}

	ptResponse->nId = ptRequest->nId;
	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	strlcpy( tInfo.szName, ptRequest->szName, sizeof(tInfo.szName) ); 

	nRet = HASH_COPY( *ptHash, szId, strlen(szId), &tInfo, sizeof(tInfo) );
	if ( 0 == nRet )
	{
		MPGLOG_ERR( "%s:: HASH_COPY() fail", __func__ );
		return HASH_FAIL;
	}

	return SUCCESS;
}

int SelectAll( hash_table_t *ptHash, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptResponse )
	{
		MPGLOG_ERR( "%s:: parameter(ptResponse) NULL", __func__ );
		return NULL_FAIL;
	}
	
	int i = 0;
	long lRet = 0;
	long lEntry = 0;
	char *paszIDs[] = { NULL, };
	char *paszValues[] = { NULL, };

	paszIDs[0] = (char*)malloc(10 * (SIZE_ID+1) );
	paszValues[0] = (char*)malloc(10 * 1024);

	ID_NAME_t tIdName;
	memset( &tIdName, 0x00, sizeof(tIdName) );

	//INFO_t *ptInfo = NULL;

	lEntry = HASH_COUNT( *ptHash );

	lRet = HASH_LIST( *ptHash, paszIDs, paszValues, lEntry );
	if ( 0 == lRet )
	{
		return HASH_EMPTY;
	}

	for ( i = 0; i < lEntry; i++ )
	{
		MPGLOG_DBG( "paszIDs[%d]: %s", i, paszIDs[i] );
/*		
		memset( &tIdName, 0x00, sizeof(tIdName) );
	
		ptInfo = (INFO_t *)paszValues[i];
	
		//DEBUG	
		MPGLOG_DBG( "%s:: paszIds[%d]=%s", __func__, i, paszIDs[i] );	
		MPGLOG_DBG( "%s:: %s | %s | %s | %s", __func__, ptInfo->szName, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone );

		tIdName.nId = atoi(paszIDs[i]);
		strlcpy( tIdName.szName, ptInfo->szName, sizeof(tIdName.szName) ); 
		
		memcpy( ptResponse->szBuffer + (i * sizeof(tIdName)), &tIdName, sizeof(tIdName) );*/
	}

	ptResponse->nCntSelectAll = lEntry;

	return SUCCESS;
}

int SelectOne( hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash)  NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter(ptRequest) NULL", __func__ );
		return NULL_FAIL;		
	}

	if ( NULL == ptResponse )
	{
		MPGLOG_ERR( "%s:: parameter(ptResponse) NULL", __func__ );
		return NULL_FAIL;
	}

	char *pszRet = NULL;
	char szId[SIZE_ID + 1];
	
	memset( szId, 0x00, sizeof(szId) );

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	pszRet = HASH_SEARCH( *ptHash, szId );
	if ( 0 == pszRet )
	{
		MPGLOG_ERR( "%s:: HASH_SEARCH fail", __func__ );
		return HASH_FAIL;
	}

	memcpy( ptResponse->szBuffer, pszRet, sizeof(struct INFO_s) ); 

	//DEBUG
	INFO_t *ptInfo = NULL;
	ptInfo = (INFO_t *)pszRet;
	MPGLOG_DBG("%s:: %s|%s|%s|%s|%s", __func__, szId,
				ptInfo->szName, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone );
	
	return SUCCESS;
}

int Update( hash_table_t *ptHash, struct REQUEST_s *ptRequest )
{
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}	

	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter(ptRequest) NULL", __func__ );
		return NULL_FAIL;		
	}

	int nRet = 0;

	char *pszRet = NULL;
	char szId[SIZE_ID + 1];

	INFO_t * ptOldInfo = NULL;
	INFO_t tNewInfo;
	
	memset( szId, 0x00, sizeof(szId) );
	memset( &tNewInfo, 0x00, sizeof(tNewInfo) );

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	pszRet = HASH_SEARCH( *ptHash, szId );

	ptOldInfo = (INFO_t *)pszRet;
	
	if ( 0 == strlen(ptRequest->szName) )
	{
		strlcpy( tNewInfo.szName, ptOldInfo->szName, sizeof(tNewInfo.szName) );
	}

	if ( 0 == strlen(ptRequest->szJobTitle) )
	{
		strlcpy( tNewInfo.szJobTitle, ptOldInfo->szJobTitle, sizeof(tNewInfo.szJobTitle) );
	}

	if ( 0 == strlen(ptRequest->szTeam) )
	{
		strlcpy( tNewInfo.szTeam, ptOldInfo->szTeam, sizeof(tNewInfo.szTeam) );
	}

	if ( 0 == strlen(ptRequest->szPhone) )
	{
		strlcpy( tNewInfo.szPhone, ptOldInfo->szPhone, sizeof(tNewInfo.szPhone) );
	}
	
	nRet = HASH_COPY( *ptHash, szId, strlen(szId), &tNewInfo, sizeof(tNewInfo) );
	if ( 0 == nRet )
	{
		MPGLOG_ERR( "%s:: HASH_COPY() fail", __func__ );
		return HASH_FAIL;
	}

	return SUCCESS;
}

int Delete( hash_table_t *ptHash, struct REQUEST_s *ptRequest )
{
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}		

	if ( NULL == ptRequest )
	{
		MPGLOG_ERR( "%s:: parameter(ptRequest) NULL\n", __func__ );
		return NULL_FAIL;		
	}

	char *pszRet = NULL;
	char szId[6];

	memset( szId, 0x00, sizeof(szId) );

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );

	pszRet = HASH_DELETE( *ptHash, szId );
	if ( 0 == pszRet )
	{
		MPGLOG_ERR( "%s:: HASH_DELETE fail", __func__);
		return HASH_FAIL;
	}

	return SUCCESS;
}

int HASH_LoadFile( hash_table_t *ptHash )
{	
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}

	int i = 0;
	int nRet = 0;
	char *pszDefaultStr = NULL;
	
	INFO_t tInfo;
	memset( &tInfo, 0x00, sizeof(tInfo) );

	mpconf_list_t *ptSectList = NULL;
	
	ptSectList = mpconf_get_sect_list( NULL, g_pszFile );
	if ( NULL == ptSectList )
	{
		MPGLOG_ERR( "%s:: mpconf_get_sect_list fail", __func__ );
		return MPCONF_FAIL;
	}

	if ( 0 == ptSectList->name_num )
	{
		MPGLOG_DBG( "No Item in File" );
		return FILE_EMPTY;
	}

	for ( i = 0; i < ptSectList->name_num; i++ )
	{
		pszDefaultStr = NULL;
		memset( &tInfo, 0x00, sizeof(tInfo) );

		nRet = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], NAME, tInfo.szName, sizeof(tInfo.szName), pszDefaultStr );
		nRet = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], JOBTITLE, tInfo.szJobTitle, sizeof(tInfo.szJobTitle), pszDefaultStr );
		nRet = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], TEAM, tInfo.szTeam, sizeof(tInfo.szTeam), pszDefaultStr );
		nRet = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], PHONE, tInfo.szPhone, sizeof(tInfo.szPhone), pszDefaultStr );
	
		nRet = HASH_COPY( *ptHash, ptSectList->name[i], strlen(ptSectList->name[i]), &tInfo, sizeof(INFO_t) );
		if ( 0 == nRet )
		{
			MPGLOG_ERR( "%s:: HASH_COPY fail", __func__ );
			return HASH_FAIL;
		}

		//DEBUG
		MPGLOG_DBG( "%s:: %s|%s|%s|%s|%s", __func__, ptSectList->name[i], tInfo.szName, tInfo.szJobTitle, tInfo.szTeam, tInfo.szPhone );
	}

	HASH_DUMP( *ptHash, NULL );
	
	ListFree( ptSectList, NULL, NULL );

	return SUCCESS;
}

int HASH_SaveInFile( hash_table_t *ptHash )
{
	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter NULL", __func__ );
		return NULL_FAIL;
	}

	int i = 0;
	int nRet = 0;
	long lRet = 0;
	long lEntry = 0;

	char *paszIDs[] = { NULL, };
	char *paszValues[] = { NULL, };
	INFO_t *ptInfo = NULL;

	lEntry = HASH_COUNT( *ptHash );
	
	MPGLOG_DBG( "%s:: Entry[%ld]", __func__, lEntry );

	nRet = rename( g_pszFile, g_pszBackup );
	if ( 0 != nRet )
	{
		MPGLOG_ERR( "%s:: rename fail=%d", __func__, errno );
		return FUNC_FAIL;	
	}
	
	lRet = HASH_LIST( *ptHash, paszIDs, paszValues, lEntry );
	if ( 0 == lRet )
	{
		printf(" Hash Empty \n ");
		return HASH_EMPTY;
	}

	for ( i = 0; i < lEntry; i++ )
	{
		MPGLOG_DBG( "ID: %s", paszIDs[i] );
		
		ptInfo = (INFO_t *)paszValues[i];
		MPGLOG_DBG( "VALUE: %s | %s | %s | %s",
					ptInfo->szName, ptInfo->szJobTitle,
			   		ptInfo->szTeam, ptInfo->szPhone );

		nRet = mpconf_set_str( NULL, g_pszFile, paszIDs[i], NAME, ptInfo->szName );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}

		nRet = mpconf_set_str( NULL, g_pszFile, paszIDs[i], JOBTITLE, ptInfo->szJobTitle );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}

		nRet = mpconf_set_str( NULL, g_pszFile, paszIDs[i], TEAM, ptInfo->szTeam );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}

		nRet = mpconf_set_str( NULL, g_pszFile, paszIDs[i], PHONE, ptInfo->szPhone );
		if ( 0 > nRet )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}
	}	

	HASH_DUMP( *ptHash, NULL );

	return SUCCESS;
}
