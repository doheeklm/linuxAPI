/* RAS_Main.c */
#include "RAS_Inc.h"

int				g_nSvcFlag = START_SVC;
Env_t			g_tEnv;

DB_t			g_tDBMain;
DB_t			g_tDBIpc;
mpipc_t			*g_ptMpipc = NULL;
oammmc_t		*g_ptOammmc = NULL;

int				g_nUser = 0;
int				g_nAlarmStatus = 0;
pthread_mutex_t	g_tMutex = PTHREAD_MUTEX_INITIALIZER;

extern WORKER_t	g_tWorker[MAX_WORKER_CNT];

void SignalHandler( int nSigno );

int main( void )
{
	mpsignal_set_func( tb_signal );
	mpsignal( SIGHUP, SignalHandler );
	mpsignal( SIGINT, SignalHandler );
	mpsignal( SIGTERM, SignalHandler );
	mpsignal( SIGQUIT, SignalHandler );
	mpsignal( SIGKILL, SignalHandler );

	int nRC = 0;
	int nIndex = 0;
	int nListenFd = 0;
	int nEpollFd = 0;
	int bCreateListenFd = ON;
	int bCreateEpollFd = ON;
	int bAddFdToEpoll = ON; 
	memset( &g_tDBMain, 0x00, sizeof(g_tDBMain) );
	memset( &g_tDBIpc, 0x00, sizeof(g_tDBIpc) );

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

	nRC = DB_Init( &(g_tDBMain.ptDBConn) );
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}	

	nRC = DB_Init( &(g_tDBIpc.ptDBConn) );
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}	

	nRC = DB_InitPreparedStatement( &g_tDBMain );
	if ( RAS_rOK != nRC )
	{
		goto _exit_main;
	}	
	
	nRC = DB_InitPreparedStatement( &g_tDBIpc );
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
		nRC = ALARM_Report();
		if ( RAS_rOK != nRC )
		{
			goto _exit_main; 
		}

		if ( ON == bCreateListenFd )
		{
			nRC = SOCKET_Init( &nListenFd );
			if ( RAS_rOK != nRC )
			{
				continue;
			}
			PRT_CREATE_FD( "(listen", nListenFd );
			bCreateListenFd = OFF;
		};

		if ( ON == bCreateEpollFd )	
		{
			nRC = EVENT_Init( &nEpollFd );
			if ( RAS_rOK != nRC )
			{
				continue;
			}
			PRT_CREATE_FD( "(epoll", nEpollFd );
			bCreateEpollFd = OFF;
		}

		if ( ON == bAddFdToEpoll )
		{
			nRC = EVENT_Add( nListenFd, nEpollFd );
			if ( RAS_rOK != nRC )
			{
				FD_CLOSE( nEpollFd );
				FD_CLOSE( nListenFd );
				bCreateListenFd = ON;
				bCreateEpollFd = ON;
				continue;
			}
			bAddFdToEpoll = OFF;
		}

		nRC = EVENT_Wait( nListenFd, nEpollFd );
		if ( RAS_rErrEpollFd == nRC )
		{
			FD_CLOSE( nEpollFd );
			bCreateEpollFd = ON;
			bAddFdToEpoll = ON;
		}
		else if ( RAS_rErrListenFd == nRC )
		{
			FD_DELETE_AND_CLOSE( nListenFd, nEpollFd );
			bCreateListenFd = ON;
			bAddFdToEpoll = ON;
		}
	} 

_exit_main:
	PRT_EXIT;
	for ( nIndex = 0; nIndex < MAX_WORKER_CNT; nIndex++ )
	{
		THREAD_CANCEL( g_tWorker[nIndex].nThreadId );
		THREAD_JOIN( g_tWorker[nIndex].nThreadId );
	}
	FD_CLOSE( nEpollFd );
	FD_CLOSE( nListenFd );
	stgen_close();
	oam_uda_del_alarm( mpipc_tap_ipc(g_ptMpipc),
			UPP_GNAME, LOW_GNAME, ITEM_NAME, OAM_SFM_UDA_NOTI_ON );
	DB_Close( &g_tDBIpc );
	DB_Close( &g_tDBMain );
	MMC_Destroy();
	IPC_Destroy();
	MPGLOG_DESTROY();
	pthread_mutex_destroy( &g_tMutex );

	return 0;
}

void SignalHandler( int nSigno )
{
	MPGLOG_DBG( "Signal <%d>", nSigno );
	
	g_nSvcFlag = STOP_SVC;
	
	return;
}
