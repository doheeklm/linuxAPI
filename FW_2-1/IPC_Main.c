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
	RUN = 100, STOP = -99, SUCCESS = 1, NOT_EXIST = 0,
	DAL_FAIL = -2, FGETS_FAIL = -3,	PTR_FAIL = -4, FUNC_FAIL = -5
} ReturnCode_t;

void	SignalHandler( int nSigno );
void	Menu();
void	ClearStdin( char *pszTemp );
int		InitPreparedStatement();
void	DestroyPreparedStatement();

int	Insert();
int Select();
int Update();
int Delete();
int CheckId( int nId );
int GetOriginalInfo( int nId, char *pszAttribute, char *pszOriginalInfo );

DAL_CONN	*g_ptConn= NULL;
DAL_PSTMT	*ptPstmt = NULL;
DAL_PSTMT	*g_ptPstmtInsert = NULL;
DAL_PSTMT	*g_ptPstmtSelectAll = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneById = NULL;
DAL_PSTMT	*g_ptPstmtSelectOneByName = NULL;
DAL_PSTMT	*g_ptPstmtUpdate = NULL;
DAL_PSTMT	*g_ptPstmtDelete = NULL;

int g_nFlag = RUN;

void SignalHandler( int nSigno )
{
	g_nFlag = STOP;
	
	printf( "\n[SignalHandler] %d\n", nSigno );

	exit( -1 );
}

int main( void )
{
	int nRet = 0;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	g_ptConn = dalConnect( NULL );
	if ( NULL == g_ptConn )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	if ( SUCCESS != InitPreparedStatement() )
	{
		goto end_of_function;
	}

	Menu( g_ptConn );

	DestroyPreparedStatement();

end_of_function:
	nRet = dalDisconnect( g_ptConn );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
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

void Menu()
{
	int nRet = 0;
	char *pszRet = NULL;
	int nPickMenu = 0;
	char szPickMenu[8];

	while ( RUN == g_nFlag )
	{
		nRet = 0;
		pszRet = NULL;
		nPickMenu = 0;
		memset( szPickMenu, 0x00, sizeof(szPickMenu) );

		do
		{
			printf( "=================\n"
					"(1) Insert Info\n"
					"(2) Select Info\n"
					"(3) Update Info\n"
					"(4) Delete Info\n"
					"(5) Exit program\n"
					"=================\n"
					"Input: " );

			pszRet = fgets( szPickMenu, sizeof(szPickMenu), stdin );
			if ( NULL == pszRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return;
			}
			ClearStdin( szPickMenu );

			nPickMenu = atoi( szPickMenu );
		}
		while ( nPickMenu < 1 || nPickMenu > 5);
	
		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert();
				if ( SUCCESS != nRet )
				{
					return;
				}
			}
				break;
			case 2:
			{
				nRet = Select();
				if ( SUCCESS != nRet )
				{	
					return;
				}
			}
				break;
			case 3:
			{
				nRet = Update();
				if ( SUCCESS != nRet )
				{
					return;
				}
			}
				break;
			case 4:
			{
				nRet = Delete();
				if ( SUCCESS != nRet )
				{
					return;
				}
			}
				break;
			case 5:
			{
				printf( "[ Menu ] Exit Program\n" );
				return;
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
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szName );

	//TODO 숫자 있으면 반환 atoi(Dora12) >> 0

	printf( "[Insert] Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szJobTitle );

	printf( "[Insert] Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szTeam );

	printf( "[Insert] Phone: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szPhone );

	// Set By Key
	nRet = dalSetStringByKey( g_ptPstmtInsert, NAME, szName );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, JOBTITLE, szJobTitle );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, TEAM, szTeam );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtInsert, PHONE, szPhone );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	// Execute	
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		printf( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	return SUCCESS;
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
	
		printf("=================\n"
			   "(1) Select All\n"
			   "(2) Select One\n"
			   "=================\n"
			   "Input: " );
	
		pszRet = fgets( szPickSelect, sizeof(szPickSelect), stdin );
		if ( NULL == pszRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return FGETS_FAIL;
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
			if ( -1 == nRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return DAL_FAIL;
			}
			else if ( 0 == nRet )
			{
				nRet = dalResFree( ptResult );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					return DAL_FAIL;
				}
				
				return NOT_EXIST;		
			}		
	
			// Get By Key	
			for ( ptEntry = dalFetchFirst( ptResult ); ptEntry != NULL; ptEntry = dalFetchNext( ptResult ) )
			{
				nRet = dalGetIntByKey( ptEntry, ID, &nId );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					goto error_return;
				}

				nRet = dalGetStringByKey( ptEntry, NAME, &pszName );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
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
			if ( NULL == pszRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return FGETS_FAIL;
			}
			ClearStdin( szInput );
		
			if ( atoi( szInput ) > 0 )
			{
				// Set By Key
				nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, atoi( szInput ) );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					return DAL_FAIL;
				}
			
				// Execute
				nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					return DAL_FAIL;
				}
				else if ( 0 == nRet )
				{
					nRet = dalResFree( ptResult );
					if ( -1 == nRet )
					{
						fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
						return DAL_FAIL;
					}
	
					printf( "[%s] Invalid\n", __func__ );				
					return NOT_EXIST;
				}
			}
			else if ( atoi( szInput ) == 0 )
			{
				// Set By Key
				nRet = dalSetStringByKey( g_ptPstmtSelectOneByName, NAME, szInput );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					return DAL_FAIL;
				}

				// Execute
				nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneByName, &ptResult );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					return DAL_FAIL;
				}
				else if ( 0 == nRet )
				{
					nRet = dalResFree( ptResult );
					if ( -1 == nRet )
					{
						fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
						return DAL_FAIL;
					}

					printf( "[%s] Invalid\n", __func__ );
					return NOT_EXIST;
				}
			}

			// Get By Key
			ptEntry = dalFetchFirst( ptResult );
			if ( NULL != ptEntry )
			{
				nRet = dalGetStringByKey( ptEntry, JOBTITLE, &pszJobTitle );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					goto error_return;
				}

				nRet = dalGetStringByKey( ptEntry, TEAM, &pszTeam );
				if ( -1 == nRet )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					goto error_return;
				}	

				if ( dalGetStringByKey( ptEntry, PHONE, &pszPhone ) == -1 )
				{
					fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
					goto error_return;
				}
			}

			printf( "[%s] JobTitle: %s | Team: %s | Phone: %s\n", __func__, pszJobTitle, pszTeam, pszPhone );
		}
			break;
		default:
			break;
	}

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}	

	printf( "[Select] Back to Menu\n" );
	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}	

	return DAL_FAIL;
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
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szInput );

	nRet = CheckId( atoi( szInput ) );
	if ( NOT_EXIST == nRet )
	{
		return NOT_EXIST;
	}
	else if ( DAL_FAIL == nRet )
	{
		return DAL_FAIL;
	}

	printf( "[Update] Job Title: " );
	pszRet = fgets( szJobTitle, sizeof(szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szJobTitle );

	printf( "[Update] Team: " );
	pszRet = fgets( szTeam, sizeof(szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szTeam );

	printf( "[Update] Phone: " );
	pszRet = fgets( szPhone, sizeof(szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szPhone );

	// If String is Empty
	if ( 0 == strlen(szJobTitle) )
	{
		if ( SUCCESS != GetOriginalInfo( atoi( szInput ), JOBTITLE, szJobTitle ) )
		{
			return FUNC_FAIL;
		}
	}

	if ( 0 == strlen(szTeam) )
	{
		if ( SUCCESS != GetOriginalInfo( atoi( szInput ), TEAM, szTeam ) )
		{
			return FUNC_FAIL;
		}
	}	

	if ( 0 == strlen(szPhone) )
	{
		if ( SUCCESS != GetOriginalInfo(  atoi( szInput ), PHONE, szPhone ) )
		{
			return FUNC_FAIL;	
		}
	}	

	// Set By Key
	nRet = dalSetIntByKey( g_ptPstmtUpdate, ID, atoi( szInput ) );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, JOBTITLE, szJobTitle );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet =  dalSetStringByKey( g_ptPstmtUpdate, TEAM, szTeam );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	nRet = dalSetStringByKey( g_ptPstmtUpdate, PHONE, szPhone );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtUpdate, NULL );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		printf( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}

	return SUCCESS;
}

