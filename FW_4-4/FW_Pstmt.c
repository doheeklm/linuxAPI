/* FW_Pstmt.c */
#include "FW_Header.h"

int PSTMT_Init()
{
	char szQuery[256];

	//INSERT
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s);",
			TABLE_NAME, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE,
			TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtInsert = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtInsert )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select * from %s", TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtSelectAll = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"delete from %s where %s = ?%s;",
			TABLE_NAME, TABLE_ATT_ID, TABLE_ATT_ID );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtDelete = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//NUMTUPLES
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"select %s from __SYS_TABLES__ where TABLE_NAME='%s'",
			NUMTUPLES, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtNumTuples = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtNumTuples )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void PSTMT_Destroy()
{
	int nRC = 0;

	nRC = dalDestroyPreparedStmt( g_ptPstmtInsert );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}
	
	nRC = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtNumTuples );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	return;
}
