/* FW_3-1/FW_Server.c */
#include "FW_Inc.h"

void SignalHandler	( int nSig );
void LogErr			( const char* pszFuncName, int nErrno );
static void ListFree( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList );
int SetUniqueId		( int *pnId );

int UTIL_LoadFile	( linked_list_t *ptList, hash_table_t *ptHash );
int UTIL_SaveInFile	( linked_list_t *ptList, hash_table_t *ptHash );

int Insert			( linked_list_t *ptList, hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int SelectAll		( linked_list_t *ptList, hash_table_t *ptHash, struct RESPONSE_s *ptResponse );
int SelectOne		( hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );
int Update			( hash_table_t *ptHash, struct REQUEST_s *ptRequest );
int Delete			( linked_list_t *ptList, hash_table_t *ptHash, struct REQUEST_s *ptRequest );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRC = 0;
	int nExit = 0;

	iipc_msg_t tRecvMsg;
	iipc_msg_t tSendMsg;
	iipc_ds_t tIpc;
	iipc_key_t tKey;
	
	REQUEST_t *ptRequest = NULL;
	RESPONSE_t *ptResponse = NULL;

	linked_list_t tList;
	memset( &tList, 0x00, sizeof(tList) );

	hash_table_t tHash;
	memset( &tHash, 0x00, sizeof(tHash) );

	/*
	 *	MPLOG
	 */
	nRC = MPGLOG_INIT( SERVER_PROCESS, NULL,
			LOG_MODE_DAILY | LOG_MODE_NO_DATE |	LOG_MODE_LEVEL_TAG,
			LOG_LEVEL_DBG );
	if ( 0 > nRC )
	{
		printf( "%s MPGLOG_INIT() ERROR", __func__ );
		return MPGLOG_FAIL;
	}

	/*
	 *	TAP_IPC
	 */
	nRC = TAP_ipc_open( &tIpc, SERVER_PROCESS );
	if ( 0 > nRC )
	{
		LogErr( __func__, ipc_errno );
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		LogErr( __func__, ipc_errno );
		nRC = TAP_ipc_close( &tIpc );
		if ( 0 > nRC )
		{
			LogErr(  __func__, ipc_errno );
		}
		return TAP_FAIL;
	}

	/*
	 *	List _ Key: Index(intcmp) | Value: ID
	 */
	tList = LIST_INIT( MEM_DEFAULT, FLAG_OW, STRCMP, NULL );
	if ( NULL == tList )
	{
		MPGLOG_ERR( "%s:: LIST_INIT fail", __func__ );
		return LIST_FAIL;		
	}

	/*
	 *	Hash _ Key: ID(strcmp) | Value: Info
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

	nRC = UTIL_LoadFile( &tList, &tHash );
	if ( SUCCESS != nRC && FILE_EMPTY != nRC )
	{
		MPGLOG_ERR( "%s:: UTIL_LoadFile fail", __func__ );
		return HASH_FAIL;
	}

	/*
	 *	Run Program (Server)
	 */
	while ( FLAG_RUN == g_nFlag )
	{
		memset( &tRecvMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &tSendMsg, 0x00, sizeof(iipc_msg_t) );

		nRC = TAP_ipc_msgrcv( &tIpc, &tRecvMsg, IPC_BLOCK );
		if ( 0 > nRC )
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

				nRC = Insert( &tList, &tHash, ptRequest, ptResponse );
				if ( SUCCESS != nRC )
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

				nRC = SelectAll( &tList, &tHash, ptResponse );
				if ( SUCCESS != nRC && HASH_EMPTY != nRC )
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

				nRC = SelectOne( &tHash, ptRequest, ptResponse );
				if ( SUCCESS != nRC )
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

				nRC = Update( &tHash, ptRequest );
				if ( SUCCESS != nRC )
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

				nRC = Delete( &tList, &tHash, ptRequest );
				if ( SUCCESS != nRC )
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

		nRC = TAP_ipc_msgsnd( &tIpc, &tSendMsg, IPC_BLOCK );
		if ( 0 > nRC )
		{
			LogErr( __func__, ipc_errno );
			continue;
		}

		LIST_DUMP( tList, NULL );
		HASH_DUMP( tHash, NULL );

		nExit = 0;
		printf( "서버 종료 및 파일 저장(1): " );
		nRC = scanf( "%d", &nExit );
		if ( 1 == nExit )
		{
			break;
		}
	}

	nRC = UTIL_SaveInFile( &tList, &tHash );
	if ( SUCCESS != nRC && HASH_EMPTY != nRC )
	{
		MPGLOG_ERR( "%s:: UTIL_SaveInFile() fail", __func__ );
		return FUNC_FAIL;
	}
	else if ( SUCCESS == nRC )
	{
		MPGLOG_DBG( "%s:: UTIL_SaveInFile() success", __func__ );
	}

	LIST_RELEASE( tList );
	HASH_RELEASE( tHash );

	nRC = TAP_ipc_close( &tIpc );
	if ( 0 > nRC )
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

int UTIL_LoadFile( linked_list_t *ptList, hash_table_t *ptHash )
{
	if ( NULL == ptList )
	{
		MPGLOG_ERR( "%s:: parameter(ptList) NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}

	int i = 0;
	int nRC = 0;
	
	char *pszDefaultStr = NULL;
	
	char szListIndex[SIZE_ID + 1];
	memset( szListIndex, 0x00, sizeof(szListIndex) );
	
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

		snprintf( szListIndex, sizeof(szListIndex), "%d", i );
		szListIndex[ strlen(szListIndex) ] = '\0';

		nRC = LIST_COPY( *ptList, szListIndex, strlen(szListIndex), ptSectList->name[i], sizeof(ptSectList->name[i]) );
		if ( 0 == nRC )
		{
			MPGLOG_ERR( "%s:: LIST_COPY fail", __func__ );
			return LIST_FAIL;
		}

		nRC = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], NAME, tInfo.szName, sizeof(tInfo.szName), pszDefaultStr );
		nRC = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], JOBTITLE, tInfo.szJobTitle, sizeof(tInfo.szJobTitle), pszDefaultStr );
		nRC = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], TEAM, tInfo.szTeam, sizeof(tInfo.szTeam), pszDefaultStr );
		nRC = mpconf_get_str( NULL, g_pszFile, ptSectList->name[i], PHONE, tInfo.szPhone, sizeof(tInfo.szPhone), pszDefaultStr );

		nRC = HASH_COPY( *ptHash, ptSectList->name[i], strlen(ptSectList->name[i]), &tInfo, sizeof(INFO_t) );
		if ( 0 == nRC )
		{
			MPGLOG_ERR( "%s:: HASH_COPY fail", __func__ );
			return HASH_FAIL;
		}
	}

	ListFree( ptSectList, NULL, NULL );

	LIST_DUMP( *ptList, NULL );
	HASH_DUMP( *ptHash, NULL );

	return SUCCESS;
}

