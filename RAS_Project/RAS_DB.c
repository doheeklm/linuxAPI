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

	DB_INIT_PSTMT( ptDB, PSTMT_INSERT_CLI_IP, SQL_INSERT_CLI_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_CLI_IP, SQL_SELECT_CLI_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_DELETE_CLI_IP, SQL_DELETE_CLI_IP );
	DB_INIT_PSTMT( ptDB, PSTMT_INSERT_USR_INFO, SQL_INSERT_USR_INFO );
	DB_INIT_PSTMT( ptDB, PSTMT_SELECT_USR_INFO, SQL_SELECT_USR_INFO );
	DB_INIT_PSTMT( ptDB, PSTMT_DELETE_USR_INFO, SQL_DELETE_USR_INFO );
	DB_INIT_PSTMT( ptDB, PSTMT_ALARM_CNT_USR, SQL_ALARM_CNT_USR );

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
	if ( NULL == ptDB )
	{
		return;
	}

	DB_ClosePreparedStatement( ptDB );

	LOG_SVC_F( "DB_ClosePreparedStatement success" );

	if ( NULL != ptDB->ptDBConn )
	{
		dalDisconnect( ptDB->ptDBConn );
		ptDB->ptDBConn = NULL;
		
		LOG_SVC_F( "dalDisconnect succes" );
	}

	return;
}
