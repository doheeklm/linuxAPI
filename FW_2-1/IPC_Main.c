/* IPC_Main.c */
#include "IPC_Inc.h"

#define TABLE_NAME	"EmployeeInfos"
#define ID			"id"
#define NAME		"name"
#define JOBTITLE	"jobTitle"
#define TEAM		"team"
#define PHONE		"phone"

typedef enum
{
	//DAL_FAIL, FGETS_FAIL 
	//__func__ %s
	//SUCCESS = 1
	//NOT_EXIST = 0
	INSERT_FAIL = -1, INSERT_SUCCESS = 1, INSERT_ZERO = 0,
	SELECT_FAIL = -2, SELECT_SUCCESS = 1, SELECT_ZERO = 0,
	UPDATE_FAIL = -3, UPDATE_SUCCESS = 1, UPDATE_ZERO = 0,
	DELETE_FAIL = -4, DELETE_SUCCESS = 1, DELETE_ZERO = 0,
	CHECKID_FAIL = -5, CHECKID_VALID = 1, CHECKID_INVALID = 0,
	GETORIGINAL_FAIL = -1, GETORIGINAL_SUCCESS = 1, GETORIGINAL_ZERO = 0,
	INIT_FAIL = -1, INIT_SUCCESS = 1,
	DESTROY_FAIL = -1, DESTROY_SUCCESS = 1
} ReturnCode_t;

void SignalHandler( int nSigno );
void Menu();
void ClearStdin( char *pszTemp );
int InitPreparedStatement();
int DestroyPreparedStatement();

int Insert();
int Select();
int Update();
int Delete();
int CheckId( int nId );
int GetOriginal( int nId, char *pszAttribute, char *pszOriginal );

DAL_CONN *g_ptConn = NULL;
DAL_PSTMT *ptPstmt = NULL;
DAL_PSTMT *g_ptPstmtInsert = NULL;
DAL_PSTMT *g_ptPstmtSelectAll = NULL;
DAL_PSTMT *g_ptPstmtSelectOneById = NULL;
DAL_PSTMT *g_ptPstmtSelectOneByName = NULL;
DAL_PSTMT *g_ptPstmtUpdate = NULL;
DAL_PSTMT *g_ptPstmtDelete = NULL;

void SignalHandler( int nSigno )
{
	//TODO while(flag) flag 0

	printf( "\n[SignalHandler] %d\n", nSigno );

	exit( -1 );
}