int UTIL_SaveInFile( linked_list_t *ptList, hash_table_t *ptHash )
{
	if ( NULL == ptList )
	{
		MPGLOG_ERR( "%s:: parameter(ptList) NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptHash )
	{
		MPGLOG_ERR( "%s:: parameter(ptHash) NULL", __func__ );
		return NULL_FAIL;
	}

	int i = 0;
	int nRC = 0;
	long lListEntry = 0;
	
	char szListIndex[SIZE_ID + 1];
	memset( szListIndex, 0x00, sizeof(szListIndex) );

	char* pszId = NULL;
	char *pszInfo = NULL;
	INFO_t *ptInfo = NULL;

	lListEntry = LIST_COUNT( *ptList );
	
	nRC = rename( g_pszFile, g_pszBackup );
	if ( 0 != nRC )
	{
		MPGLOG_ERR( "%s:: rename fail=%d", __func__, errno );
		return FUNC_FAIL;	
	}
	
	for ( i = 0; i < lListEntry; i++ )
	{
		snprintf( szListIndex, sizeof(szListIndex), "%d", i );
		szListIndex[ strlen(szListIndex) ] = '\0';

		pszId = LIST_SEARCH( *ptList, szListIndex );
		if ( 0 == pszId )
		{
			MPGLOG_ERR( "%s:: LIST_SEARCH fail=%d", __func__, errno );
			return LIST_FAIL;
		}

		pszInfo = HASH_SEARCH( *ptHash, pszId );
		if ( 0 == pszInfo )
		{
			MPGLOG_ERR( "%s:: HASH_SEARCH fail=%d", __func__, errno );
			return HASH_FAIL;
		}
	
		ptInfo = (INFO_t *)pszInfo;
			
		nRC = mpconf_set_str( NULL, g_pszFile, pszId, NAME, ptInfo->szName );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_set_str( NULL, g_pszFile, pszId, JOBTITLE, ptInfo->szJobTitle );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_set_str( NULL, g_pszFile, pszId, TEAM, ptInfo->szTeam );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_set_str( NULL, g_pszFile, pszId, PHONE, ptInfo->szPhone );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_set_str() fail", __func__ );
			return MPCONF_FAIL;
		}
	}	

	return SUCCESS;
}

int Insert( linked_list_t *ptList, hash_table_t *ptHash, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptList )
	{
		MPGLOG_ERR( "%s:: parameter(ptList) NULL", __func__ );
		return NULL_FAIL;
	}

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

	int nRC = 0;
	int lListEntry = 0;
	
	char szListIndex[SIZE_ID + 1];
	memset( szListIndex, 0x00, sizeof(szListIndex) );

	char szId[SIZE_ID + 1];
	memset( szId, 0x00, sizeof(szId) );

	INFO_t tInfo;
	memset( &tInfo, 0x00, sizeof(tInfo) );

	nRC = SetUniqueId( &(ptRequest->nId) );
	if ( SUCCESS != nRC )
	{
		return FUNC_FAIL;
	}

	ptResponse->nId = ptRequest->nId;
	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );
	szId[ strlen(szId) ] = '\0';

	strlcpy( tInfo.szName, ptRequest->szName, sizeof(tInfo.szName) ); 
	strlcpy( tInfo.szJobTitle, ptRequest->szJobTitle, sizeof(tInfo.szJobTitle) );
	strlcpy( tInfo.szTeam, ptRequest->szTeam, sizeof(tInfo.szTeam) );
	strlcpy( tInfo.szPhone, ptRequest->szPhone, sizeof(tInfo.szPhone) );

	lListEntry = LIST_COUNT( *ptList );
	snprintf( szListIndex, sizeof(szListIndex), "%d", lListEntry );
   	szListIndex[ strlen(szListIndex) ] = '\0';	

	nRC = LIST_COPY( *ptList, szListIndex, strlen(szListIndex), szId, sizeof(szId) );
	if ( 0 == nRC )
	{
		MPGLOG_ERR( "%s:: LIST_COPY() fail", __func__ );
		return LIST_FAIL;
	}

	nRC = HASH_COPY( *ptHash, szId, strlen(szId), &tInfo, sizeof(tInfo) );
	if ( 0 == nRC )
	{
		MPGLOG_ERR( "%s:: HASH_COPY() fail", __func__ );
		return HASH_FAIL;
	}

	return SUCCESS;
}

