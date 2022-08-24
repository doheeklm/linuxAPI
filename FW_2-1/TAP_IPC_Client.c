/* TAP_IPC_Client.c */
//입출력을 담당하는 프로세스

#include "TAP_IPC_Inc.h"

void SignalHandler( int nSig );
void ClearStdin( char *pszTemp );

int	Insert( struct INFO_s *ptInfo );
int Select( struct INFO_s *ptInfo );
int Update( struct INFO_s *ptInfo );
int Delete( struct INFO_s *ptInfo );

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	INFO_t *ptInfo = NULL;

	iipc_msg_t	tMsg;
	iipc_ds_t	tIpc;
	iipc_key_t	tSrcKey;
	iipc_key_t	tDstKey;

	int nRet = 0;
	int nPickMenu = 0;
	char szPickMenu[8];
	char *pszRet = NULL;

	nRet = TAP_ipc_open( &tIpc, CLIENT_PROCESS );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_open() errno[%d]\n", __func__, ipc_errno );
		return -1;
	}	

	tDstKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tDstKey )
	{
		fprintf( stderr, "%s TAP_ipc_getkey() errno[%d]\n", __func__, ipc_errno );
		goto end_of_function;
	}

	tSrcKey = TAP_ipc_getkey( &tIpc, CLIENT_PROCESS );
	if ( IPC_NOPROC == tSrcKey )
	{
		fprintf( stderr, "%s TAP_ipc_getkey() errno[%d]\n", __func__, ipc_errno );
		goto end_of_function;
	}

	while ( FLAG_RUN == g_nFlag )
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
				return -1;
			}
			ClearStdin( szPickMenu );

			nPickMenu = atoi( szPickMenu );
		} while ( nPickMenu < 1 || nPickMenu > 5);
	
		ptInfo = (INFO_t *)tMsg.buf.msgq_buf;	

		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert( ptInfo );
				if ( SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 2:
			{
				nRet = Select( ptInfo );
				if ( SUCCESS != nRet )
				{	
					goto end_of_function;
				}
			}
				break;
			case 3:
			{
				nRet = Update( ptInfo );
				if ( SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 4:
			{
				nRet = Delete( ptInfo );
				if ( SUCCESS != nRet )
				{
					goto end_of_function;
				}
			}
				break;
			case 5:
			{
				printf( "[ Menu ] Exit Program\n" );
				goto end_of_function;
			}
				break;
			default:
				break;
		} // switch

		printf( "\n[CLIENT SEND]\n"
				"DB: %d\n"
				"Id: %d\n"
				"Name: %s\n"
				"JobTitle: %s\n"
				"Team: %s\n"
				"Phone: %s\n",
				ptInfo->nDB, ptInfo->nId, ptInfo->szName, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone ); 

		tMsg.u.h.src = tSrcKey;
		tMsg.u.h.dst = tDstKey;
		tMsg.u.h.len = sizeof(struct INFO_s);

		printf( "msg len: %d\n", tMsg.u.h.len );

		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgsnd() errno[%d]\n", __func__, ipc_errno );
			goto end_of_function;
		}

		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgrcv() errno[%d]\n", __func__, ipc_errno );
			goto end_of_function;
		}
//TODO Response`
		ptInfo = (INFO_t *)tMsg.buf.msgq_buf;
		
		printf( "\n[CLIENT RECV]\n"
				"DB: %d\n"
				"Id: %d\n"
				"Name: %s\n"
				"JobTitle: %s\n"
				"Team: %s\n"
				"Phone: %s\n",
				ptInfo->nDB, ptInfo->nId, ptInfo->szName, ptInfo->szJobTitle, ptInfo->szTeam, ptInfo->szPhone ); 
	} // while

end_of_function:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_close() errno[%d]\n", __func__, ipc_errno );
		return -1;
	}

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	printf( "\n[SignalHandler] %d\n", nSig );

	exit( -1 );
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

int Insert( struct INFO_s *ptInfo )
{
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}
	
	char *pszRet = NULL;

	ptInfo->nDB = 1;

	printf( "[%s] Name: ", __func__ );

	pszRet = fgets( ptInfo->szName, sizeof(ptInfo->szName), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szName );

	/*
	size_t i = 0;
	int nRet = 0;
	for ( i = 0; i < strlen(ptInfo->szName); i++ )
	{
		nRet = isdigit( ptInfo->szName[i] );
		if ( 0 != nRet )
		{
			printf( "[%s] Name %s에 숫자가 포함되었\n..Back to Menu..\n", __func__, ptInfo->szName );	
			return INPUT_FAIL;
		}
	}*/

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptInfo->szJobTitle, sizeof(ptInfo->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptInfo->szTeam, sizeof(ptInfo->szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptInfo->szPhone, sizeof(ptInfo->szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szPhone );
	
	return SUCCESS;
}

int Select( struct INFO_s *ptInfo )
{	
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}

	int nPickSelect = 0;
	char szPickSelect[8];
	char szInput [32];
	char *pszRet = NULL;
	memset( szInput, 0x00, sizeof(szInput) );

	ptInfo->nDB = 2;

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
			printf( "No Input\n" );
		}
			break;
		case 2:
		{
			printf( "[%s] Input ID or Name: ", __func__ );
			
			pszRet = fgets( szInput, sizeof(szInput), stdin );
			if ( NULL == pszRet )
			{
				fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
				return FGETS_FAIL;
			}
			ClearStdin( szInput );

			//Input ID: 5
			//Name: Dohee >x
			//
			//Input ID: 3
			//Name: Dohee >ok
			//
			//Input ID:
			//Name: Dohee >ok
		//TODO id check ..number
			
			if ( atoi( szInput ) > 0 )
			{
				ptInfo->nId = atoi( szInput );
			}
			else if ( atoi( szInput ) == 0 )
			{
				strlcpy( ptInfo->szName, szInput, sizeof(szInput) );
			}
		}
			break;
		default:
			break;
	}

	return SUCCESS;
}

int Update( struct INFO_s *ptInfo )
{
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}
	
	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	ptInfo->nDB = 3;

	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szInput );

	if ( atoi( szInput ) > 0 )
	{
		ptInfo->nId = atoi( szInput );
	}
	//TODO else errno

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( ptInfo->szJobTitle, sizeof(ptInfo->szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( ptInfo->szTeam, sizeof(ptInfo->szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( ptInfo->szPhone, sizeof(ptInfo->szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( ptInfo->szPhone );

	return SUCCESS;
}

int Delete( struct INFO_s *ptInfo )
{
	if ( NULL != ptInfo )
	{
		memset( ptInfo, 0x00, sizeof(struct INFO_s) );
	}

	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	ptInfo->nDB = 4;

	printf( "[%s] Input ID: ", __func__ );
	pszRet = fgets( szInput, sizeof(szInput), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( szInput );

	if ( atoi( szInput ) > 0 )
	{
		ptInfo->nId = atoi( szInput );
	}
	//TODO else name

	return SUCCESS;
}
