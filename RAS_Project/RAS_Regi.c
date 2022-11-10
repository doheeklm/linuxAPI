/* RAS_Regi.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

int REGI_Init()
{
	int nRC = 0;

	nRC = TAP_Registry_udp_open( g_tEnv.szRegiIp, g_tEnv.nRegiPort, '0', REGI_SYS_ID );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "TAP_Registry_udp_open fail <%d>", nRC );
		return RAS_rErrRegiInit;
	}

	nRC = TAP_Registry_udp_manager_check_alive( REGI_SYS_ID );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "TAP_Registry_udp_manager_check_alive fail <%d>", nRC );
		return RAS_rErrRegiInit;
	}

	return RAS_rOK;
}
