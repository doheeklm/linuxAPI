/* RAS_DBQuery.c */
#include "RAS_Inc.h"

int DB_CheckClientIp( DB_t tDB, char *pszIp )
{
	CHECK_PARAM_RC( pszIp );

	int nRC = 0;

	DAL_RESULT_SET *ptRes = NULL;

	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );

	nRC = dalPreparedExec( tDB.ptDBConn, tDB.patPstmt[PSTMT_SELECT_CLI_IP], &ptRes );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "dalPreparedExec fail <%d>", dalErrno() );
		return RAS_rErrDBExecute;
	}
	else if ( 0 == nRC )
	{
		DB_FREE( ptRes );

		return RAS_rErrDBNotFound;
	}

	DB_FREE( ptRes );

	return RAS_rOK;
}
