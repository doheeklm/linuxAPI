/* TAP_IPC_Client.c */
//입출력을 담당하는 프로세스

#include "TAP_IPC_Inc.h"

#define SERVER_PROCESS	"MIPCSVR"
#define CLIENT_PROCESS	"MIPCCLI"
#define INSERT			"Insert"
#define SELECT			"Select"
#define UPDATE			"Update"
#define DELETE			"Delete"

typedef enum
{
	RUN = 100, STOP = -99,
	SUCCESS = 1, INPUT_FAIL = 1,
	NOT_EXIST = 0,
	DAL_FAIL = -2, FGETS_FAIL = -3,
	PTR_FAIL = -4, //매개변수로 받은 포인터가 NULL일때
	FUNC_FAIL = -5
} ReturnCode_t;

typedef struct INFO_s
{
	int nDB;
	int nId;
	char szName		[32];
	char szJobTitle	[32];
	char szTeam		[32];
	char szPhone	[14];
} INFO_t;

INFO_t g_tInfo;

int g_nFlag = RUN;

void SignalHandler( int nSig );
void ClearStdin( char *pszTemp );

int	Insert();
int Select();
int Update();
int Delete();

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15
	
	iipc_ds_t	tIpc;		// IPC open하여 얻어지는 ipc common data structure pointer
	iipc_msg_t	tMsg;		// IPC를 통해 송신되는 message structure에 대한 pointer
	iipc_key_t	tSrcKey;
	iipc_key_t	tDstKey;

	int nRet = 0;
	char *pszRet = NULL;
	int nPickMenu = 0;
	char szPickMenu[8];

	// TAP_ipc_open: IPC를 사용하기 위한 초기화 작업
	nRet = TAP_ipc_open( &tIpc, CLIENT_PROCESS );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_open() errno[%d]\n", __func__, ipc_errno );
		return -1;
	}	

	// TAP_ipc_getkey: IPC를 통해 메세지를 전송하기 위해서 해당 프로세스의 IPC Key를 얻어오는 작업
	tDstKey = TAP_ipc_getkey( &tIpc, SERVER_PROCESS );
	if ( IPC_NOPROC == tDstKey )
	{
		fprintf( stderr, "%s TAP_ipc_getkey() errno[%d]\n", __func__, ipc_errno );
		goto error_return;
	}

	tSrcKey = TAP_ipc_getkey( &tIpc, CLIENT_PROCESS );
	if ( IPC_NOPROC == tSrcKey )
	{
		fprintf( stderr, "%s TAP_ipc_getkey() errno[%d]\n", __func__, ipc_errno );
		goto error_return;
	}

	while ( RUN == g_nFlag )
	{
		nRet = 0;
		pszRet = NULL;
		nPickMenu = 0;
		memset( szPickMenu, 0x00, sizeof(szPickMenu) );
		memset( &g_tInfo, 0x00, sizeof(INFO_t) );	
		
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
		}
		while ( nPickMenu < 1 || nPickMenu > 5);
		
		switch ( nPickMenu )
		{
			case 1:
			{
				nRet = Insert();
				//TODO INPUT_FAIL
				if ( SUCCESS != nRet )
				{
					goto error_return;
				}
			}
				break;
			case 2:
			{
				nRet = Select();
				if ( SUCCESS != nRet )
				{	
					goto error_return;
				}
			}
				break;
			case 3:
			{
				nRet = Update();
				if ( SUCCESS != nRet )
				{
					goto error_return;
				}
			}
				break;
			case 4:
			{
				nRet = Delete();
				if ( SUCCESS != nRet )
				{
					goto error_return;
				}
			}
				break;
			case 5:
			{
				printf( "[ Menu ] Exit Program\n" );
				goto error_return;
			}
				break;
			default:
				break;
		}
		
		//tMsg.buf.msgq_buf 와 g_tInfo 형변환
		//memcpy( tMsg.buf.msgq_buf, &g_tInfo, sizeof(INFO_t) );

		tMsg.u.h.src = tSrcKey;
		tMsg.u.h.dst = tDstKey;
		tMsg.u.h.len = strlen(tMsg.buf.msgq_buf) + 1; // for null-terminating

		nRet = TAP_ipc_msgsnd( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgsnd() errno[%d]\n", __func__, ipc_errno );
			goto error_return;
		}

		nRet = TAP_ipc_msgrcv( &tIpc, &tMsg, IPC_BLOCK );
		if ( 0 > nRet )
		{
			fprintf( stderr, "%s TAP_ipc_msgrcv() errno[%d]\n", __func__, ipc_errno );
			goto error_return;
		}

		printf( "\n[CLEINT RECEIVING]: %s\n", tMsg.buf.msgq_buf );
	}

	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_close() errno[%d]\n", __func__, ipc_errno );
		return -1;
	}

	return 0;

