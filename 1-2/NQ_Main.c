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

void ClearStdin( char *c );
void Menu( DAL_CONN *ptConn );
void Insert( DAL_CONN *ptConn );
void Select( DAL_CONN *ptConn );
void Update( DAL_CONN *ptConn );
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
				Update( ptConn );
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
		return;
	}
	ClearStdin( szName );

	printf( "사원 직급: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szJobTitle );

	printf( "소속팀: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szTeam );

	printf( "사원 전화번호: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( pszRet == NULL )
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
	char *pszRet = NULL;
	char szPick[2];
	char szInput[32];
	char szQuery[256];
	
	DAL_RESULT_SET *ptResult; //NULL 하면 에러
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
		return;
	}

	do
	{
		memset( szPick, 0, sizeof(szPick) );
		
		printf("==========\n(1) 전체 직원 조회\n(2) 특정 직원 조회\n==========\n입력: " );
		pszRet = fgets( szPick, sizeof(szPick), stdin );
		if ( pszRet == NULL )
		{
			fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return;
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
						goto end_of_function;
					}

					if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto end_of_function;
					}

					printf( "ID: %d Name: %s\n", nId, pszName );
				}
			}
			break;
		case 2:
			{
				printf( "Input ID or Name: " );
				pszRet = fgets( szInput, sizeof(szInput), stdin );
				if ( pszRet == NULL )
				{
					fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto end_of_function;
				}
				ClearStdin( szInput );

				for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
				{
					if ( dalGetIntByKey( ptEntry, ID, &nId ) == -1 )
					{
						fprintf( stderr, "dalGetIntByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto end_of_function;
					}

					if ( nId == atoi( szInput ) )
					{
						if ( dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto end_of_function;							
						}
					
						if ( dalGetStringByKey( ptEntry, TEAM, &pszTeam ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto end_of_function;
						}

						if ( dalGetStringByKey( ptEntry, PHONE, &pszPhone ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto end_of_function;
						}

						printf( "[ID:%d] 사원 직급:%s | 소속팀:%s | 사원 전화번호:%s\n", atoi( szInput ), pszJobTitle, pszTeam, pszPhone );
	
						goto end_of_function;
					} 
					
					if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto end_of_function;
					}
					
					if ( strcmp( pszName, szInput ) == 0 )
					{
						if ( dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto end_of_function;							
						}
					
						if ( dalGetStringByKey( ptEntry, TEAM, &pszTeam ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto end_of_function;
						}

						if ( dalGetStringByKey( ptEntry, PHONE, &pszPhone ) == -1 )
						{
							fprintf( stderr, "dalGetStringByKey() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
							goto end_of_function;
						}

						printf( "[NAME:%s] 사원 직급:%s | 소속팀:%s | 사원 전화번호:%s\n", szInput, pszJobTitle, pszTeam, pszPhone );

						goto end_of_function;
					}
				}
			}
			break;
		default:
			break;
	}

end_of_function:
	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() error: errno[%d], errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}	
	
	return;
}

void Update( DAL_CONN *ptConn )
{
	int nComma = 0;

	char szTemp[32];
	memset( szTemp, 0, sizeof( szTemp ) );

	char *pszRet = NULL;
	char szId[8];
	char szQuery[256];
	char szUpdateJobTitle[32];
	char szUpdateTeam[32];
	char szUpdatePhone[32];

	memset( szId, 0, sizeof(szId) );
	memset( szQuery, 0, sizeof(szQuery) );
	memset( szUpdateJobTitle, 0, sizeof(szUpdateJobTitle) );
	memset( szUpdateTeam, 0, sizeof(szUpdateTeam) );
	memset( szUpdatePhone, 0, sizeof(szUpdatePhone) );
	
	printf( "Input ID to update: " );
	pszRet = fgets( szId, sizeof(szId), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}
	ClearStdin( szId );

	//TODO Change to strncat
	snprintf( szTemp, sizeof(szTemp), "update %s set ", TABLE_NAME );
	strcat( szQuery, szTemp );

	printf( "Update JobTitle: " );
	pszRet = fgets( szUpdateJobTitle, sizeof(szUpdateJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}
	ClearStdin( szUpdateJobTitle );

	if ( strlen( szUpdateJobTitle ) != 0 )
	{
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", JOBTITLE, szUpdateJobTitle );
		strcat( szQuery, szTemp );
		nComma = 1;
	}
	
	printf( "Update Team: " );
	pszRet = fgets( szUpdateTeam, sizeof(szUpdateTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}
	ClearStdin( szUpdateTeam );

	if ( strlen( szUpdateTeam ) != 0 )
	{
		if ( nComma == 1 )
		{
			strcat( szQuery, ", " );
		}
		
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TEAM, szUpdateTeam );
		strcat( szQuery, szTemp );
		nComma = 1;
	}
	
	printf( "Update Phone: " );
	pszRet = fgets( szUpdatePhone, sizeof(szUpdatePhone), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}
	ClearStdin( szUpdatePhone );
	
	if ( strlen( szUpdatePhone ) != 0 )
	{
		if ( nComma == 1 )
		{
			strcat( szQuery, ", " );	
		}
	
		snprintf( szTemp, sizeof(szTemp), "%s = '%s'", PHONE, szUpdatePhone );
		strcat( szQuery, szTemp );
	}

	snprintf( szTemp, sizeof(szTemp), " where %s = %d;", ID, atoi( szId ) );
	strcat( szQuery, szTemp );
	szQuery[ strlen(szQuery) ] = '\0';

	if ( dalExecUpdate( ptConn, szQuery ) == -1 )
	{
		fprintf( stderr, "dalExecUpdate() error: errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}

	return;
}

//void Delete( DAL_CONN *ptConn )
//{

//}
