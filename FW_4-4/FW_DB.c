/* FW_DB.c */
#include "FW_Header.h"

int DB_CheckDuplicate( char *pszPhone )
{
	int nRC = 0;

	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );

	DAL_RESULT_SET *ptRes = NULL;

	snprintf( szQuery, sizeof(szQuery), "select * from %s where %s = '%s';",
			TABLE_NAME, TABLE_ATT_PHONE, pszPhone );
	szQuery[ strlen(szQuery) ] = '\0';

	nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalExecute() fail=%d", __func__, nRC );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		nRC = dalResFree( ptRes );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, nRC );
			return DAL_FAIL;
		}
		return RC_SUCCESS;
	}

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, nRC );
		return DAL_FAIL;
	}

	return RC_FAIL;	
}

int DB_Insert( struct REQUEST_s *ptRequestFromClient )
{
	if ( NULL == ptRequestFromClient )
	{
		MPGLOG_ERR( "%s:: ptRequestFromClient NULL", __func__ );
		return NULL_FAIL;		
	}

	int nRC = 0;

	char szKey[TAP_REGI_KEY_SIZE];
 	memset( szKey, 0x00, sizeof(szKey) );

	nRC = DB_CheckDuplicate( ptRequestFromClient->szPhone );
	if ( RC_SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: DB_CheckDuplicate() fail=%d", __func__, nRC );
		return RC_FAIL;
	}
	else
	{
		MPGLOG_DBG( "%s:: no duplicate phone num [%s]", __func__, ptRequestFromClient->szPhone );	
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_NAME, ptRequestFromClient->szName );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_POSITION, ptRequestFromClient->szPosition );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_TEAM, ptRequestFromClient->szTeam );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_PHONE, ptRequestFromClient->szPhone );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() = 0", __func__ );
		return DAL_EXEC_ZERO;
	}

	snprintf( szKey, sizeof(szKey), "%s%s", REGI_KEY_DIR, ptRequestFromClient->szPhone );
	szKey[ strlen(szKey) ] = '\0';

	nRC = TAP_Registry_udp_key_create( szKey, strlen(szKey), TAP_REGISTRY_FILE, REGI_MAN_SYSTEM_ID );
	if ( TAP_REGI_ALREADY_EXIST == nRC )
	{
		MPGLOG_DBG( "%s:: create key", __func__ );
	}
	else if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_key_create() fail=%d", __func__ , nRC );
		return REGI_FAIL;
	}

	return RC_SUCCESS;
}

int DB_Select( struct REQUEST_s *ptRequestFromClient, struct RESPONSE_s *ptResponseToClient )
{
	if ( NULL == ptRequestFromClient )
	{
		MPGLOG_DBG( "%s:: ptResponseToClient NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptResponseToClient )
	{
		MPGLOG_DBG( "%s:: ptResponseToClient NULL", __func__ );
	}

	int nRC = 0;
	int nCnt = 0;

	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );

	int nId = 0;
	char* pszName = NULL;
	char* pszPosition = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;
	
	SELECT_t tSelect;
	
	DAL_ENTRY *ptEntry = NULL;
	DAL_RESULT_SET *ptRes = NULL;

	if ( MTYPE_SELECTALL == ptRequestFromClient->nMsgType )
	{
		nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtSelectAll, &ptRes );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRC )
		{
			nRC = dalResFree( ptRes );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				return DAL_FAIL;
			}

			MPGLOG_ERR( "%s:: no info exist", __func__ );
			return DAL_EXEC_ZERO;
		}
		for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
		{
			memset( &tSelect, 0x00, sizeof(tSelect) );

			nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			tSelect.nId = nId;
			strlcpy( tSelect.szName, pszName, sizeof(tSelect.szName) );
			strlcpy( tSelect.szPosition, pszPosition, sizeof(tSelect.szPosition) );
			strlcpy( tSelect.szTeam, pszTeam, sizeof(tSelect.szTeam) );
			strlcpy( tSelect.szPhone, pszPhone, sizeof(tSelect.szPhone) );

			// 버퍼사이즈를 초과하여 중간에 메모리가 잘릴 가능성이 있으면 memcpy 하지 않고 break한다.
			if ( ( (nCnt + 1) * sizeof(tSelect) ) > sizeof(ptResponseToClient->szBuffer) )
			{
				MPGLOG_DBG( "%s:: 현재까지 읽은 사이즈[%ld], 버퍼사이즈[%ld]:"
						"버퍼 사이즈 초과될 가능성이 있어 memcpy 하지 않음",
						__func__, nCnt * sizeof(tSelect), sizeof(ptResponseToClient->szBuffer) );
				break;
			}

			memcpy( ptResponseToClient->szBuffer + (nCnt*sizeof(tSelect)), &tSelect, sizeof(tSelect) );

			nCnt++;
		}

		ptResponseToClient->nId = 0;
		ptResponseToClient->nCntSelectAll = nCnt;
	}
	else if ( MTYPE_SELECTONE == ptRequestFromClient->nMsgType )
	{
		snprintf( szQuery, sizeof(szQuery), "select * from %s where %s = %d;",
				TABLE_NAME, TABLE_ATT_ID, ptRequestFromClient->nId );
		szQuery[ strlen(szQuery) ] = '\0';

		nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalExecute() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}
		else if ( 0 == nRC )
		{
			nRC = dalResFree( ptRes );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				return DAL_FAIL;
			}

			MPGLOG_ERR( "%s:: id not exist", __func__ );
			return DAL_EXEC_ZERO;
		}

		ptEntry = dalFetchFirst( ptRes );
		if ( NULL != ptEntry )
		{
			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}	

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto error_return;
			}

			nCnt++;
		}

		ptResponseToClient->nId = ptRequestFromClient->nId;
		ptResponseToClient->nCntSelectAll = nCnt;

		tSelect.nId = ptRequestFromClient->nId;
		strlcpy( tSelect.szName, pszName, sizeof(tSelect.szName) );
		strlcpy( tSelect.szPosition, pszPosition, sizeof(tSelect.szPosition) );
		strlcpy( tSelect.szTeam, pszTeam, sizeof(tSelect.szTeam) );
		strlcpy( tSelect.szPhone, pszPhone, sizeof(tSelect.szPhone) );

		memcpy( ptResponseToClient->szBuffer, &tSelect, sizeof(tSelect) );
	}
	else
	{
		MPGLOG_ERR( "%s:: nMsgType Fail", __func__ );
		goto error_return;
	}

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return RC_SUCCESS;

