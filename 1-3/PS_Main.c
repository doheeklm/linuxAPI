/* PS_Main.c */
#include "PS_Inc.h"

#define TABLE_NAME	"EmployeeInfos"
#define ID			"id"
#define NAME		"name"
#define JOBTITLE	"jobTitle"
#define TEAM		"team"
#define PHONE		"phone"
//#define SIZE		32

typedef enum
{
	INSERT_ERROR = -1, INSERT_SUCCESS = 0,
	SELECT_ERROR = -1, SELECT_SUCCESS = 0,
	UPDATE_ERROR = -1, UPDATE_SUCCESS = 0,
	DELETE_ERROR = -1, DELETE_SUCCESS = 0
} ReturnCode_t;
/*
typedef struct EMPLOYEE_s
{
	char szName[SIZE + 1];
	char sz
} EMPLOYEE_t;
*/
void ClearStdin( char *c );
void Menu( DAL_CONN *ptConn );
int Insert( DAL_CONN *ptConn );
int Select( DAL_CONN *ptConn );
int Update( DAL_CONN *ptConn );
int Delete( DAL_CONN *ptConn );

/*=========================== Function Header ==============================
 * name  : main
 * desc  : Start point
 * return: 0
 * param : None
 * author: AUTHOR_NAME
=======================================================================*/
int main( void )
{
	DAL_CONN *ptConn;

	ptConn = dalConnect( NULL );
	if ( ptConn == NULL )
	{
		fprintf( stderr, "dalConnect() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	Menu( ptConn );

	if ( dalDisconnect( ptConn ) == -1 )
	{
		fprintf( stderr, "dalDisconnect() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	return 0;
}

/*=========================== Function Header ==============================
 * name  : ClearStdin
 * desc  : Clear stdin buffer
 * return: None
 * param : c
 * author: AUTHOR_NAME
=======================================================================*/
void ClearStdin( char *c )
{
	if ( c == NULL )
	{
		return;
	}

	if ( c[ strlen(c) - 1 ] == '\n' )
	{
		c[ strlen(c) - 1 ] = '\0';
	}

	__fpurge( stdin );
	return;
}

/*=========================== Function Header ==============================
 * name  : Menu
 * desc  : Select menu 
 * return: None
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
void Menu( DAL_CONN *ptConn )
{
	if ( ptConn == NULL )
	{
		return;
	}

	char *pszRet = NULL;
	char szPick[2];

	while ( 1 )
	{
		memset( szPick, 0x00, sizeof(szPick) );

		printf( "=================\n(1) Insert Info\n(2) Select Info\n(3) Update Info\n(4) Delete Info\n(5) Exit program\n=================\nInput: " );
		pszRet = fgets( szPick, sizeof(szPick), stdin );
		if ( pszRet == NULL )
		{
			fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return;
		}
		ClearStdin( szPick );

		switch ( atoi( szPick ) )
		{
			case 1:
			{
				if ( INSERT_ERROR == Insert( ptConn ) )
				{
					return;	
				}
			}
				break;
			case 2:
			{
				if ( SELECT_ERROR == Select( ptConn ) )
				{
					return;
				}
			}
				break;
			case 3:
			{
				if ( UPDATE_ERROR == Update( ptConn ) )
				{
					return;
				}
			}
				break;
			case 4:
			{
				if ( DELETE_ERROR == Delete( ptConn ) )
				{
					return;
				}
			}
				break;
			case 5:
			{
				printf( "Exiting program...\n" );
				return;
			}
				break;
			default:
			{
				printf( "Wrong choice...Back to Menu\n" );
				continue;
			}
				break;
		}
	}

	return;
}

/*=========================== Function Header ==============================
 * name  : Insert
 * desc  : Insert info
 * return: INSERT_SUCCESS, or INSERT_ERROR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Insert( DAL_CONN *ptConn )
{
	if ( ptConn == NULL )
	{
		return INSERT_ERROR;
	}

	int nRet = 0;
	int nId = 1;
	char szName[32];
	char szJobTitle[32];
	char szTeam[32];
	char szPhone[14];
	char szTemp[32];
	char szQuery[256];
	char *pszRet = NULL;
	
	DAL_PSTMT *ptPstmt = NULL;
	DAL_STRING pszQuery = NULL;

	memset( szName, 0x00, sizeof(szName) );
	memset( szJobTitle, 0x00, sizeof(szJobTitle) );
	memset( szTeam, 0x00, sizeof(szTeam) );
	memset( szPhone, 0x00, sizeof(szPhone) );
	memset( szTemp, 0x00, sizeof(szTemp) );
	memset( szQuery, 0x00, sizeof(szQuery) );

	printf( "Name: " );
	pszRet = fgets( szName, sizeof(szName), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERROR;
	}
	ClearStdin( szName );

	printf( "Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERROR;
	}
	ClearStdin( szJobTitle );

	printf( "Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERROR;
	}
	ClearStdin( szTeam );

	printf( "Phone(xxx-xxxx-xxxx): " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERROR;
	}
	ClearStdin( szPhone );

	snprintf( szQuery, sizeof(szQuery), "insert into %s values (?%s, ?%s, ?%s, ?%s, ?%s)", TABLE_NAME, ID, NAME, JOBTITLE, TEAM, PHONE );
	szQuery[ strlen(szQuery) ] = '\0';

	/*
	 *	Prepared Statement
	 */
	ptPstmt = dalPreparedStatement( ptConn, szQuery );
	if ( ptPstmt == NULL )
	{
		fprintf( stderr, "dalPreparedStatement() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERROR;
	}

	/*
	 *	Set Value By Key
	 */
	//TODO 아이디는 랜덤?
	if ( dalSetValueByKey( ptPstmt, ID, DAL_ATTR_INT, &nId ) == -1 )
	{
		fprintf( stderr, "dalSetValueByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}
	nId++;

	if ( dalSetValueByKey( ptPstmt, NAME, DAL_ATTR_STRING, szName ) == -1 )
	{
		fprintf( stderr, "dalSetValueByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}

	if ( dalSetValueByKey( ptPstmt, JOBTITLE, DAL_ATTR_STRING, szJobTitle ) == -1 )
	{
		fprintf( stderr, "dalSetValueByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}

	if ( dalSetValueByKey( ptPstmt, TEAM, DAL_ATTR_STRING, szTeam ) == -1 )
	{
		fprintf( stderr, "dalSetValueByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}

	if ( dalSetValueByKey( ptPstmt, PHONE, DAL_ATTR_STRING, szPhone ) == -1 )
	{
		fprintf( stderr, "dalSetValueByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}

	pszQuery = dalPreparedGetQuery( ptPstmt );
	if ( pszQuery == NULL )
	{
		fprintf( stderr, "dalPreparedGetQuery() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}
	
	printf( "pszQuery = %s\n", pszQuery );

	nRet = dalPreparedExec( ptConn, ptPstmt, NULL );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;
	}
	else if ( nRet == 0 )
	{
		printf( "nRet = %d\n", nRet );
		goto error_return;
	}

	if ( dalDestroyPreparedStmt( ptPstmt ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}
	printf( "{Insert Success} Back to Menu\n" );
	return INSERT_SUCCESS;

error_return:
	if ( dalDestroyPreparedStmt( ptPstmt ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}
	printf( "{Insert Error} Back to Menu\n" );
	return INSERT_ERROR;
}

/*=========================== Function Header ==============================
 * name  : Select
 * desc  : Select info
 * return: SELECT_SUCCESS, or SELECT_ERROR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Select( DAL_CONN *ptConn )
{
	if ( ptConn == NULL )
	{
		return SELECT_ERROR;
	}

	int nRet = 0;
	char *pszRet = NULL;
	char szPick[2];
	char szInput[32];
	char szQuery[256];
	
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;
	DAL_PSTMT *ptPstmt = NULL;

	DAL_INT nId = 0;
	DAL_STRING pszName = NULL;
	
	memset( szInput, 0x00, sizeof(szInput) );
	memset( szQuery, 0x00, sizeof(szQuery) );

	do
	{
		memset( szPick, 0, sizeof(szPick) );
		
		printf("=================\n(1) Select All\n(2) Select One\n=================\nInput: " );
		pszRet = fgets( szPick, sizeof(szPick), stdin );
		if ( pszRet == NULL )
		{
			fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return SELECT_ERROR;
		}
		ClearStdin( szPick );

	} while ( atoi( szPick ) != 1 && atoi( szPick ) != 2 );
	
	switch ( atoi( szPick ) )
	{
		case 1:
			{
				// select id, name from EmployeeInfos;						
			}
			break;
		case 2:
			{
				//TODO 조회 안되는 ID 처리 0
				printf( "Input ID or Name: " );
				pszRet = fgets( szInput, sizeof(szInput), stdin );
				if ( pszRet == NULL )
				{
					fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return SELECT_ERROR;
				}
				ClearStdin( szInput );

				snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID );
				szQuery[ strlen(szQuery) ] = '\0';

				ptPstmt = dalPreparedStatement( ptConn, szQuery );
				if ( ptPstmt == NULL )
				{
					fprintf( stderr, "dalPreparedStatement() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return SELECT_ERROR;
				}
				
				nRet = dalPreparedExec( ptConn, ptPstmt, &ptResult );
				if ( nRet == -1 )
				{
					fprintf( stderr, "dalPreparedExec() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;
				}
				else if ( nRet == 0 )
				{
					printf( "nRet = %d\n", nRet );
				}
			}
			break;
		default:
		{
			break;
		}
			break;
	}

	if ( dalDestroyPreparedStmt( ptPstmt ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return SELECT_ERROR;
	}

	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return SELECT_ERROR;
	}	

	printf( "{Select Success} Back to Menu\n" );
	return SELECT_SUCCESS;

error_return:
	if ( dalDestroyPreparedStmt( ptPstmt ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return SELECT_ERROR;
	}

	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() error: errno[%d], errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	printf( "{Select Fail} End program\n" );
	return SELECT_ERROR;
}

/*=========================== Function Header ==============================
 * name  : Update
 * desc  : Update info
 * return: UPDATE_SUCCESS, or UPDATE_ERROR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Update( DAL_CONN *ptConn )
{
	if ( ptConn == NULL )
	{
		return UPDATE_ERROR;
	}

	int nComma = 0;
	char szTemp[32];
	char *pszRet = NULL;
	char szId[8];
	char szQuery[256];
	char szUpdateJobTitle[32];
	char szUpdateTeam[32];
	char szUpdatePhone[32];

	memset( szTemp, 0x00, sizeof(szTemp) );
	memset( szId, 0x00, sizeof(szId) );
	memset( szQuery, 0x00, sizeof(szQuery) );
	memset( szUpdateJobTitle, 0x00, sizeof(szUpdateJobTitle) );
	memset( szUpdateTeam, 0x00, sizeof(szUpdateTeam) );
	memset( szUpdatePhone, 0x00, sizeof(szUpdatePhone) );
	
	printf( "정보 수정할 사원의 ID를 입력해주세요.: " );
	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERROR;
	}
	ClearStdin( szId );

	snprintf( szTemp, sizeof(szTemp), "update %s set ", TABLE_NAME );
	strncat( szQuery, szTemp, strlen(szTemp) );

	printf( "(수정) 사원 직급: " );
	pszRet = fgets( szUpdateJobTitle, sizeof(szUpdateJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERROR;
	}
	ClearStdin( szUpdateJobTitle );

	if ( strlen( szUpdateJobTitle ) != 0 )
	{
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", JOBTITLE, szUpdateJobTitle );
		strncat( szQuery, szTemp, strlen(szTemp) );
		nComma = 1;
	}
	
	printf( "(수정) 소속팀: " );
	pszRet = fgets( szUpdateTeam, sizeof(szUpdateTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERROR;
	}
	ClearStdin( szUpdateTeam );

	if ( strlen( szUpdateTeam ) != 0 )
	{
		if ( nComma == 1 )
		{
			strcat( szQuery, ", " );
		}
		
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TEAM, szUpdateTeam );
		strncat( szQuery, szTemp, strlen(szTemp) );
		nComma = 1;
	}
	
	printf( "(수정) 사원 전화번호: " );
	pszRet = fgets( szUpdatePhone, sizeof(szUpdatePhone), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERROR;
	}
	ClearStdin( szUpdatePhone );
	
	if ( strlen( szUpdatePhone ) != 0 )
	{
		if ( nComma == 1 )
		{
			strcat( szQuery, ", " );	
		}
	
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", PHONE, szUpdatePhone );
		strncat( szQuery, szTemp, strlen(szTemp) );
	}

	snprintf( szTemp, sizeof(szTemp), " where %s = %d;", ID, atoi( szId ) );
	strncat( szQuery, szTemp, strlen(szTemp) );
	szQuery[ strlen(szQuery) ] = '\0';

	if ( dalExecUpdate( ptConn, szQuery ) == -1 )
	{
		fprintf( stderr, "dalExecUpdate() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERROR;
	}

	printf( "{Update Success} Back to Menu\n" );
	return UPDATE_SUCCESS;
}

/*=========================== Function Header ==============================
 * name  : Delete
 * desc  : Delete info
 * return: DELETE_SUCCESS, or DELETE_ERROR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Delete( DAL_CONN *ptConn )
{
	if ( ptConn == NULL )
	{
		return DELETE_ERROR;
	}
	
	char *pszRet = NULL;
	char szId[8];
	char szQuery[256];

	memset( szId, 0x00, sizeof(szId) );
	memset( szQuery, 0x00, sizeof(szQuery) );

	//TODO 존재하는지 확인	
	printf( "정보 삭제할 사원의 ID를 입력해주세요.: " );
	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_ERROR;
	}
	ClearStdin( szId );

	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = %s;", TABLE_NAME, ID, szId );
	szQuery[ strlen(szQuery) ] = '\0';
	
	if ( dalExecUpdate( ptConn, szQuery ) == -1 )
	{
		fprintf( stderr, "dalExecUpdate() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_ERROR;
	}
	
	printf( "{Delete Success} Back to Menu\n" );
	return DELETE_SUCCESS;
}


