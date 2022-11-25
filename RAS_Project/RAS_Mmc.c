/* RAS_Mmc.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;
extern mpipc_t *g_ptMpipc;
extern oammmc_t *g_ptOammmc;

int MMC_Init()
{
	int nRC = 0;

	g_ptOammmc = oammmc_init( PROCESS_NAME );
	if ( NULL == g_ptOammmc )
	{
		LOG_ERR_F( "oammmc_init fail" );
		return RAS_rErrMmcInit;
	}

	/*
	 *	Bind MMT Channel
	 *	MMT: Main Machine Terminal
	 *	GUI를 통하지 않고 MMC 수행 블록에 직접 연동하여 MMC 수행
	 *	TCP로 통신하며, 포트 번호를 지정하지 않을 경우 UDS로 통신함
	 */
	if ( 1 == g_tEnv.nMmtEnable )
	{
		nRC = oammmc_bind_mmt( g_ptOammmc, PROCESS_NAME,
				g_tEnv.nMmtPort, g_tEnv.nMmtConnMax, g_tEnv.nMmtLocalOnly, g_tEnv.nMmtIsQuiet );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_bind_mmt fail <%d>", nRC );
			return RAS_rErrMmcInit;
		}

		LOG_DBG_F( "oammmc_bind_mmt success" );
	}

	/*
	 *	Bind MML Channel
	 *	MML: Man Machine Language
	 *	기존 Legacy OAM에서 지원하던 타입
	 *	MMC 수행 블록까지 구조체 형식으로 전송
	 */	
	if ( 1 == g_tEnv.nMmlEnable )
	{
		nRC = oammmc_bind_mml( g_ptOammmc, mpipc_tap_ipc(g_ptMpipc), oammmc_name(g_ptOammmc) );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_bind_mml fail <%d>", nRC );
			return RAS_rErrMmcInit;
		}

		LOG_DBG_F( "oammmc_bind_mml success" );
	}

	if ( 0 == g_tEnv.nMmtEnable && 0 == g_tEnv.nMmlEnable )
	{
		LOG_ERR_F( "all oammmc channels are disabled" );
		return RAS_rErrMmcInit;
	}

	/*
	 *	Register Command
	 */
	nRC = MMCHDL_IP_Init( g_ptOammmc );
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrMmcInit;
	}
	
	nRC = MMCHDL_TRC_Init( g_ptOammmc );
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrMmcInit;
	}

	nRC = MMCHDL_INFO_Init( g_ptOammmc );
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrMmcInit;
	}
	
	nRC = mpipc_regi_hdlr( g_ptMpipc, oammmc_mpipc_hdlr, g_ptOammmc );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpipc_regi_hdlr fail <%d>", nRC );
		return RAS_rErrMmcInit;
	}

	nRC = oammmc_start( g_ptOammmc );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "oammmc_start fail <%d>", nRC );
		return RAS_rErrMmcInit;
	}

	return RAS_rOK;
}

void MMC_Destroy()
{
	int nRC = 0;
	
	if ( NULL != g_ptOammmc )
	{
		nRC = oammmc_stop( g_ptOammmc );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_stop fail <%d>", nRC );
		}
		else
		{
			LOG_DBG_F( "oammmc_stop success" );
		}
		oammmc_destroy( g_ptOammmc );
		LOG_DBG_F( "oammmc_destroy success" );
	}

	return;
}
