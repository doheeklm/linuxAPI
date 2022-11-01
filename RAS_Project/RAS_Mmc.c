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
	 */
	if ( 1 == g_tEnv.nMmtEnable )
	{
		nRC = oammmc_bind_mmt( g_ptOammmc, PROCESS_NAME, g_tEnv.nMmtPort, g_tEnv.nMmtConnMax, g_tEnv.nMmtLocalOnly, g_tEnv.nMmtIsQuiet );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_bind_mmt fail <%d>", nRC );
			return RAS_rErrMmcInit;
		}

		LOG_DBG_F( "mmt_enabled" );
	}

	/*
	 *	Bind MML Channel
	 */	
	if ( 1 == g_tEnv.nMmlEnable )
	{
		nRC = oammmc_bind_mml( g_ptOammmc, mpipc_tap_ipc(g_ptMpipc), oammmc_name(g_ptOammmc) );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_bind_mml fail <%d>", nRC );
			return RAS_rErrMmcInit;
		}

		LOG_DBG_F( "mml_enabled" );
	}

	if ( 0 == g_tEnv.nMmtEnable && 0 == g_tEnv.nMmlEnable )
	{
		LOG_ERR_F( "all oammmc channels are disabled" );
		return RAS_rErrMmcInit;
	}

	/*
	 *	Register Command
	 */
	nRC = MMCHDL_CLIIP_Init( g_ptOammmc );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "MMCHDL_CLIIP_Init fail <%d>", nRC );
		return RAS_rErrMmcInit;
	}
	
	nRC = MMCHDL_CLIIPTRC_Init( g_ptOammmc );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "MMCHDL_CLIIPTRC_Init fail <%d>", nRC );
		return RAS_rErrMmcInit;
	}

	nRC = MMCHDL_USRINFO_Init( g_ptOammmc );
	if ( RAS_rOK != nRC )
	{
		LOG_ERR_F( "MMCHDL_USRINFO_Init fail <%d>", nRC );
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

void MMC_Destroy( oammmc_t *ptOammmc )
{
	int nRC = 0;
	
	if ( NULL != ptOammmc )
	{
		nRC = oammmc_stop( ptOammmc );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_stop fail <%d>", nRC );
		}
		LOG_DBG_F( "oammmc_stop" );
		oammmc_destroy( ptOammmc );
		LOG_DBG_F( "oammmc_destroy" );
	}

	return;
}