int SelectAll( linked_list_t *ptList, hash_table_t *ptHash, struct RESPONSE_s *ptResponse )
{
	if ( NULL == ptList )
	{
		MPGLOG_ERR( "%s:: parameter(ptList) NULL", __func__ );
		return NULL_FAIL;
	}

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
	long lListEntry = 0;
	
	char szListIndex[SIZE_ID + 1];
	memset( szListIndex, 0x00, sizeof(szListIndex) );
	
	char *pszId = NULL;
	char *pszInfo = NULL;

	INFO_t *ptInfo = NULL;

	ID_NAME_t tIdName;
	memset( &tIdName, 0x00, sizeof(tIdName) );

	lListEntry = LIST_COUNT( *ptList );

	for ( i = 0; i < lListEntry; i++ )
	{
		snprintf( szListIndex, sizeof(szListIndex), "%d", i );
		szListIndex[ strlen(szListIndex) ] = '\0';	
	
		pszId = LIST_SEARCH( *ptList, szListIndex );
		if ( 0 == pszId )
		{
			MPGLOG_ERR( "%s:: LIST_SEARCH fail=%d", __func__, errno );
			return LIST_FAIL;
		}

		pszInfo = HASH_SEARCH( *ptHash, pszId );
		if ( 0 == pszInfo )
		{
			MPGLOG_ERR( "%s:: HASH_SEARCH fail=%d", __func__, errno );
			return HASH_FAIL;
		}

		ptInfo = (INFO_t *)pszInfo;	
		
		tIdName.nId = atoi(pszId);
		strlcpy( tIdName.szName, ptInfo->szName, sizeof(tIdName.szName) );

		memcpy( ptResponse->szBuffer + (i * sizeof(tIdName)), &tIdName, sizeof(tIdName) );
	}

	ptResponse->nCntSelectAll = lListEntry;

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

	char *pszInfo = NULL;
	
	char szId[SIZE_ID + 1];
	memset( szId, 0x00, sizeof(szId) );

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );
	szId[ strlen(szId) ] = '\0';

	pszInfo = HASH_SEARCH( *ptHash, szId );
	if ( 0 == pszInfo )
	{
		if ( 1003 == errno )
		{
			MPGLOG_DBG( "%s:: id not exist", __func__ );
			return ID_NOT_EXIST;
		}
		MPGLOG_ERR( "%s:: HASH_SEARCH fail", __func__ );
		return HASH_FAIL;
	}

	memcpy( ptResponse->szBuffer, pszInfo, sizeof(struct INFO_s) ); 

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

	int nRC = 0;

	char *pszInfo = NULL;
	
	char szId[SIZE_ID + 1];
	memset( szId, 0x00, sizeof(szId) );

	INFO_t *ptOldInfo = NULL;
	
	INFO_t tNewInfo;
	memset( &tNewInfo, 0x00, sizeof(tNewInfo) );

	snprintf( szId, sizeof(szId), "%d", ptRequest->nId );
	szId[ strlen(szId) ] = '\0';

	pszInfo = HASH_SEARCH( *ptHash, szId );
	if ( 0 == pszInfo )
	{
		MPGLOG_ERR( "%s:: HASH_SEARCH fail", __func__ );
		return HASH_FAIL;
	}
	
	ptOldInfo = (INFO_t *)pszInfo;
	
	if ( 0 == strlen(ptRequest->szName) )
	{
		strlcpy( tNewInfo.szName, ptOldInfo->szName, sizeof(tNewInfo.szName) );
	}
	else
	{
		strlcpy( tNewInfo.szName, ptRequest->szName, sizeof(tNewInfo.szName) );
	}

	if ( 0 == strlen(ptRequest->szJobTitle) )
	{
		strlcpy( tNewInfo.szJobTitle, ptOldInfo->szJobTitle, sizeof(tNewInfo.szJobTitle) );
	}
	else
	{
		strlcpy( tNewInfo.szJobTitle, ptRequest->szJobTitle, sizeof(tNewInfo.szJobTitle) );
	}

	if ( 0 == strlen(ptRequest->szTeam) )
	{
		strlcpy( tNewInfo.szTeam, ptOldInfo->szTeam, sizeof(tNewInfo.szTeam) );
	}
	else
	{
		strlcpy( tNewInfo.szTeam, ptRequest->szTeam, sizeof(tNewInfo.szTeam) );
	}

	if ( 0 == strlen(ptRequest->szPhone) )
	{
		strlcpy( tNewInfo.szPhone, ptOldInfo->szPhone, sizeof(tNewInfo.szPhone) );
	}
	else
	{
		strlcpy( tNewInfo.szPhone, ptRequest->szPhone, sizeof(tNewInfo.szPhone) );
	}
	
	nRC = HASH_COPY( *ptHash, szId, strlen(szId), &tNewInfo, sizeof(tNewInfo) );
	if ( 0 == nRC )
	{
		MPGLOG_ERR( "%s:: HASH_COPY fail=%d", __func__, errno );
		if ( 1003 == errno )
		{
			MPGLOG_DBG( "%s:: id not exist", __func__ );
			return ID_NOT_EXIST;
		}
		return HASH_FAIL;
	}

	return SUCCESS;
}

