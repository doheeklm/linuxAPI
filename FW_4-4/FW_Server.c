/* FW_Server.c */
#include "FW_Header.h"

int g_nFlag = FLAG_RUN;

DAL_CONN *g_ptDalConn = NULL;
DAL_PSTMT *g_ptPstmtInsert = NULL;
DAL_PSTMT *g_ptPstmtSelectAll = NULL;
DAL_PSTMT *g_ptPstmtDelete = NULL;
DAL_PSTMT *g_ptPstmtNumTuples = NULL;

mpipc_t *ptMpipc = NULL;

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRC = 0;
	int nPort = 0;
	char szIp[SIZE_IP + 1];
	memset( szIp, 0x00, sizeof(szIp) );

	oammmc_t *ptOammmc = NULL;
	
	/*
	 *	MPGLOG
	 */
	nRC = MPGLOG_INIT( PROCNAME_SERVER, NULL, LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG, LOG_LEVEL_DBG );
	if ( 0 > nRC )
	{
		printf( "%s MPGLOG_INIT() fail\n", __func__ );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Init MPIPC
	 */
	ptMpipc = mpipc_init( PROCNAME_SERVER, 100, 300 );
	if ( NULL == ptMpipc )
	{
		MPGLOG_ERR( "%s:: mpipc_init() fail", __func__ );
		exit( EXIT_FAILURE );
	}
	//ipc_check_interval: 100 (mpipc thread가 ipc message를 확인하는 주기)
	//grace_time: 300 (mpipc 종료시 대기시간)

	/*
	 *	Register IPC Handler
	 *	프로세스에 IPC Message가 수신되었을때 호출되는 콜백함수를 등록한다.
	 */
	nRC = mpipc_regi_hdlr( ptMpipc, IPC_Handler, NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpipc_regi_hdlr() fail=%d", __func__, nRC );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Start MPIPC
	 *	API 호출 후부터 IPC Message를 수신하여 Handler를 호출한다.
	 */
	nRC = mpipc_start( ptMpipc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpipc_start() fail=%d", __func__, nRC );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	MMC
	 */
	nRC = MMC_Init( PROCNAME_SERVER, ptOammmc, ptMpipc );
	if ( RC_SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: MMC_Init() fail=%d", __func__, nRC );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	DAL Connect
	 */
	g_ptDalConn = dalConnect( NULL );
	if ( NULL == g_ptDalConn )
	{
		MPGLOG_ERR( "%s:: dalConnect() fail=%d", __func__, nRC );
		goto _exit_failure;
	}

	/*
	 *	Prepared Statement
	 */
	nRC = PSTMT_Init();	
	if ( RC_SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: PSTMT_Init() fail=%d", __func__, nRC );
		nRC = dalDisconnect( g_ptDalConn );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		}
		goto _exit_failure;
	}

	/*
	 * 	MPCONF
	 */
	nRC = UTIL_GetConfig( szIp, &nPort, sizeof(szIp) );
	if ( RC_SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: UTIL_GetConfig() fail", __func__ );
		goto _exit_failure;
	}

	/*
	 *	Registry
	 */
	nRC = TAP_Registry_udp_open( szIp, nPort, '0', REGI_MAN_SYSTEM_ID );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_open() fail=%d", __func__, nRC );
		goto _exit_failure;
	}

	/*
	 *	Trace
	 */




	while ( FLAG_RUN == g_nFlag )
	{
		mpthr_sleep_msec(500);
	}

	PSTMT_Destroy();
	
	nRC = dalDisconnect( g_ptDalConn );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
	}
	
	MMC_Destroy( ptOammmc );
	IPC_Destroy( ptMpipc );

	exit( EXIT_SUCCESS );

_exit_failure:

	MMC_Destroy( ptOammmc );
	IPC_Destroy( ptMpipc );
	
	exit( EXIT_FAILURE );	
}

void SignalHandler( int nSigno )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal = %d", nSigno );

	exit( EXIT_FAILURE );
}
