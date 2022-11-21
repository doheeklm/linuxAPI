/* RAS_DB.h */
#ifndef _RAS_DB_H_
#define _RAS_DB_H_

#define CLI_IP_TBL				"RAS_CLI_IP"
#define ATTR_IP					"cliIp"
#define ATTR_DESC				"cliIpDesc"

#define USR_INFO_TBL			"RAS_USR_INFO"
#define ATTR_ID					"id"
#define ATTR_NAME				"name"
#define ATTR_GENDER				"gender"
#define ATTR_BIRTH				"birth"
#define ATTR_ADDRESS			"address"

#define SQL_INSERT_IP			"insert into "CLI_IP_TBL" "\
								"("ATTR_IP", "ATTR_DESC") values "\
								"(?"ATTR_IP", ?"ATTR_DESC");"
#define SQL_SELECT_IP_ALL		"select * from "CLI_IP_TBL";"
#define SQL_SELECT_IP_BY_IP		"select * from "CLI_IP_TBL" where "ATTR_IP"=?"ATTR_IP";"
#define SQL_DELETE_IP			"delete from "CLI_IP_TBL" where "ATTR_IP"=?"ATTR_IP";"

#define SQL_INSERT_INFO			"insert into "USR_INFO_TBL" "\
								"("ATTR_NAME", "ATTR_GENDER", "ATTR_BIRTH", "ATTR_ADDRESS") values "\
								"(?"ATTR_NAME", ?"ATTR_GENDER", ?"ATTR_BIRTH", ?"ATTR_ADDRESS");"
#define SQL_SELECT_INFO_ALL		"select * from "USR_INFO_TBL";"
#define SQL_SELECT_INFO_BY_ID	"select * from "USR_INFO_TBL" where "ATTR_ID"=?"ATTR_ID";"
#define SQL_DELETE_INFO			"delete from "USR_INFO_TBL" where "ATTR_ID"=?"ATTR_ID";"

#define SQL_ALARM_CNT_USR		"select NUMTUPLES from __SYS__TABLES__ where TABLE_NAME="USR_INFO_TBL";"

typedef enum
{
	PSTMT_INSERT_IP = 0,
	PSTMT_SELECT_IP_ALL,
	PSTMT_SELECT_IP_BY_IP,
	PSTMT_DELETE_IP,

	PSTMT_INSERT_INFO,
	PSTMT_SELECT_INFO_ALL,
	PSTMT_SELECT_INFO_BY_ID,
	PSTMT_DELETE_INFO,

	PSTMT_ALARM_CNT_USR,
	PSTMT_MAX
} Pstmt_e;

typedef struct DB_s
{
	DAL_CONN	*ptDBConn;
	DAL_PSTMT	*patPstmt[PSTMT_MAX];
} DB_t;

int DB_Init( DAL_CONN **pptDBConn );
int DB_InitPreparedStatement( DB_t *ptDB );
void DB_ClosePreparedStatement( DB_t *ptDB );
void DB_Close( DB_t *ptDB );

#endif /* _RAS_DB_H_ */
