/* RAS_Main.c */
#include "RAS_Inc.h"

int g_nSvcFlag = START_SVC;
int g_nWorkerIndex = 0; 
int g_nAlarmStatus = 0; //Thread

Env_t g_tEnv;
mpipc_t *g_ptMpipc = NULL;
oammmc_t *g_ptOammmc = NULL;
WORKER_t g_tWorker[MAX_WORKER_CNT];
TRC_t g_tTrc[MAX_TRC_CNT];

static void SignalHandler( int nSigno );

int main( void )
{
	mpsignal_set_func( tb_signal );
	mpsignal( SIGHUP, SignalHandler );
	mpsignal( SIGINT, SignalHandler );
	mpsignal( SIGTERM, SignalHandler );
	mpsignal( SIGQUIT, SignalHandler );
	mpsignal( SIGKILL, SignalHandler );
	mpsignal( SIGPIPE, SIG_IGN );

	int nRC = 0;
	int nIndex = 0;
	int nListenFd = 0;
	int nEpollFd = 0;
	int nCreateListenFd = CREATE_ON;
	int nCreateEpollFd = CREATE_ON;
	
	DB_t tDBMain;
	memset( &tDBMain, 0x00, sizeof(tDBMain) );

	nRC = CONFIG_Init();
	if ( RAS_rOK != nRC )
	{
		printf( "CONFIG_Init fail <%d>\n", nRC );
		return 0;
	}

	nRC = LOG_Init();
	if ( RAS_rOK != nRC ) 
	{
		printf( "LOG_Init fail <%d>\n", nRC );
		return 0;
	}
	
	//TODO DBIpc
	nRC = IPC_Init();
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;	
	}
	
	nRC = REGI_Init();
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}

	nRC = MMC_Init();
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}

	nRC = DB_Init( &(tDBMain.ptDBConn) );
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}	
	
	nRC = DB_InitPreparedStatement( &tDBMain );
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}	

	nRC = ALARM_Init();
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}

	nRC = STAT_Init();
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}

	nRC = WORKER_Init();
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}

	while ( g_nSvcFlag )
	{
		if ( CREATE_ON == nCreateListenFd )
		{
			nRC = SOCKET_Init( &nListenFd );
			if ( RAS_rOK != nRC )
			{
				goto _continue_with_error;
			}
			nCreateListenFd = CREATE_OFF;
		};

		if ( CREATE_ON == nCreateEpollFd )	
		{
			nRC = EVENT_Init( &nEpollFd );
			if ( RAS_rOK != nRC )
			{
				goto _continue_with_error;
			}
			nCreateEpollFd = CREATE_OFF;
		}

		//TODO 둘다 close epoll listen fd strerr _
		nRC = EVENT_AddToList( nListenFd, nEpollFd );
		if ( RAS_rOK != nRC )
		{
			goto _continue_with_error;
		}

		nRC = EVENT_Wait( nListenFd, nEpollFd, tDBMain );
		if ( RAS_rOK != nRC )
		{
			goto _continue_with_error;
		}

		nCreateListenFd = CREATE_OFF;
		nCreateEpollFd = CREATE_OFF;

_continue_with_error:
		switch ( nRC )
		{
			case RAS_rErrSocketInit:
			{
				nCreateListenFd = CREATE_ON;
			}
				break;
			case RAS_rErrEventInit:
			{
				nCreateEpollFd = CREATE_ON;
			}
				break;
			case RAS_rErrListenFd:
			{
				FD_DELETE( nEpollFd, nListenFd );
				FD_CLOSE( nListenFd );
				nCreateListenFd = CREATE_ON;
				nCreateEpollFd = CREATE_OFF;
			}
				break;
			case RAS_rErrEpollFd:
			{
				FD_CLOSE( nEpollFd );
				nCreateListenFd = CREATE_OFF;
				nCreateEpollFd = CREATE_ON;
			}
			case RAS_rErrEpollFdListenFd:
			{
				FD_CLOSE( nEpollFd );
				FD_CLOSE( nListenFd );
				nCreateListenFd = CREATE_ON;
				nCreateEpollFd = CREATE_ON;
			}
				break;
			default:
				break;
		}
	} 

_exit_main:
	printf( "end while loop\n" );

	for ( nIndex = 0; nIndex < MAX_WORKER_CNT; nIndex++ )
	{
		nRC = pthread_join( g_tWorker[nIndex].nThreadId, NULL );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_join fail <%d>", nRC );
		}
		LOG_DBG_F( "pthread_join <%d>", nRC );
	}

	FD_CLOSE( nEpollFd );
	FD_CLOSE( nListenFd );
	stgen_close();
	//oam_uda_del_alarm( iipc_ds_t *, upp_gname, low_gname, item_name, OAM_SFM_UDA_NOTI_ON );
	DB_Close( &tDBMain );
	MMC_Destroy();
	IPC_Destroy();
	MPGLOG_DESTROY();
	
	return 0;
}

static void SignalHandler( int nSigno )
{
	MPGLOG_DBG( "Signal <%d>", nSigno );
	
	g_nSvcFlag = STOP_SVC;
	
	return;
}
