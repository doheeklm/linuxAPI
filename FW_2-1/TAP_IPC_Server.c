/* TAP_IPC_Server.c */
//DB operation을 담당하는 프로세스
#include "TAP_IPC_Inc.h"

#define SERVER_PROCESS	"MIPCSVR"

#define INSERT			"Insert"
#define SELECT			"Select"
#define UPDATE			"Update"
#define DELETE			"Delete"

#define COMMA			","

#define TABLE_NAME		"EmployeeInfos"
#define ID				"id"
#define NAME			"name"
#define JOBTITLE		"jobTitle"
#define TEAM			"team"
#define PHONE			"phone"

#define RUN				100
#define STOP			99

typedef enum
{
	SUCCESS = 1, DAL_FAIL = -2,
	FGETS_FAIL = -3, TAP_FAIL = -4
} ReturnCode_t;

typedef struct INFO_s
{
	int nDB; //0, 1, 2, 3, 4
	int nId;
	char szName     [32];
	char szJobTitle [32];
	char szTeam     [32];
	char szPhone    [14];
} INFO_t;

INFO_t g_tInfo;

int g_nFlag = RUN;

DAL_CONN	*g_ptConn= NULL;
DAL_PSTMT	*g_ptPstmtInsert = NULL;
DAL_PSTMT	*g_ptPstmtSelectAll = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneById = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneByName = NULL;
DAL_PSTMT	*g_ptPstmtUpdate = NULL;
DAL_PSTMT	*g_ptPstmtDelete = NULL;

void SignalHandler( int nSig );
int InitPreparedStatement();
void DestroyPreparedStatement();

void Insert();

//TODO strlcpy

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRet = 0;
	iipc_ds_t tIpc;
	iipc_key_t tKey;
	iipc_msg_t tMsg;

	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = TAP_ipc_open( &tIpc, SERVER_PROCESS );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_open() errno[%d]\n", __func__, ipc_errno );
		
		nRet = dalDisconnect( g_ptConn );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		return TAP_FAIL;
	}

	tKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tKey )
	{
		fprintf( stderr, "%s TAP_ipc_getkey() errno[%d]\n", __func__, ipc_errno );
		goto end_of_function;
	}

	while ( RUN == g_nFlag )
	{
		memset( &tMsg, 0x00, sizeof(iipc_msg_t) );
		memset( &g_tInfo, 0x00, sizeof(INFO_t) );

		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgrcv() errno[%d]\n", __func__, ipc_errno );
			continue;
		}
		
		printf( "\n[SERVER] %s\n", tMsg.buf.msgq_buf );	 

		

		tMsg.u.h.dst = tMsg.u.h.src;
		tMsg.u.h.src = tKey;

		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgsnd() errno[%d]\n", __func__, ipc_errno );
			continue;
		}
	}

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_close() errno[%d]\n", __func__, ipc_errno );
		return -1;
	}
	
	nRet = dalDisconnect( g_ptConn );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return -1;
	}

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = STOP;

	printf( "\n[SignalHandler] %d\n", nSig );

	exit( -1 );
}

int InitPreparedStatement()
{
	char szQuery[256];
	
	//INSERT
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s);", TABLE_NAME, NAME, JOBTITLE, TEAM, PHONE, NAME, JOBTITLE, TEAM, PHONE );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtInsert = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtInsert )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s from %s;", ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectAll = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY ID ==> Select(), CheckId(), GetOriginalInfo()에서 사용
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneById = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneById )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ONE BY NAME
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneByName = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneByName )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;", TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtUpdate = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtDelete = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void DestroyPreparedStatement()
{
	int nRet = 0;

	nRet = dalDestroyPreparedStmt( g_ptPstmtInsert );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneById );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtSelectOneByName );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtUpdate );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	nRet = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	return;
}

void Insert()
{
	return;
}
