/* RAS_DB.h */
#ifndef _RAS_DB_H_
#define _RAS_DB_H_

#define CLI_IP_TBL				"RAS_CLI_IP"
#define ATTR_CLI_IP				"cliIp"
#define ATTR_CLI_IP_DESC		"cliIpDesc"

#define USR_INFO_TBL			"RAS_USR_INFO"
#define ATTR_ID					"id"
#define ATTR_NAME				"name"
#define ATTR_GENDER				"gender"
#define ATTR_BIRTH				"birth"
#define ATTR_ADDRESS			"address"

#define SQL_INSERT_CLI_IP		"insert into "CLI_IP_TBL" ("ATTR_CLI_IP", "ATTR_CLI_IP_DESC") values (?"ATTR_CLI_IP", ?"ATTR_CLI_IP_DESC");"
#define SQL_SELECT_CLI_IP		"select * from "CLI_IP_TBL" where "ATTR_CLI_IP"=?"ATTR_CLI_IP";"
#define SQL_DELETE_CLI_IP		"delete from "CLI_IP_TBL" where "ATTR_CLI_IP"=?"ATTR_CLI_IP";"
#define SQL_INSERT_USR_INFO		"insert into "USR_INFO_TBL" ("ATTR_ID", "ATTR_NAME", "ATTR_GENDER", "ATTR_BIRTH", "ATTR_ADDRESS") values (?"ATTR_ID", ?"ATTR_NAME", ?"ATTR_GENDER", ?"ATTR_BIRTH", ?"ATTR_ADDRESS");"
#define SQL_SELECT_USR_INFO		"select * from "USR_INFO_TBL";"
#define SQL_DELETE_USR_INFO		"delete from "USR_INFO_TBL" where "ATTR_ID"=?"ATTR_ID";"
#define SQL_ALARM_CNT_USR		"select NUMTUPLES from __SYS__TABLES__ where TABLE_NAME="USR_INFO_TBL";"

typedef enum
{
	PSTMT_INSERT_CLI_IP = 0,
	PSTMT_SELECT_CLI_IP,
	PSTMT_DELETE_CLI_IP,
	PSTMT_INSERT_USR_INFO,
	PSTMT_SELECT_USR_INFO,
	PSTMT_DELETE_USR_INFO,
	PSTMT_ALARM_CNT_USR,
	PSTMT_MAX
} Pstmt_e;

typedef struct DB_s
{
	DAL_CONN	*ptDBConn;
	DAL_PSTMT	*patPstmt[PSTMT_MAX];
} DB_t;

#define DB_INIT_PSTMT( p_db, _index, _sql ) \
	do { \
		(p_db)->patPstmt[(_index)] = dalPreparedStatement( (p_db)->ptDBConn, (_sql) ); \
		if ( NULL == (p_db)->patPstmt[(_index)] ) \
		{ \
			LOG_ERR_F( "dalPreparedStatement fail <%d:%s> (%d:%s)",\
				   	dalErrno(), dalErrmsg(dalErrno()), (_index), (_sql) ); \
			return RAS_rErrDBPstmt; \
		} \
	} while (0)

#define DB_DESTROY_PSTMT( p_db, _index ) \
	do { \
		if ( NULL != (p_db)->patPstmt[(_index)] ) \
		{ \
			if ( 0 > dalDestroyPreparedStmt( (p_db)->patPstmt[(_index)] ) ) \
			{ \
				LOG_ERR_F( "dalDestroyPreparedStatement fail <%d:%s>", dalErrno(),dalErrmsg(dalErrno()) ); \
				return; \
			} \
			(p_db)->patPstmt[(_index)] = NULL; \
		} \
	} while (0)

#define DB_FREE( p_res ) \
	do { \
		if ( NULL != p_res ) \
		{ \
			if ( -1 == dalResFree( p_res ) ) \
			{ \
				LOG_ERR_F( "dalResFree fail <%d>", dalErrno() ); \
				return RAS_rErrDBResFree; \
			} \
			p_res = NULL; \
		} \
	} while (0)

#define DB_SET_INT_BY_KEY( p_pstmt, _key, _value ) \
	do { \
		if ( -1 == dalSetIntByKey( p_pstmt, _key, _value ) ) \
		{ \
			LOG_ERR_F( "dalSetIntByKey fail <%d>", dalErrno() ); \
			return RAS_rErrDBSetValue; \
		} \
	} while (0)

#define DB_SET_STRING_BY_KEY( p_pstmt, _key, _value ) \
	do { \
		if ( -1 == dalSetStringByKey( p_pstmt, _key, _value ) ) \
		{ \
			LOG_ERR_F( "dalSetStringByKey fail <%d>", dalErrno() ); \
			return RAS_rErrDBSetValue; \
		} \
	} while (0)

#define DB_PREPARED_EXEC( p_conn, p_pstmt, pp_res, _rc ) \
	do { \
		_rc = dalPreparedExec( p_conn, p_pstmt, pp_res ); \
		if ( -1 == _rc ) \
		{ \
			LOG_ERR_F( "dalPreparedExec fail <%d>", dalErrno() ); \
			return RAS_rErrDBExecute; \
		} \
	} while (0)

int DB_Init( DAL_CONN **pptDBConn );
int DB_InitPreparedStatement( DB_t *ptDB );
void DB_ClosePreparedStatement( DB_t *ptDB );
void DB_Close( DB_t *ptDB );

#endif /* _RAS_DB_H_ */
