/* FW_4-3/FW_Pstmt.c */
#include "FW_Inc.h"

int PSTMT_Init( DAL_CONN *ptDal, DAL_PSTMT *ptPstmtInsert, DAL_PSTMT *ptPstmtSelectAll, DAL_PSTMT *ptPstmtSelectOne, DAL_PSTMT *ptPstmtUpdate, DAL_PSTMT *ptPstmtDelete, DAL_PSTMT *ptPstmtNumtuples )
{
	ptDal = dalConnect( NULL );
	if ( NULL == ptDal )
	{
		return DAL_FAIL;
	}	

	char szQuery[256];
	
	//INSERT
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s);",
			TABLE_NAME, NAME, POSITION, TEAM, PHONE, NAME, POSITION, TEAM, PHONE );
	szQuery[ strlen(szQuery) ] = '\0';
	ptPstmtInsert = dalPreparedStatement( ptDal, szQuery );
	if ( NULL == ptPstmtInsert )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"select %s, %s from %s;",
			ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';
	ptPstmtSelectAll = dalPreparedStatement( ptDal, szQuery );
	if ( NULL == ptPstmtSelectAll )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"select %s, %s, %s from %s where %s = ?%s;",
			POSITION, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';
	ptPstmtSelectOne = dalPreparedStatement( ptDal, szQuery );
	if ( NULL == ptPstmtSelectOne )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;",
			TABLE_NAME, POSITION, POSITION, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';
	ptPstmtUpdate = dalPreparedStatement( ptDal, szQuery );
	if ( NULL == ptPstmtUpdate )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"delete from %s where %s = ?%s;",
			TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';
	ptPstmtDelete = dalPreparedStatement( ptDal, szQuery );
	if ( NULL == ptPstmtDelete )
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
	ptPstmtNumtuples = dalPreparedStatement( ptDal, szQuery );
	if ( NULL == ptPstmtNumtuples )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void PSTMT_Destroy( DAL_PSTMT *ptPstmtInsert, DAL_PSTMT *ptPstmtSelectAll, DAL_PSTMT *ptPstmtSelectOne, DAL_PSTMT *ptPstmtUpdate, DAL_PSTMT *ptPstmtDelete, DAL_PSTMT *ptPstmtNumTuples )
{
	int nRC = 0;

	nRC = dalDestroyPreparedStmt( ptPstmtInsert );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( ptPstmtSelectAll );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( ptPstmtSelectOne );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( ptPstmtUpdate );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( ptPstmtDelete );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( ptPstmtNumTuples );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	return;
}