error_return:
	nRet = TAP_ipc_close( &tIpc );
	if ( 0 > nRet )
	{
		fprintf( stderr, "%s TAP_ipc_close() errno[%d]\n", __func__, ipc_errno );
	}

	return -1;
}

void SignalHandler( int nSig )
{
	g_nFlag = STOP;

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

int Insert()
{
	size_t i = 0;
	int nRet = 0;
	char *pszRet = NULL;

	g_tInfo.nDB = 0;

	printf( "[%s] Name: ", __func__ );

	pszRet = fgets( g_tInfo.szName, sizeof(g_tInfo.szName), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szName );

	for ( i = 0; i < strlen(g_tInfo.szName); i++ )
	{
		nRet = isdigit( g_tInfo.szName[i] );
		if ( 0 != nRet )
		{
			printf( "[%s] Number in name:%s | Back to Menu\n", __func__, g_tInfo.szName );	
			return INPUT_FAIL;
		}
	}

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( g_tInfo.szJobTitle, sizeof(g_tInfo.szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( g_tInfo.szTeam, sizeof(g_tInfo.szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( g_tInfo.szPhone, sizeof(g_tInfo.szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szPhone );

	return SUCCESS;
}

int Select()
{
	int nPickSelect = 0;
	char szPickSelect[8];
	char szInput [32];
	char *pszRet = NULL;
	memset( szInput, 0x00, sizeof(szInput) );

	g_tInfo.nDB = 1;

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
		
			if ( atoi( szInput ) > 0 )
			{
				g_tInfo.nId = atoi( szInput );
			}
			else if ( atoi( szInput ) == 0 )
			{
				// NOTE: size check
				strncpy( g_tInfo.szName, szInput, sizeof(g_tInfo.szName) - 1 );
			  	g_tInfo.szName[ strlen(g_tInfo.szName) ] = '\0';	
			}
		}
			break;
		default:
			break;
	}

	return SUCCESS;
}

int Update()
{
	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	g_tInfo.nDB = 2;

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
		g_tInfo.nId = atoi( szInput );
	}

	printf( "[%s] Job Title: ", __func__ );
	pszRet = fgets( g_tInfo.szJobTitle, sizeof(g_tInfo.szJobTitle), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szJobTitle );

	printf( "[%s] Team: ", __func__ );
	pszRet = fgets( g_tInfo.szTeam, sizeof(g_tInfo.szTeam), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szTeam );

	printf( "[%s] Phone: ", __func__ );
	pszRet = fgets( g_tInfo.szPhone, sizeof(g_tInfo.szPhone), stdin );
	if ( NULL == pszRet )
	{
		fprintf( stderr, "%s errno[%d]\n", __func__, dalErrno() );
		return FGETS_FAIL;
	}
	ClearStdin( g_tInfo.szPhone );

	return SUCCESS;
}

int Delete()
{
	char *pszRet = NULL;
	char szInput[32];
	memset( szInput, 0x00, sizeof(szInput) );

	g_tInfo.nDB = 3;

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
		g_tInfo.nId = atoi( szInput );
	}

	return SUCCESS;
}
