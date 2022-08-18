/* PS_Main.c */
#include "PS_Inc.h"

#define TABLE_NAME	"EmployeeInfos"
#define ID			"id"
#define NAME		"name"
#define JOBTITLE	"jobTitle"
#define TEAM		"team"
#define PHONE		"phone"

typedef enum
{
	INSERT_FAIL = -1, INSERT_SUCCESS = 1, INSERT_ZERO = 0,
	SELECT_FAIL = -1, SELECT_SUCCESS = 1, SELECT_ZERO = 0,
	UPDATE_FAIL = -1, UPDATE_SUCCESS = 1, UPDATE_ZERO = 0,
	DELETE_FAIL = -1, DELETE_SUCCESS = 1, DELETE_ZERO = 0,
	CHECKID_FAIL = -1, CHECKID_VALID = 1, CHECKID_INVALID = 0
} ReturnCode_t;

void SignalHandler( int nSigno );
void Menu( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt );
void ClearStdin( char *pszTemp );
void InitPreparedStatement( DAL_CONN *ptConn, DAL_PSTMT **ptPstmt, char* pszQuery, int nSizeQuery, int nPickMenu );

int Insert( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt );
//int Select( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt );
int Update( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt );
int Delete( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt );
int CheckId( DAL_CONN *ptConn, int nId );

DAL_CONN *ptConn = NULL;
DAL_PSTMT *ptPstmt = NULL;