error_return:
	
	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}

	return DAL_FAIL;
}

int DB_Update( struct REQUEST_s *ptRequestFromClient )
{
	if ( NULL == ptRequestFromClient )
	{
		MPGLOG_ERR( "%s:: ptRequestFromClient NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;
	int nCommaFlag = 0;
	
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;

	char szSelectQuery[256];
	char szUpdateQuery[256];
	char szTemp[256];

	memset( szSelectQuery, 0x00, sizeof(szSelectQuery) );
	memset( szUpdateQuery, 0x00, sizeof(szUpdateQuery) );
	memset( szTemp, 0x00, sizeof(szTemp) );

	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	snprintf( szSelectQuery, sizeof(szSelectQuery), "select * from %s where %s = %d;", TABLE_NAME, TABLE_ATT_ID, ptRequestFromClient->nId );
	szSelectQuery[ strlen(szSelectQuery) ] = '\0';

	nRC = dalExecute( g_ptDalConn, szSelectQuery, &ptRes );
	if ( 0 == nRC )
	{
		MPGLOG_ERR( "%s:: dalExecute() fail=0", __func__ );
		nRC = dalResFree( ptRes );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		}
		return DAL_EXEC_ZERO;
	}
	else if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalExecute() fail=%d", __func__, nRC );
		return DAL_FAIL;
	}

	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;	
		}

		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}

		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}	

		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
	}

	snprintf( szTemp, sizeof(szTemp), "update %s set ", TABLE_NAME );
	szTemp[ strlen(szTemp) ] = '\0';
	strncat( szUpdateQuery, szTemp, strlen(szTemp) );

	if ( 0 < strlen(ptRequestFromClient->szName) )
	{
		memset( szTemp, 0x00, sizeof(szTemp) );	
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_NAME, ptRequestFromClient->szName );
		szTemp[ strlen(szTemp) ] = '\0';
		strncat( szUpdateQuery, szTemp, strlen(szTemp) );

		nCommaFlag = 1;
	}

	if ( 0 < strlen(ptRequestFromClient->szPosition) )
	{
		if ( 1 == nCommaFlag )
		{
			memset( szTemp, 0x00, sizeof(szTemp) );	
			snprintf( szTemp, sizeof(szTemp), ", %s = '%s'", TABLE_ATT_POSITION, ptRequestFromClient->szPosition );
			szTemp[ strlen(szTemp) ] = '\0';
			strncat( szUpdateQuery, szTemp, strlen(szTemp) );
		}
		else
		{
			memset( szTemp, 0x00, sizeof(szTemp) );	
			snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_POSITION, ptRequestFromClient->szPosition );
			szTemp[ strlen(szTemp) ] = '\0';
			strncat( szUpdateQuery, szTemp, strlen(szTemp) );
			
			nCommaFlag = 1;
		}
	}

	if ( 0 < strlen(ptRequestFromClient->szTeam) )
	{
		if ( 1 == nCommaFlag )
		{
			memset( szTemp, 0x00, sizeof(szTemp) );	
			snprintf( szTemp, sizeof(szTemp), ", %s = '%s'", TABLE_ATT_TEAM, ptRequestFromClient->szTeam );
			szTemp[ strlen(szTemp) ] = '\0';
			strncat( szUpdateQuery, szTemp, strlen(szTemp) );
		}
		else
		{
			memset( szTemp, 0x00, sizeof(szTemp) );	
			snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_TEAM, ptRequestFromClient->szTeam );
			szTemp[ strlen(szTemp) ] = '\0';
			strncat( szUpdateQuery, szTemp, strlen(szTemp) );
			
			nCommaFlag = 1;
		}
	}

	if ( 0 < strlen(ptRequestFromClient->szPhone) )
	{
		if ( 1 == nCommaFlag )
		{
			memset( szTemp, 0x00, sizeof(szTemp) );	
			snprintf( szTemp, sizeof(szTemp), ", %s = '%s'", TABLE_ATT_PHONE, ptRequestFromClient->szPhone );
			szTemp[ strlen(szTemp) ] = '\0';
			strncat( szUpdateQuery, szTemp, strlen(szTemp) );
		}
		else
		{
			memset( szTemp, 0x00, sizeof(szTemp) );	
			snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_PHONE, ptRequestFromClient->szPhone );
			szTemp[ strlen(szTemp) ] = '\0';
			strncat( szUpdateQuery, szTemp, strlen(szTemp) );
			
			nCommaFlag = 1;
		}
	}

	memset( szTemp, 0x00, sizeof(szTemp) );
	snprintf( szTemp, sizeof(szTemp), " where %s = %d;", TABLE_ATT_ID, ptRequestFromClient->nId );
	strncat( szUpdateQuery, szTemp, strlen(szTemp) );
	szUpdateQuery[ strlen(szUpdateQuery) ] = '\0';

	MPGLOG_DBG( "szUpdateQuery: %s", szUpdateQuery );

	if ( NULL != pszName && NULL != ptRequestFromClient->szName &&
		 NULL != pszPosition && NULL != ptRequestFromClient->szPosition &&
		 NULL != pszTeam && NULL != ptRequestFromClient->szTeam &&
		 NULL != pszPhone && NULL != ptRequestFromClient->szPhone )
	{
		if ( strcmp(pszName, ptRequestFromClient->szName) == 0 &&
			 strcmp(pszPosition, ptRequestFromClient->szPosition) == 0 &&
			 strcmp(pszTeam, ptRequestFromClient->szTeam) == 0 &&
			 strcmp(pszPhone, ptRequestFromClient->szPhone) == 0 )
		{
			MPGLOG_SVC( "%s:: Old Data == New Data", __func__ ); //단 테이블에 null값이 없어야함
			return INPUT_FAIL;
		}
	}

	nRC = dalExecute( g_ptDalConn, szUpdateQuery, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalExecute() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_DBG( "%s:: id not exist", __func__ );
		return DAL_EXEC_ZERO;
	}

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	
	return RC_SUCCESS;