int Delete( linked_list_t *ptList, hash_table_t *ptHash, struct REQUEST_s *ptRequest )
{
	if ( NULL == ptList )
	{
		MPGLOG_ERR( "%s:: parameter(ptList) NULL", __func__ );
		return NULL_FAIL;
	}

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

	int i = 0;
	int lListEntry = 0;

	char szListIndex[SIZE_ID + 1];
	memset( szListIndex, 0x00, sizeof(szListIndex) );

	char *pszId = NULL;
	char *pszDeleteId = NULL;
	char *pszDeleteInfo = NULL;

	char szId[SIZE_ID + 1];
	memset( szId, 0x00, sizeof(szId) );

	lListEntry = LIST_COUNT( *ptList );

	for ( i = 0; i < lListEntry; i++ )
	{
		snprintf( szListIndex, sizeof(szListIndex), "%d", i );
		szListIndex[ strlen(szListIndex) ] = '\0';

		pszId = LIST_SEARCH( *ptList, szListIndex );

		if ( atoi(pszId) == ptRequest->nId )
		{
			pszDeleteId = LIST_DELETE( *ptList, szListIndex );
			if ( 0 == pszDeleteId )
			{
				MPGLOG_ERR( "%s:: LIST_DELETE fail=%d", __func__, errno );	
				return LIST_FAIL;
			}
			break;
		}
		
		if ( i == (lListEntry - 1) && ptRequest->nId != atoi(pszId) )
		{
			MPGLOG_DBG( "%s:: id not exist", __func__ );
			return ID_NOT_EXIST;
		}
	}

	pszDeleteInfo = HASH_DELETE( *ptHash, pszId );
	if ( 0 == pszDeleteInfo )
	{
		MPGLOG_ERR( "%s:: HASH_DELETE fail=%d", __func__, errno );
		if ( 1003 == errno )
		{
			MPGLOG_ERR( "%s:: id not exist", __func__ );
			return ID_NOT_EXIST;
		}
		return HASH_FAIL;
	}

	return SUCCESS;
}
