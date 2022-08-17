/* NQ_Main.c */
#include "NQ_Inc.h"

#define TABLE_NAME	"EmployeeInfos"
#define ID			"id"
#define NAME		"name"
#define JOBTITLE	"jobTitle"
#define TEAM		"team"
#define PHONE		"phone"

typedef enum
{
	INSERT_ERR = -1, INSERT_SUC = 0,
	SELECT_ERR = -1, SELECT_SUC = 0,
	UPDATE_ERR = -1, UPDATE_SUC = 0,
	DELETE_ERR = -1, DELETE_SUC = 0
} ReturnCode_t;

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
	int nRet = 0;
	DAL_CONN *ptConn;

	ptConn = dalConnect( NULL );
	if ( ptConn == NULL )
	{
		fprintf( stderr, "dalConnect() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	nRet = dalIsConnected( ptConn );
	if ( nRet == 0 )
	{
		fprintf( stderr, "DB Connection is dead.\n" );
		exit( - 1 );
	}

	Menu( ptConn );

	if ( dalDisconnect( ptConn ) < 0 )
	{
		fprintf( stderr, "dalDisconnect() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	nRet = dalIsDisconnected( ptConn );
	if ( nRet == 1 )
	{
		fprintf( stderr, "DB Connection is dead.\n" );
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
	char *pszRet = NULL;
	char szPick[2];

	while ( 1 )
	{
		memset( szPick, 0, sizeof(szPick) );

		printf( "==========\n(1) 등록\n(2) 조회\n(3) 수정\n(4) 삭제\n(5) 종료\n==========\n입력: " );
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
				if ( INSERT_ERR == Insert( ptConn ) )
				{
					return;	
				}
			}
				break;
			case 2:
			{
				if ( SELECT_ERR == Select( ptConn ) )
				{
					return;
				}
			}
				break;
			case 3:
			{
				if ( UPDATE_ERR == Update( ptConn ) )
				{
					return;
				}
			}
				break;
			case 4:
			{
				if ( DELETE_ERR == Delete( ptConn ) )
				{
					return;
				}
			}
				break;
			case 5:
			{
				return;
			}
				break;
			default:
			{
				printf( "잘 못 입력하여 메뉴로 돌아갑니다.\n" );
				continue;
			}
				break;
		}
	}
}

/*=========================== Function Header ==============================
 * name  : Insert
 * desc  : Insert info
 * return: INSERT_SUC, or INSERT_ERR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Insert( DAL_CONN *ptConn )
{
	char *pszRet = NULL;
	char szName[32];
	char szJobTitle[32];
	char szTeam[32];
	char szPhone[14];
	char szQuery[256];

	memset( szName, 0, sizeof(szName) );
	memset( szJobTitle, 0, sizeof(szJobTitle) );
	memset( szTeam, 0, sizeof(szTeam) );
	memset( szPhone, 0, sizeof(szPhone) );
	memset( szQuery, 0, sizeof(szQuery) );
	
	printf( "사원 이름: " );
	pszRet = fgets( szName, sizeof(szName), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERR;
	}
	ClearStdin( szName );

	printf( "사원 직급: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERR;
	}
	ClearStdin( szJobTitle );

	printf( "소속팀: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERR;
	}
	ClearStdin( szTeam );

	printf( "사원 전화번호: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERR;
	}
	ClearStdin( szPhone );

	snprintf( szQuery, sizeof(szQuery), "insert into %s (%s, %s, %s, %s) values ('%s', '%s', '%s', '%s');", TABLE_NAME, NAME, JOBTITLE, TEAM, PHONE, szName, szJobTitle, szTeam, szPhone );
	szQuery[ strlen(szQuery) ] = '\0';

	if ( dalExecUpdate( ptConn, szQuery ) == -1 )
	{
		fprintf( stderr, "dalExecUpdate() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_ERR;
	}

	printf( "[ 등록 완료 ] 메뉴로 돌아갑니다.\n" );
	return INSERT_SUC;
}
/*=========================== Function Header ==============================
 * name  : Select
 * desc  : Select info
 * return: SELECT_SUC, or SELECT_ERR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Select( DAL_CONN *ptConn )
{
	char *pszRet = NULL;
	char szPick[2];
	char szInput[32];
	char szQuery[256];
	
	DAL_RESULT_SET *ptResult; //NULL로 초기화 하면 에러
	DAL_ENTRY *ptEntry;
	
	DAL_INT nId = 0;
	DAL_STRING pszName = NULL;
	DAL_STRING pszJobTitle = NULL;
	DAL_STRING pszTeam = NULL;
	DAL_STRING pszPhone = NULL;

	memset( szInput, 0, sizeof(szInput) );
	memset( szQuery, 0, sizeof(szQuery) );

	snprintf( szQuery, sizeof(szQuery), "select * from %s;", TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	ptResult = dalExecQuery( ptConn, szQuery );
	if ( ptResult == NULL )
	{
		fprintf( stderr, "dalExecQuery() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return SELECT_ERR;
	}

	do
	{
		memset( szPick, 0, sizeof(szPick) );
		
		printf("==========\n(1) 전체 직원 조회\n(2) 특정 직원 조회\n==========\n입력: " );
		pszRet = fgets( szPick, sizeof(szPick), stdin );
		if ( pszRet == NULL )
		{ fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			goto error_return;
		}
		ClearStdin( szPick );

	} while ( atoi( szPick ) != 1 && atoi( szPick ) != 2 );
	
	switch ( atoi( szPick ) )
	{
		case 1:
			{
				for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
				{
					if ( dalGetIntByKey( ptEntry, ID, &nId ) == -1 )
					{
						fprintf( stderr, "dalGetIntByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}

					if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}

					printf( "ID: %d Name: %s\n", nId, pszName );
				}
			}
			break;
		case 2:
			{
				printf( "정보 조회할 사원의 ID 또는 이름을 입력해주세요.: " );
				pszRet = fgets( szInput, sizeof(szInput), stdin );
				if ( pszRet == NULL )
				{
					fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;
				}
				ClearStdin( szInput );

				for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
				{
					if ( dalGetIntByKey( ptEntry, ID, &nId ) == -1 )
					{
						fprintf( stderr, "dalGetIntByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}

					if ( nId == atoi( szInput ) )
					{
						if ( dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto error_return;
						}
					
						if ( dalGetStringByKey( ptEntry, TEAM, &pszTeam ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto error_return;
						}

						if ( dalGetStringByKey( ptEntry, PHONE, &pszPhone ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto error_return;
						}

						printf( "[ID:%d] 사원 직급:%s | 소속팀:%s | 사원 전화번호:%s\n", atoi( szInput ), pszJobTitle, pszTeam, pszPhone );
						goto end_of_function;
					} 
					
					if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}
					
					if ( strcmp( pszName, szInput ) == 0 )
					{
						if ( dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto error_return;
						}
					
						if ( dalGetStringByKey( ptEntry, TEAM, &pszTeam ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto error_return;
						}

						if ( dalGetStringByKey( ptEntry, PHONE, &pszPhone ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto error_return;
						}

						printf( "[NAME:%s] 사원 직급:%s | 소속팀:%s | 사원 전화번호:%s\n", szInput, pszJobTitle, pszTeam, pszPhone );
					}
				}
			}
			break;
		default:
		{
			goto error_return;
		}
			break;
	}

end_of_function:
	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() error: errno[%d], errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}	
	printf( "[ 조회 완료 ] 메뉴로 돌아갑니다.\n" );
	return SELECT_SUC;

error_return:
	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() error: errno[%d], errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}	
	printf( "[ 조회 실패 ] 프로그램을 종료합니다.\n" );
	return SELECT_ERR;
}
/*=========================== Function Header ==============================
 * name  : Update
 * desc  : Update info
 * return: UPDATE_SUC, or UPDATE_ERR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Update( DAL_CONN *ptConn )
{
	int nComma = 0;

	char szTemp[32];
	char *pszRet = NULL;
	char szId[8];
	char szQuery[256];
	char szUpdateJobTitle[32];
	char szUpdateTeam[32];
	char szUpdatePhone[32];

	memset( szTemp, 0, sizeof(szTemp) );
	memset( szId, 0, sizeof(szId) );
	memset( szQuery, 0, sizeof(szQuery) );
	memset( szUpdateJobTitle, 0, sizeof(szUpdateJobTitle) );
	memset( szUpdateTeam, 0, sizeof(szUpdateTeam) );
	memset( szUpdatePhone, 0, sizeof(szUpdatePhone) );
	
	printf( "정보 수정할 사원의 ID를 입력해주세요.: " );
	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERR;
	}
	ClearStdin( szId );

	snprintf( szTemp, sizeof(szTemp), "update %s set ", TABLE_NAME );
	strncat( szQuery, szTemp, strlen(szTemp) );

	printf( "(수정) 사원 직급: " );
	pszRet = fgets( szUpdateJobTitle, sizeof(szUpdateJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_ERR;
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
		return UPDATE_ERR;
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
		return UPDATE_ERR;
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
		return UPDATE_ERR;
	}

	printf( "[ 수정 완료 ] 메뉴로 돌아갑니다.\n" );
	return UPDATE_SUC;
}
/*=========================== Function Header ==============================
 * name  : Delete
 * desc  : Delete info
 * return: DELETE_SUC, or DELETE_ERR
 * param : ptConn
 * author: AUTHOR_NAME
=======================================================================*/
int Delete( DAL_CONN *ptConn )
{
	char *pszRet = NULL;
	char szId[8];
	char szQuery[256];

	memset( szId, 0, sizeof(szId) );
	memset( szQuery, 0, sizeof(szQuery) );
	
	printf( "정보 삭제할 사원의 ID를 입력해주세요.: " );
	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_ERR;
	}
	ClearStdin( szId );

	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = %s;", TABLE_NAME, ID, szId );
	szQuery[ strlen(szQuery) ] = '\0';
	
	if ( dalExecUpdate( ptConn, szQuery ) == -1 )
	{
		fprintf( stderr, "dalExecUpdate() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_ERR;
	}
	
	printf( "[ 삭제 완료 ] 메뉴로 돌아갑니다.\n" );
	return DELETE_SUC;
}