int Delete()
{
	int nRet = 0;
	char *pszRet = NULL;
	char szInput[32];
	
	memset( szInput, 0x00, sizeof(szInput) );

	// Input
	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szInput );

	nRet = CheckId( atoi( szInput ) );
	if ( NOT_EXIST == nRet )
	{
		return NOT_EXIST;
	}
	else if ( DAL_FAIL == nRet )
	{
		return DAL_FAIL;
	}

	// Set By Key
	nRet = dalSetIntByKey( g_ptPstmtDelete, ID, atoi( szInput ) );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtDelete, NULL );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		printf( "[%s] %d Tuple\n", __func__, nRet );
		return NOT_EXIST;
	}
	
	return SUCCESS;
}

int CheckId( int nId )
{
	int nRet = 0;
	DAL_RESULT_SET *ptResult = NULL;

	// Set By Key
	nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, nId );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		nRet = dalResFree( ptResult );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		return NOT_EXIST;
	}

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

int GetOriginalInfo( int nId, char *pszAttribute, char *pszOriginalInfo )
{
	if ( NULL == pszAttribute )
	{
		printf( "pszAttribute NULL\n" );
		return PTR_FAIL;
	}
	
	int nRet = 0;
	char *pszTemp = NULL;
	
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	// Set By Key
	nRet = dalSetIntByKey( g_ptPstmtSelectOneById, ID, nId );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	// Execute
	nRet = dalPreparedExec( g_ptConn, g_ptPstmtSelectOneById, &ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}
	else if ( 0 == nRet )
	{
		nRet = dalResFree( ptResult );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			return DAL_FAIL;
		}
		return NOT_EXIST;
	}

	// Get By Key
	ptEntry = dalFetchFirst( ptResult );
	if ( NULL != ptResult )
	{
		nRet = dalGetStringByKey( ptEntry, pszAttribute, &pszTemp );
		if ( -1 == nRet )
		{
			fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
			goto error_return;
		}
	
		strcpy( pszOriginalInfo, pszTemp );
	}

	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;

error_return:
	nRet = dalResFree( ptResult );
	if ( -1 == nRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
	}

	return DAL_FAIL;
}