void SignalHandler( int nSigno )
{
	printf( "[SignalHandler] %d\n", nSigno );

	if ( ptConn != NULL )
	{
		ptConn = NULL;
	}

	if ( ptPstmt != NULL )
	{
		ptPstmt = NULL;
	}

	if ( dalDestroyPreparedStmt( ptPstmt ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDisconnect( ptConn ) == -1 )
	{
		fprintf( stderr, "dalDisconnect() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	exit( -1 );
}

int main( void )
{
	tb_signal( SIGINT, SignalHandler );
	tb_signal( SIGQUIT, SignalHandler );
	tb_signal( SIGTERM, SignalHandler );
	tb_signal( SIGHUP, SignalHandler );

	ptConn = dalConnect( NULL );
	if ( NULL == ptConn )
	{
		fprintf( stderr, "dalConnect() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	Menu( ptConn, ptPstmt );

	if ( dalDestroyPreparedStmt( ptPstmt ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDisconnect( ptConn ) == -1 )
	{
		fprintf( stderr, "dalDisconnect() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	return 0;
}

void ClearStdin( char *pszTemp )
{
	if ( NULL == pszTemp )
	{
		return;
	}

	if ( pszTemp[ strlen(pszTemp) - 1 ] == '\n' )
	{
		pszTemp[ strlen(pszTemp) - 1 ] = '\0';
	}

	__fpurge( stdin );

	return;
}

void Menu( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt )
{
	if ( NULL == ptConn )
	{
		printf( "ptConn NULL\n" );
		return;
	}

	int nPickMenu = 0;
	char *pszRet = NULL;
	char szPickMenu[8];
	char szQuery[256];

	while ( 1 )
	{
		nPickMenu = 0;
		memset( szPickMenu, 0x00, sizeof(szPickMenu) );
		memset( szQuery, 0x00, sizeof(szQuery) );

		printf( "=================\n(1) Insert Info\n(2) Select Info\n(3) Update Info\n(4) Delete Info\n(5) Exit program\n=================\nInput: " );
	
		pszRet = fgets( szPickMenu, sizeof(szPickMenu), stdin );
		if ( pszRet == NULL )
		{
			fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return;
		}
		ClearStdin( szPickMenu );

		nPickMenu = atoi( szPickMenu );

		if ( nPickMenu >= 1 && nPickMenu <= 4 )
		{
			InitPreparedStatement( ptConn, &ptPstmt, szQuery, sizeof(szQuery), atoi( szPickMenu ) );
		}
		else if ( nPickMenu == 5 )
		{
			printf( "[Menu] Exit Program\n" );
			break;
		}
		else
		{
			printf( "[Menu] Back to Menu\n" );
			continue;
		}

		printf( "[Menu] szQuery: %s\n", szQuery );

		switch ( nPickMenu )
		{
			case 1:
			{
				if ( INSERT_FAIL == Insert( ptConn, ptPstmt ) )
				{
					return;
				}
			}
				break;
			case 2:
			{
			//	if ( SELECT_FAIL == Select( ptConn, ptPstmt ) )
			//	{
			//		return;
			//	}
			}
				break;
			case 3:
			{
				if ( UPDATE_FAIL == Update( ptConn, ptPstmt ) )
				{
					return;
				}
			}
				break;
			case 4:
			{
				if ( DELETE_FAIL == Delete( ptConn, ptPstmt ) )
				{
					return;
				}
			}
				break;
			default:
				break;
		}
	}

	return;
}

void InitPreparedStatement( DAL_CONN *ptConn, DAL_PSTMT **ptPstmt, char* pszQuery, int nSizeQuery, int nPickMenu )
{
	char *pszRet = NULL;
	char szPickSelect[2];
	char szInput[32];

	switch ( nPickMenu )
	{
		case 1: //INSERT
		{
			snprintf( pszQuery, nSizeQuery, "insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s)",
					TABLE_NAME, NAME, JOBTITLE, TEAM, PHONE, NAME, JOBTITLE, TEAM, PHONE );
			pszQuery[ strlen(pszQuery) ] = '\0';	
		}
			break;
		case 2: //SELECT
		{
			do
			{
				memset( szPickSelect, 0x00, sizeof(szPickSelect) );	
				printf("=================\n(1) Select All\n(2) Select One\n=================\nInput: " );
				pszRet = fgets( szPickSelect, sizeof(szPickSelect), stdin );
				if ( pszRet == NULL )
				{
					fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return;
				}
				ClearStdin( szPickSelect );
			} while ( atoi( szPickSelect ) != 1 && atoi( szPickSelect ) != 2 );
		
			switch ( atoi( szPickSelect ) )
			{
				case 1: //SELECT ALL
				{
					snprintf( pszQuery, nSizeQuery, "select %s, %s from %s;", ID, NAME, TABLE_NAME );
					pszQuery[ strlen(pszQuery) ] = '\0';
				}
					break;
				case 2: //SELECT ONE
				{
					memset( szInput, 0x00, sizeof(szInput) );

					printf( "Input ID or Name: " );
					pszRet = fgets( szInput, sizeof(szInput), stdin );
					if ( pszRet == NULL )
					{
						fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						return;
					}
					ClearStdin( szInput );

					if ( atoi( szInput )  > 0 )
					{
						snprintf( pszQuery, nSizeQuery,	"select %s, %s, %s from %s where %s = ?%s;",
								JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID ); 
						pszQuery[ strlen(pszQuery) ] = '\0';
					}
					else if ( atoi( szInput ) == 0 )
					{
						snprintf( pszQuery, nSizeQuery,	"select %s, %s, %s from %s where %s = ?%s;",
								JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME ); 
						pszQuery[ strlen(pszQuery) ] = '\0';
					}
				}
					break;
				default:
					break;
			}
		}
			break;
		case 3: //UPDATE
		{
			snprintf( pszQuery, nSizeQuery,	"update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;",
						TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
			pszQuery[ strlen(pszQuery) ] = '\0';
		}
			break;
		case 4: //DELETE
		{
			snprintf( pszQuery, nSizeQuery,	"delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
			pszQuery[ strlen(pszQuery) ] = '\0';
		}
			break;
		default:
			break;
	}

	*ptPstmt = dalPreparedStatement( ptConn, pszQuery );
	if ( NULL == *ptPstmt )
	{
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return;
	}

	return;
}

int Insert( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt )
{
	if ( NULL == ptConn )
	{	
		printf( "[Insert] ptConn NULL\n" );
		return INSERT_FAIL;
	}

	if ( NULL == ptPstmt )
	{
		printf( "[Insert] ptPstmt NULL\n" );
		return INSERT_FAIL;
	}

	int nRet = 0;
	char *pszRet = NULL;

	char szName		[32];
	char szJobTitle	[32];
	char szTeam		[32];
	char szPhone	[14];
	char szTemp		[32];
	char szQuery	[256];
	
	memset( szName,		0x00, sizeof(szName) );
	memset( szJobTitle,	0x00, sizeof(szJobTitle) );
	memset( szTeam,		0x00, sizeof(szTeam) );
	memset( szPhone,	0x00, sizeof(szPhone) );
	memset( szTemp,		0x00, sizeof(szTemp) );
	memset( szQuery,	0x00, sizeof(szQuery) );

	printf( "Name: " );
	pszRet = fgets( szName, sizeof(szName), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szName );

	printf( "Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szJobTitle );

	printf( "Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szTeam );

	printf( "Phone: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szPhone );

	////////////////
	// Set By Key //
	////////////////
	if ( dalSetStringByKey( ptPstmt, NAME, szName ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	if ( dalSetStringByKey( ptPstmt, JOBTITLE, szJobTitle ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	if ( dalSetStringByKey( ptPstmt, TEAM, szTeam ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	if ( dalSetStringByKey( ptPstmt, PHONE, szPhone ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	
	nRet = dalPreparedExec( ptConn, ptPstmt, NULL );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	else if ( nRet == 0 )
	{
		printf( "[Insert] %d Tuple\n", nRet );
		return INSERT_ZERO;
	}

	printf( "[Insert] Back to Menu\n" );
	return INSERT_SUCCESS;
}
/*
int Select( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt )
{
	if ( NULL == ptConn )
	{	
		printf( "[Select] ptConn NULL\n" );
		return SELECT_FAIL;
	}

	if ( NULL == ptPstmt )
	{
		printf( "[Select] ptPstmt NULL\n" );
		return SELECT_FAIL;
	}

	int nRet = 0;
	char *pszRet = NULL;
	
	char szInput[32];
	char szQuery[256];
	
	int nId = 0;
	char* pszName = NULL;
	char* pszJobTitle = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	memset( szInput, 0x00, sizeof(szInput) );
	memset( szQuery, 0x00, sizeof(szQuery) );

	switch ( atoi( szPick ) )
	{
		case 1:
			{
				ptResult = dalExecQuery( ptConn, szQuery );
				if ( ptResult == NULL )
				{
					fprintf( stderr, "dalExecQuery() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return SELECT_FAIL;
				}
			
				for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
				{
					if ( dalGetIntByKey( ptEntry, ID, &nId ) == -1 )
					{
						fprintf( stderr, "dalGetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );

						if ( dalResFree( ptResult ) == -1 )
						{
							fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						}

						return SELECT_FAIL;
					}

					if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	
						if ( dalResFree( ptResult ) == -1 )
						{
							fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						}

						return SELECT_FAIL;				
					}	

					printf( "ID: %3d | Name: %s\n", nId, pszName );
				}
			}
			break;
		case 2:
		{
				if ( dalSetStringByKey( ptPstmt, NAME, szInput ) == -1 )
				{
					fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return SELECT_FAIL;
				}
				
				nRet = dalPreparedExec( ptConn, ptPstmt, &ptResult );
				if ( nRet == -1 )
				{
					fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;	
				}
				else if ( nRet == 0 )
				{
					printf( "[Select] %d Tuple\n", nRet );
					if ( dalResFree( ptResult ) == -1 )
					{
						fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						return SELECT_FAIL;
					}
					return SELECT_ZERO;		
				}

				for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
				{
					if ( dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}

					if ( dalGetStringByKey( ptEntry, TEAM, &pszTeam ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}	

					if ( dalGetStringByKey( ptEntry, PHONE, &pszPhone ) == -1 )
					{
						fprintf( stderr, "dalGetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						goto error_return;
					}
				}
				
				printf( "JobTitle: %s | Team: %s | Phone: %s\n", pszJobTitle, pszTeam, pszPhone );
		}
			break;
		default:
			break;
	}
	
	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return SELECT_FAIL;
	}	

	printf( "[Select] Back to Menu\n" );
	return SELECT_SUCCESS;

error_return:
	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}	

	return SELECT_FAIL;
}
*/

int Update( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt )
{
	if ( NULL == ptConn )
	{	
		printf( "[Update] ptConn NULL\n" );
		return UPDATE_FAIL;
	}

	if ( NULL == ptPstmt )
	{
		printf( "[Update] ptPstmt NULL\n" );
		return UPDATE_FAIL;
	}

	int nRet = 0;
	char *pszRet = NULL;
	
	char szInput	[32];
	char szQuery	[256];
	char szJobTitle	[32];
	char szTeam		[32];
	char szPhone	[32];

	memset( szInput,	0x00, sizeof(szInput) );
	memset( szQuery,	0x00, sizeof(szQuery) );
	memset( szJobTitle,	0x00, sizeof(szJobTitle) );
	memset( szTeam,		0x00, sizeof(szTeam) );
	memset( szPhone,	0x00, sizeof(szPhone) );

	printf( "Input ID: " );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szInput );

	nRet = CheckId( ptConn, atoi( szInput ) );
	if ( CHECKID_FAIL == nRet )
	{
		return UPDATE_FAIL;
	}
	else if ( CHECKID_INVALID == nRet )
	{
		printf( "[Update] ID Invalid\n" );
		return UPDATE_ZERO;
	}

	printf( "Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szJobTitle );

	printf( "Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szTeam );

	printf( "Phone: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szPhone );

	////////////////
	// Set By Key //
	////////////////
	if ( dalSetIntByKey( ptPstmt, ID, atoi( szInput ) ) == -1 )
	{
		fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}

	if ( dalSetStringByKey( ptPstmt, JOBTITLE, szJobTitle ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;	
	}

	if ( dalSetStringByKey( ptPstmt, TEAM, szTeam ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}

	if ( dalSetStringByKey( ptPstmt, PHONE, szPhone ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}

	nRet = dalPreparedExec( ptConn, ptPstmt, NULL );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	else if ( nRet == 0 )
	{
		printf( "[Update] %d Tuple\n", nRet );
		return UPDATE_ZERO;
	}

	printf( "[Update] Back to Menu\n" );
	return UPDATE_SUCCESS;
}

int Delete( DAL_CONN *ptConn, DAL_PSTMT *ptPstmt )
{
	if ( NULL == ptConn )
	{	
		printf( "[Delete] ptConn NULL\n" );
		return DELETE_FAIL;
	}

	if ( NULL == ptPstmt )
	{
		printf( "[Delete] ptPstmt NULL\n" );
		return DELETE_FAIL;
	}

	int nRet = 0;
	char *pszRet = NULL;
	
	char szInput[32];
	char szQuery[256];

	memset( szInput, 0x00, sizeof(szInput) );
	memset( szQuery, 0x00, sizeof(szQuery) );

	printf( "Input ID: " );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_FAIL;
	}
	ClearStdin( szInput );

	nRet = CheckId( ptConn, atoi( szInput ) );
	if ( CHECKID_FAIL == nRet )
	{
		return DELETE_FAIL;
	}
	else if ( CHECKID_INVALID == nRet )
	{
		printf( "[Delete] ID Invalid\n" );
		return DELETE_ZERO;
	}
	
	////////////////
	// Set By Key //
	////////////////
	if ( dalSetIntByKey( ptPstmt, ID, atoi( szInput ) ) == -1 )
	{
		fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_FAIL;
	}

	nRet = dalPreparedExec( ptConn, ptPstmt, NULL );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_FAIL;
	}
	else if ( nRet == 0 )
	{
		printf( "[DELETE] %d Tuple\n", nRet );
		return DELETE_ZERO;
	}

	printf( "[Delete] Back to Menu\n" );
	return DELETE_SUCCESS;
}

int CheckId( DAL_CONN *ptConn, int nId )
{
	if ( NULL == ptConn )
	{
		return CHECKID_FAIL;
	}
		
	int nRet = 0;
	char szQuery[256];
	
	DAL_RESULT_SET *ptResult = NULL;

	memset( szQuery, 0x00, sizeof(szQuery) );

	snprintf( szQuery, sizeof(szQuery), "select * from %s where %s = %d;", TABLE_NAME, ID, nId );
	szQuery[ strlen(szQuery) ] = '\0';

	nRet = dalExecute( ptConn, szQuery, &ptResult );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalExecute() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return CHECKID_FAIL;
	}	
	else if ( nRet == 0 )
	{
		printf( "[Check ID] %d Tuple\n", nRet );
		
		if ( dalResFree( ptResult ) == -1 )
		{
			return CHECKID_FAIL;
		}

		return CHECKID_INVALID;
	}

	if ( dalResFree( ptResult ) == -1 )
	{
		return CHECKID_FAIL;
	}

	printf( "[Check Id] Valid ID\n" );
	return CHECKID_VALID;
}
