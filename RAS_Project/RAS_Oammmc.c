/* RAS_Oammmc.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

int OAMMMC_Init( mpipc_t *ptMpipc, oammmc_t *ptOammmc )
{
	CHECK_PARAM_RC( ptMpipc );

	int nRC = 0;

	ptOammmc = oammmc_init( MODULE_NAME );
	if ( NULL == ptOammmc )
	{
		LOG_ERR_F( "oammmc_init fail" );
		return RAS_rErrFail;
	}

	if ( 1 == g_tEnv.nMmtEnable )
	{
		nRC = oammmc_bind_mmt( ptOammmc, MODULE_NAME, g_tEnv.nMmtPort, g_tEnv.nMmtConnMax, g_tEnv.nMmtLocalOnly, g_tEnv.nMmtIsQuiet );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_bind_mmt fail <%d>", nRC );
			return RAS_rErrFail;
		}

		LOG_DBG_F( "mmt_enabled" );
	}
	
	if ( 1 == g_tEnv.nMmlEnable )
	{
		nRC = oammmc_bind_mml( ptOammmc, mpipc_tap_ipc(ptMpipc), oammmc_name(ptOammmc) );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "oammmc_bind_mml fail <%d>", nRC );
			return RAS_rErrFail;
		}

		LOG_DBG_F( "mml_enabled" );
	}

	if ( 0 == g_tEnv.nMmtEnable && 0 == g_tEnv.nMmlEnable )
	{
		LOG_ERR_F( "all oammmc channels are disabled" );
		return RAS_rErrFail;
	}

	nRC = oammmc_regi_cmd( ptOammmc, tCommand, NULL );
	if ( 0 > nRC )
	{
		
	}

	return RAS_rOK;
}