int main( void )
{

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		fprintf( stderr, "dalConnect() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		exit( -1 );
	}

	if ( INIT_FAIL == InitPreparedStatement() )
	{
		goto end_of_function;
	}

	Menu( g_ptConn );

	if ( DESTROY_FAIL == DestroyPreparedStatement() )
	{

	}

end_of_function:
	if ( dalDisconnect( g_ptConn ) == -1 )
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
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INIT_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s from %s;", ID, NAME, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectAll = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INIT_FAIL;
	}

	//SELECT ONE BY ID ==> Select(), CheckId(), GetOriginal()에서 사용
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneById = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneById )
	{
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INIT_FAIL;
	}

	//SELECT ONE BY NAME
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s from %s where %s = ?%s;", JOBTITLE, TEAM, PHONE, TABLE_NAME, NAME, NAME );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtSelectOneByName = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtSelectOneByName )
	{
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INIT_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;", TABLE_NAME, JOBTITLE, JOBTITLE, TEAM, TEAM, PHONE, PHONE, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtUpdate = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INIT_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "delete from %s where %s = ?%s;", TABLE_NAME, ID, ID );
	szQuery[ strlen(szQuery) ] = '\0';

	g_ptPstmtDelete = dalPreparedStatement( g_ptConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		fprintf( stderr, "dalPreparedStatement() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INIT_FAIL;
	}

	return INIT_SUCCESS;
}

int DestroyPreparedStatement()
{
	if ( dalDestroyPreparedStmt( g_ptPstmtInsert ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDestroyPreparedStmt( g_ptPstmtSelectAll ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDestroyPreparedStmt( g_ptPstmtSelectOneById ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDestroyPreparedStmt( g_ptPstmtSelectOneByName ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDestroyPreparedStmt( g_ptPstmtUpdate ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	if ( dalDestroyPreparedStmt( g_ptPstmtDelete ) == -1 )
	{
		fprintf( stderr, "dalDestroyPreparedStmt() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}
//TODO void	
	return DESTROY_SUCCESS;
}

void Menu()
{
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;

	while ( 1 )
	{
		nPickMenu = 0;
		memset( szPickMenu, 0x00, sizeof(szPickMenu) );

		printf( "=================\n(1) Insert Info\n(2) Select Info\n(3) Update Info\n(4) Delete Info\n(5) Exit program\n=================\nInput: " );
		pszRet = fgets( szPickMenu, sizeof(szPickMenu), stdin );
		if ( pszRet == NULL )
		{
			fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return;
		}
		ClearStdin( szPickMenu );

		nPickMenu = atoi( szPickMenu );

		if ( nPickMenu == 5 )
		{
			printf( "[ Menu ] Exit Program\n" );
			break;
		}
		else if ( nPickMenu < 1 || nPickMenu > 4 )
		{
			printf( "[ Menu ] Back to Menu\n" );
			continue;
		}
	
		int nRet;
		
		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert();
				if ( INSERT_FAIL == nRet ) //SUCCESS != nRet
				{
					return;
				}
			}
				break;
			case 2:
			{ //TODO nRet
				if ( SELECT_FAIL == Select() )
				{	
					return;
				}
			}
				break;
			case 3:
			{
				if ( UPDATE_FAIL == Update() )
				{
					return;
				}
			}
				break;
			case 4:
			{
				if ( DELETE_FAIL == Delete() )
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

int Insert()
{
	int nRet = 0;
	char *pszRet = NULL;

	char szName		[32];
	char szJobTitle	[32];
	char szTeam		[32];
	char szPhone	[14];
	
	memset( szName,		0x00, sizeof(szName) );
	memset( szJobTitle,	0x00, sizeof(szJobTitle) );
	memset( szTeam,		0x00, sizeof(szTeam) );
	memset( szPhone,	0x00, sizeof(szPhone) );

	// Input
	printf( "[Insert] Name: " );
	pszRet = fgets( szName, sizeof(szName), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szName );
//TODO 숫자 있으면 반환 atoi(Dora12) >> 0
	printf( "[Insert] Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szJobTitle );

	printf( "[Insert] Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szTeam );

	printf( "[Insert] Phone: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}
	ClearStdin( szPhone );

	// Set By Key
	if ( dalSetStringByKey( g_ptPstmtInsert, NAME, szName ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	if ( dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, szJobTitle ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	if ( dalSetStringByKey( g_ptPstmtInsert, TEAM, szTeam ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	if ( dalSetStringByKey( g_ptPstmtInsert, PHONE, szPhone ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return INSERT_FAIL;
	}

	// Execute	
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtInsert, NULL );
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

int Select()
{
	int nRet = 0;
	int nPickSelect = 0;
	char szPickSelect[8];
	char* pszRet = NULL;	

	int nId = 0;
	char* pszName = NULL;
	char* pszJobTitle = NULL;
	char* pszTeam = NULL;
	char* pszPhone = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;
	
	char szInput [32];
	memset( szInput, 0x00, sizeof(szInput) );

	// Input
	do
	{
		nPickSelect = 0;
		memset( szPickSelect, 0x00, sizeof(szPickSelect) );
	
		printf("=================\n(1) Select All\n(2) Select One\n=================\nInput: " );
	
		pszRet = fgets( szPickSelect, sizeof(szPickSelect), stdin );
		if ( pszRet == NULL )
		{
			fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return SELECT_FAIL;
		}
		ClearStdin( szPickSelect );

		nPickSelect = atoi( szPickSelect );
	} while ( nPickSelect != 1 && nPickSelect != 2 );

	switch ( nPickSelect )
	{
		case 1:
		{
			// Execute
			nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectAll, &ptResult );
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
	
			// Get By Key	
			for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
			{
				if ( dalGetIntByKey( ptEntry, ID, &nId ) == -1 )
				{
					fprintf( stderr, "dalGetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;
				}

				if ( dalGetStringByKey( ptEntry, NAME, &pszName ) == -1 )
				{
					fprintf( stderr, "dalGetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;
				}	

					printf( "[Select] ID: %3d | Name: %s\n", nId, pszName );
				}
		}
			break;
		case 2:
		{
			// Input
			printf( "[Select] Input ID or Name: " );
			pszRet = fgets( szInput, sizeof(szInput), stdin );
			if ( pszRet == NULL )
			{
				fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
				return UPDATE_FAIL;
			}
			ClearStdin( szInput );
		
			if ( atoi( szInput )  > 0 )
			{
				// Set By Key
				if ( dalSetIntByKey( g_ptPstmtSelectOneById, ID, atoi( szInput ) ) == -1 )
				{
					fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return SELECT_FAIL;
				}
			
				// Execute
				nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
				if ( nRet == -1 )
				{
					fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;	
				}
				else if ( nRet == 0 )
				{
					printf( "[Select] Invalid ID\n" );
					if ( dalResFree( ptResult ) == -1 )
					{
						fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						return SELECT_FAIL;
					}
					return SELECT_ZERO;	
				}
			}
			else if ( atoi( szInput ) == 0 )
			{
				// Set By Key
				if ( dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, szInput ) == -1 )
				{
					fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					return SELECT_FAIL;
				}

				// Execute
				nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneByName, &ptResult );
				if ( nRet == -1 )
				{
					fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
					goto error_return;	
				}
				else if ( nRet == 0 )
				{
					printf( "[Select] Invalid NAME\n" );
					if ( dalResFree( ptResult ) == -1 )
					{
						fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
						return SELECT_FAIL;
					}
					return SELECT_ZERO;		
				}
			}

			// Get By Key
		//	for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
		//TODO for문	
			ptEntry = dalFetchFirst( ptResult );
			
			if ( ptEntry != NULL )
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

			printf( "[Select] JobTitle: %s | Team: %s | Phone: %s\n", pszJobTitle, pszTeam, pszPhone );
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

int Update()
{
	int nRet = 0;
	char *pszRet = NULL;
	
	char szInput	[32];
	char szJobTitle	[32];
	char szTeam		[32];
	char szPhone	[32];

	memset( szInput,	0x00, sizeof(szInput) );
	memset( szJobTitle,	0x00, sizeof(szJobTitle) );
	memset( szTeam,		0x00, sizeof(szTeam) );
	memset( szPhone,	0x00, sizeof(szPhone) );

	// Input
	printf( "[Update] Input ID: " );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szInput );

	nRet = CheckId( atoi( szInput ) );
	if ( CHECKID_FAIL == nRet )
	{
		return UPDATE_FAIL;
	}
	else if ( CHECKID_INVALID == nRet )
	{
		printf( "[Update] Invalid ID\n" );
		return UPDATE_ZERO;
	}

	printf( "[Update] Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szJobTitle );

	printf( "[Update] Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szTeam );

	printf( "[Update] Phone: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}
	ClearStdin( szPhone );

	// If String is Empty
	if ( strlen(szJobTitle) == 0 )
	{
		if ( GETORIGINAL_FAIL == GetOriginal( atoi( szInput ), JOBTITLE, szJobTitle ) )
		{
			return UPDATE_FAIL;
		}
	}

	if ( strlen(szTeam) == 0 )
	{
		if ( GETORIGINAL_FAIL == GetOriginal( atoi( szInput ), TEAM, szTeam ) )
		{
			return UPDATE_FAIL;
		}
	}	

	if ( strlen(szPhone) == 0 )
	{
		if ( GETORIGINAL_FAIL == GetOriginal(  atoi( szInput ), PHONE, szPhone ) )
		{
			return UPDATE_FAIL;
		}
	}	

	// Set By Key
	if ( dalSetIntByKey( g_ptPstmtUpdate, ID, atoi( szInput ) ) == -1 )
	{
		fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}

	if ( dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, szJobTitle ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;	
	}

	if ( dalSetStringByKey( g_ptPstmtUpdate, TEAM, szTeam ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}

	if ( dalSetStringByKey( g_ptPstmtUpdate, PHONE, szPhone ) == -1 )
	{
		fprintf( stderr, "dalSetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return UPDATE_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtUpdate, NULL );
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

int Delete()
{
	int nRet = 0;
	char *pszRet = NULL;
	char szInput[32];
	
	memset( szInput, 0x00, sizeof(szInput) );

	// Input
	printf( "[Delete] Input ID: " );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( pszRet == NULL )
	{
		fprintf( stderr, "fgets() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_FAIL;
	}
	ClearStdin( szInput );

	nRet = CheckId( atoi( szInput ) );
	if ( CHECKID_FAIL == nRet )
	{
		return DELETE_FAIL;
	}
	else if ( CHECKID_INVALID == nRet )
	{
		printf( "[Delete] Invalid ID\n" );
		return DELETE_ZERO;
	}
	
	// Set By Key
	if ( dalSetIntByKey( g_ptPstmtDelete, ID, atoi( szInput ) ) == -1 )
	{
		fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtDelete, NULL );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return DELETE_FAIL;
	}
	else if ( nRet == 0 )
	{
		printf( "[Delete] %d Tuple\n", nRet );
		return DELETE_ZERO;
	}

	printf( "[Delete] Back to Menu\n" );
	return DELETE_SUCCESS;
}

int CheckId( int nId )
{
	int nRet = 0;

	DAL_RESULT_SET *ptResult = NULL;

	// Set By Key
	if ( dalSetIntByKey( g_ptPstmtSelectOneById, ID, nId ) == -1 )
	{
		fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return CHECKID_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return CHECKID_FAIL;
	}
	else if ( nRet == 0 )
	{
		if ( dalResFree( ptResult ) == -1 )
		{
			fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return CHECKID_FAIL;
		}
		return CHECKID_INVALID;
	}

	if ( dalResFree( ptResult ) == -1 )
	{
		return CHECKID_FAIL;
	}

	return CHECKID_VALID;
}

int GetOriginal( int nId, char *pszAttribute, char *pszOriginal )
{
	if ( NULL == pszAttribute )
	{
		printf( "pszAttribute NULL\n" );
		return GETORIGINAL_FAIL;
	}
	
	int nRet = 0;
	char *pszTemp = NULL;
	
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	// Set By Key	
	if ( dalSetIntByKey( g_ptPstmtSelectOneById, ID, nId ) == -1 )
	{
		fprintf( stderr, "dalSetIntByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return GETORIGINAL_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( nRet == -1 )
	{
		fprintf( stderr, "dalPreparedExec() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		goto error_return;	
	}
	else if ( nRet == 0 )
	{
		if ( dalResFree( ptResult ) == -1 )
		{
			fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			return GETORIGINAL_FAIL;
		}
		return GETORIGINAL_ZERO;	
	}

	// Get By Key
	// TODO 수정
	for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
	{
		if ( dalGetStringByKey( ptEntry, pszAttribute, &pszTemp ) == -1 )
		{
			fprintf( stderr, "dalGetStringByKey() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
			goto error_return;	
		}
		else
		{
			strcpy( pszOriginal, pszTemp );
			break;
		}
	}

	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
		return GETORIGINAL_FAIL;
	}

	return GETORIGINAL_SUCCESS;

error_return:
	if ( dalResFree( ptResult ) == -1 )
	{
		fprintf( stderr, "dalResFree() errno[%d] errmsg[%s]\n", dalErrno(), dalErrmsg( dalErrno() ) );
	}

	return GETORIGINAL_FAIL;
}