_exit_failure:

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}
	
	return DAL_FAIL;
}

int DB_Delete( struct REQUEST_s *ptRequestFromClient )
{
	if ( NULL == ptRequestFromClient )
	{
		MPGLOG_ERR( "%s:: ptRequestFromClient NULL", __func__ );
		return NULL_FAIL;
	}
	
	int nRC = 0;

	char szQuery[256];
	char szKey[TAP_REGI_KEY_SIZE];

	memset( szQuery, 0x00, sizeof(szQuery) );
	memset( szKey, 0x00, sizeof(szKey) );
	
	char *pszPhone = NULL;
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	snprintf( szQuery, sizeof(szQuery), "select * from %s where %s = %d;",
			TABLE_NAME, TABLE_ATT_ID, ptRequestFromClient->nId );
	szQuery[ strlen(szQuery) ] = '\0';

	nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalExecute() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		nRC = dalResFree( ptRes );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
			return DAL_FAIL;
		}

		MPGLOG_ERR( "%s:: id not exist", __func__ );
		return DAL_EXEC_ZERO;
	}

	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto error_return;
		}
	}

	/*
	 *	Delete Info in DB
	 */
	nRC = dalSetIntByKey( g_ptPstmtDelete, TABLE_ATT_ID, ptRequestFromClient->nId );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );		
		goto error_return;
	}

	nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtDelete, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		goto error_return;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_DBG( "%s:: id not exist", __func__ );

		return DAL_EXEC_ZERO;
	}

	/*
	 *	Delete Key(Phone Num) in Registry
	 */
	snprintf( szKey, sizeof(szKey), "%s%s", REGI_KEY_DIR, pszPhone );
	szKey[ strlen(szKey) ] = '\0';

	nRC = TAP_Registry_udp_key_delete( szKey, strlen(szKey), TAP_REGISTRY_UDP_REMOVEALL, REGI_MAN_SYSTEM_ID );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Regsitry_udp_key_delete() fail=%d", __func__, nRC );
		goto error_return;
	}

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return RC_SUCCESS;

error_return:

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}

	return DAL_FAIL;
}
