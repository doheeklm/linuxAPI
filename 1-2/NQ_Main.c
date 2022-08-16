/* NQ_Main.c */

//TODO 문자열 사이즈 체크
//TODO 에러처리+자원해제
//TODO 함수 헤더
//TODO 시그널 처리

#include "NQ_Inc.h"

#define TABLE_NAME	"EmployeeInfos"
#define ID			"id"
#define NAME		"name"
#define JOBTITLE	"jobTitle"
#define TEAM		"team"
#define PHONE		"phone"

void SigintHandler( int nSig );
void ClearStdin( char *c );

void Menu( DAL_CONN *ptConn );
void Insert( DAL_CONN *ptConn );
void Select( DAL_CONN *ptConn );
//void Update( DAL_CONN *ptConn );
//void Delete( DAL_CONN *ptConn );

/*=========================== Function Header ==============================
 * name  : main
 * desc  : Start point
 * return: CCBLK_rOk, or 
 * param : None
 * author: AUTHOR_NAME
=======================================================================*/
int main( void )
{
	int nRet = 0;
	DAL_CONN *ptConn;

	if ( signal( SIGINT, (void *)SigintHandler ) == SIG_ERR )
	{
		fprintf( stderr, "signal() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}	

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
	}

	return 0;
}

void SigintHandler( int nSig )
{
	printf( "%d EXIT\n", nSig );
}

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
}

void Menu( DAL_CONN *ptConn )
{
	char *nRet = NULL;
	char szPick[2];

	while ( 1 )
	{
		memset( szPick, 0, sizeof(szPick) );

		printf( "==========\n(1) 등록\n(2) 조회\n(3) 수정\n(4) 삭제\n(5) 종료\n==========\n입력: " );
		nRet = fgets( szPick, sizeof(szPick), stdin );
		if ( nRet == NULL )
		{
			fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return;
		}
		ClearStdin( szPick );

		switch ( atoi( szPick ) )
		{
			case 1:
			{
				Insert( ptConn );
			}
				break;
			case 2:
			{
				Select( ptConn );
			}
				break;
			case 3:
			{
				//Update( ptConn );
			}
				break;
			case 4:
			{
				//Delete( ptConn );
			}
				break;
			case 5:
			{
				return;
			}
				break;
			default:
				break;
		}
	}
}

void Insert( DAL_CONN *ptConn )
{
	char *nRet = NULL;
	
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
	nRet = fgets( szName, sizeof(szName), stdin );
	if ( nRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szName );

	printf( "사원 직급: " );
	nRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( nRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szJobTitle );

	printf( "소속팀: " );
	nRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( nRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szTeam );

	printf( "사원 전화번호: " );
	nRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( nRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szPhone );

	snprintf( szQuery, sizeof(szQuery), "insert into %s (%s, %s, %s, %s) values ('%s', '%s', '%s', '%s');", TABLE_NAME, NAME, JOBTITLE, TEAM, PHONE, szName, szJobTitle, szTeam, szPhone );
	szQuery[ strlen(szQuery) ] = '\0';

	if ( dalExecUpdate( ptConn, szQuery ) < 0 )
	{
		fprintf( stderr, "dalExecUpdate() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	return;
}

void Select( DAL_CONN *ptConn )
{
	printf("%p\n", ptConn );

	char *nRet = NULL;
	char szPick[2];
//	char szInput[32];
	char szQuery[256];
	
	DAL_RESULT_SET *ptResult; //DEBUG 초기화하면 에러?
//	DAL_ENTRY *ptEntry = NULL;
//	DAL_INT nId = 0;
//	DAL_STRING pszName = NULL;

//	memset( szInput, 0, sizeof(szInput) );
	memset( szQuery, 0, sizeof(szQuery) );
//	memset( ptEntry, 0, sizeof(struct DAL_ENTRY *) );

	snprintf( szQuery, sizeof(szQuery), "select * from %s;", TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	printf( "%s\n", szQuery );

	ptResult = dalExecQuery( ptConn, szQuery );
	if ( ptResult == NULL )
	{
		fprintf( stderr, "dalExecQuery() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}

	do
	{
		memset( szPick, 0, sizeof(szPick) );
		
		printf("==========\n(1) 전체 직원 조회\n(2) 특정 직원 조회\n==========\n입력: " );
		nRet = fgets( szPick, sizeof(szPick), stdin );
		if ( nRet == NULL )
		{
			fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return;
		}
		ClearStdin( szPick );

	} while ( atoi( szPick ) != 1 && atoi( szPick ) != 2 );
	
/*	switch ( atoi( szPick ) )
	{
		case 1:
			{
				for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
				{
					if ( dalGetIntByKey( ptEntry, ID, &nId ) == -1 )
					{
						fprintf( stderr, "dalGetIntByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						return;
					}

					if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						return;
					}

					printf( "ID: %d Name: %s\n", nId, pszName );
				}
			}
			break;
		case 2:
			{
				printf( "Input ID or Name: " );
				nRet = fgets( szInput, sizeof(szInput), stdin );
				if ( nRet == NULL )
				{
					fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return;
				}
				ClearStdin( szInput );

				printf( "Input ==> %s\n", szInput );
			}
			break;
		default:
			break;
	}

	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() error: errno[%d], errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}	
*/
	return;
}

/*
void Update( DAL_CONN *ptConn )
{
}

void Delete( DAL_CONN *ptConn )
{
}
*/
