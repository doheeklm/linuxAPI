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

	DB_INIT_PSTMT( ptDB, PSTMT_INSERT_IP, SQL_INSERT_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_IP_ALL, SQL_SELECT_IP_ALL );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_IP_BY_IP, SQL_SELECT_IP_BY_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_DELETE_IP, SQL_DELETE_IP );

	DB_INIT_PSTMT( ptDB, PSTMT_INSERT_INFO, SQL_INSERT_INFO );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_INFO_ALL, SQL_SELECT_INFO_ALL );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_INFO_BY_ID, SQL_SELECT_INFO_BY_ID );
	DB_INIT_PSTMT( ptDB, PSTMT_DELETE_INFO, SQL_DELETE_INFO );

	DB_INIT_PSTMT( ptDB, PSTMT_NUMTUPLE_INFO, SQL_NUMTUPLE_INFO );

	return RAS_rOK;
}

void DB_ClosePreparedStatement( DB_t *ptDB )
{
	CHECK_PARAM( ptDB, return );

	int nIndex = 0;

	for ( nIndex = 0; nIndex < PSTMT_MAX; nIndex++ )
	{
		DB_DESTROY_PSTMT( ptDB, nIndex );
	}

	return;
}

void DB_Close( DB_t *ptDB )
{
	CHECK_PARAM( ptDB, return );

	DB_ClosePreparedStatement( ptDB );

	LOG_DBG_F( "DB_ClosePreparedStatement success" );

	if ( NULL != ptDB->ptDBConn )
	{
		dalDisconnect( ptDB->ptDBConn );
		ptDB->ptDBConn = NULL;
		
		LOG_DBG_F( "dalDisconnect succes" );
	}

	return;
}
