/* RAS_Main.c */
#include "RAS_Inc.h"

int g_nDoSvcFlag = DO_SVC_START;
int g_nAlarmStatus = 0;

Env_t g_tEnv;

mpipc_t *g_ptMpipc = NULL;
oammmc_t *g_ptOammmc = NULL;

THREAD_t g_tThread[WORKER_THR_CNT];

static void ALL_Close();
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
		goto _exit_ras;	
	}

	//TODO need DB to process MMC Handler message
	nRC = MMC_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "MMC_Init fail <%d>", nRC );
		goto _exit_ras;
	}

	nRC = REGI_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "REGI_Init fail <%d>", nRC );
		goto _exit_ras;
	}

	//TODO need DALConn, DB, Pstmt to count Users
	nRC = ALARM_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "ALARM_Init fail <%d>", nRC );
		goto _exit_ras;
	}

	nRC = STAT_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "STAT_Init fail <%d>", nRC );
		goto _exit_ras;
	}

	nRC = THREAD_Init();
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "THREAD_Init fail <%d>", nRC );
		goto _exit_ras;
	}

	while ( g_nDoSvcFlag )
	{
		mpthr_sleep_msec(5000);

		printf( "%d\n", g_nDoSvcFlag );
	}

	printf( "end while loop\n" );

_exit_ras:
	ALL_Close();

	return 0;
}

static void ALL_Close()
{
	//thread 종료
	stgen_close();
	//alarm 삭제?
	MMC_Destroy( g_ptOammmc );
	IPC_Destroy( g_ptMpipc );
	MPGLOG_DESTROY();
}

static void SignalHandler( int nSigno )
{
	MPGLOG_DBG( "Signal<%d>", nSigno );
	
	g_nDoSvcFlag = DO_SVC_STOP;
	
	return;
}
