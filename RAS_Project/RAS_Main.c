/* RAS_Main.c */
#include "RAS_Inc.h"

int g_nFlag = SIGFLAG_RUN;
Env_t g_tEnv;
mpipc_t *g_ptMpipc = NULL;

void SignalHandler( int nSigno );

int main( void )
{
	tb_signal( SIGHUP, SignalHandler );
	tb_signal( SIGINT, SignalHandler );
	tb_signal( SIGQUIT, SignalHandler );
	tb_signal( SIGKILL, SignalHandler );
	tb_signal( SIGTERM, SignalHandler );

	int nRC = 0;
	oammmc_t *ptOammmc = NULL;

	/*
	 *	Get Config
	 */
	nRC = CONF_Init();
	if ( RAS_rOK != nRC )
	{
		printf( "CONF_Init fail <%d>\n", nRC );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Init LOG
	 */
	nRC = MPGLOG_INIT( MODULE_NAME, NULL, g_tEnv.nLogMode, g_tEnv.nLogLevel );
	if ( 0 > nRC )
	{
		printf( "MPGLOG_INIT fail <%d>\n", nRC );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Init IPC
	 */	
	nRC = IPC_Init( g_ptMpipc );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "IPC_Init fail <%d>", nRC );
		IPC_Destroy( g_ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Init MMC
	 */
	nRC = OAMMMC_Init( g_ptMpipc, ptOammmc );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "OAMMMC_Init fail <%d>", nRC )
		goto end_of_function;
	}

end_of_function:
	IPC_Destroy( g_ptMpipc );

	return 0;
}

void SignalHandler( int nSigno )
{
	g_nFlag = SIGFLAG_STOP;

	MPGLOG_DBG( "Signal<%d>", nSigno );
}
