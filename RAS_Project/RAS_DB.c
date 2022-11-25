/* RAS_DB.c */
#include "RAS_Inc.h"

int DB_Init( DAL_CONN **pptDBConn )
{
	DAL_CONN *ptDBConn = NULL;

	ptDBConn = dalConnect( NULL );
	if ( NULL == ptDBConn )
	{
		LOG_ERR_F( "dalConnect fail" );
		return RAS_rErrDBConn;
	}

	*pptDBConn = ptDBConn;

	return RAS_rOK;
}

int DB_InitPreparedStatement( DB_t *ptDB )
{
	CHECK_PARAM_RC( ptDB );

	DB_INIT_PSTMT( ptDB, PSTMT_INSERT_IP,			SQL_INSERT_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_IP_ALL,		SQL_SELECT_IP_ALL );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_IP_BY_IP,		SQL_SELECT_IP_BY_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_DELETE_IP,			SQL_DELETE_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_INSERT_INFO,			SQL_INSERT_INFO );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_INFO_ALL,		SQL_SELECT_INFO_ALL );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_INFO_BY_ID,	SQL_SELECT_INFO_BY_ID );
	DB_INIT_PSTMT( ptDB, PSTMT_DELETE_INFO,			SQL_DELETE_INFO );
	DB_INIT_PSTMT( ptDB, PSTMT_NUMTUPLES_INFO,		SQL_NUMTUPLES_INFO );

#if 0
	int nIndex = 0;
	for ( nIndex = 0; nIndex < PSTMT_MAX; nIndex++ )
	{
		printf( "[%d] %s\n", nIndex, dalPreparedGetQuery( ptDB->patPstmt[nIndex] ) );
	}	
#endif

	return RAS_rOK;
}

void DB_Close( DB_t *ptDB )
{
	CHECK_PARAM( ptDB, return );
	
	int nRC = 0;
	int nIndex = 0;

	for ( nIndex = 0; nIndex < PSTMT_MAX; nIndex ++ )
	{
		if ( NULL != ptDB->patPstmt[nIndex] )
		{
			nRC = dalDestroyPreparedStmt( ptDB->patPstmt[nIndex] );
			if ( -1 == nRC )
			{
				//LOG_ERR_F( "dalDestroyPreparedStmt fail <%d:%s>", dalErrno(), dalErrmsg(dalErrno()) );
			}
			ptDB->patPstmt[nIndex] = NULL;
		}
	}
	
	if ( NULL != ptDB->ptDBConn )
	{
		nRC = dalDisconnect( ptDB->ptDBConn );
		if ( -1 == nRC )
		{
			LOG_ERR_F( "dalDisconnect fail <%d:%s>", dalErrno(), dalErrmsg(dalErrno()) );
		}
		else if ( 0 == nRC )
		{
			LOG_DBG_F( "dalDisconnect succes" );
		}
		ptDB->ptDBConn = NULL;
	}

	return;
}
