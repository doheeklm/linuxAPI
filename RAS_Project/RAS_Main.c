/* RAS_Main.c */
#include "RAS_Inc.h"

int g_nSvcFlag = START_SVC;
int g_nThreadIndex = 0; 
int g_nAlarmStatus = 0; //Thread

Env_t g_tEnv;
DB_t g_tDBMain;
mpipc_t *g_ptMpipc = NULL;
oammmc_t *g_ptOammmc = NULL;
THREAD_t g_tThread[THREAD_CNT];

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
	int nCreateFlag = START_CREATE;

	memset( &g_tDBMain, 0x00, sizeof(g_tDBMain) );

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
		LOG_ERR_F( "IPC_Init fail <%d>", nRC );
		goto _exit_main;	
	}
	
	nRC = DB_Init( &(g_tDBMain.ptDBConn) );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "DB_Init fail <%d>", nRC );
		goto _exit_main;
	}	
	
	nRC = DB_InitPreparedStatement( &g_tDBMain );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "DB_InitPreparedStatement fail <%d>", nRC );
		goto _exit_main;
	}	
	
	nRC = REGI_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "REGI_Init fail <%d>", nRC );
		goto _exit_main;
	}

	nRC = MMC_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "MMC_Init fail <%d>", nRC );
		goto _exit_main;
	}
	
	nRC = ALARM_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "ALARM_Init fail <%d>", nRC );
		goto _exit_main;
	}

	nRC = STAT_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "STAT_Init fail <%d>", nRC );
		goto _exit_main;
	}

	nRC = THREAD_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "THREAD_Init fail <%d>", nRC );
		goto _exit_main;
	}

	while ( g_nSvcFlag )
	{
		if ( START_CREATE == nCreateFlag )
		{
			nRC = SOCKET_Init( &nListenFd );
			if ( RAS_rOK != nRC )
			{
				LOG_ERR_F( "SOCKET_Init fail <%d>", nRC );
				goto _exit_main;
			}

			LOG_SVC_F( "SOCKET_Init nListenFd=%d", nListenFd );

			nRC = EVENT_Init( &nEpollFd, nListenFd );
			if ( RAS_rOK != nRC )
			{
				LOG_ERR_F( "EVENT_Init fail <%d>", nRC );
				goto _exit_main;
			}

			LOG_SVC_F( "EVENT_Init nEpollFd=%d", nEpollFd );
		}

		nRC = EVENT_WaitAndAccept( nEpollFd, nListenFd );
		if ( RAS_rErrEventRecreateYes == nRC )
		{
			nCreateFlag = START_CREATE;
		}
		else
		{
			nCreateFlag = STOP_CREATE;
		}
	}

	printf( "end while loop\n" );

	for ( nIndex = 0; nIndex < THREAD_CNT; nIndex++ )
	{
		nRC = pthread_join( g_tThread[nIndex].nThreadId, NULL );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_join fail <%d>", nRC );
		}
		LOG_SVC_F( "pthread_join <%d>", nRC );
	}

_exit_main:
	//epoll
	//socket
	//thread
	stgen_close();
	//alarm
	MMC_Destroy();
	//regi
	DB_Close( &g_tDBMain );
	IPC_Destroy();
	MPGLOG_DESTROY();
	
	return 0;
}

static void SignalHandler( int nSigno )
{
	MPGLOG_DBG( "Signal<%d>", nSigno );
	
	g_nSvcFlag = STOP_SVC;
	
	return;
}
