/* RAS_Config.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

static int CONFIG_GetConnConf();
static int CONFIG_GetLogConf();
static int CONFIG_GetMmcConf();

int CONFIG_Init()
{
	int nRC = 0;
	
	nRC = CONFIG_GetConnConf();
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrConfInit;
	}
	
	nRC = CONFIG_GetLogConf();
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrConfInit;
	}

	nRC = CONFIG_GetMmcConf();
	if ( RAS_rOK != nRC )
	{
		return RAS_rErrConfInit;
	}

	LOG_SVC_F( "IP(%s) PORT(%d) REGI_LBPORT(%d)\n"
			"log_level(%d) log_mode(%d)\n"
			"mmt_enable(%d) mmt_port(%d) mmt_conn_max(%d)\n"
			"mmt_local_only(%d) mmt_is_quiet(%d) mml_enable(%d)",
			g_tEnv.szIp, g_tEnv.nPort, g_tEnv.nRegiLbPort,
			g_tEnv.nLogMode, g_tEnv.nLogLevel,
			g_tEnv.nMmtEnable, g_tEnv.nMmtPort, g_tEnv.nMmtConnMax,
			g_tEnv.nMmtLocalOnly, g_tEnv.nMmtIsQuiet, g_tEnv.nMmlEnable );

	return RAS_rOK;
}

static int CONFIG_GetConnConf()
{
	int nRC = 0;

	nRC = mpconf_get_str( NULL, CONFIG_PATH, CONN_SECTION, "IP", g_tEnv.szIp, sizeof(g_tEnv.szIp), NULL );
	if ( 0 > nRC )
	{
		printf( "mpconf_get_str() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, CONN_SECTION, "PORT", &(g_tEnv.nPort), 0, 0 );
	if ( 0 > nRC )
	{
		printf( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, CONN_SECTION, "REGI_LBPORT", &(g_tEnv.nRegiLbPort), 0, 0 );
	if ( 0 > nRC )
	{
		printf( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	return RAS_rOK;
}

static int CONFIG_GetLogConf()
{
	int nRC = 0;

	nRC = mpconf_get_int( NULL, CONFIG_PATH, LOG_SECTION, "log_mode", &(g_tEnv.nLogMode), 0, 0 );
	if ( 0 > nRC )
	{
		printf( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, LOG_SECTION, "log_level", &(g_tEnv.nLogLevel), 0, 0 );
	if ( 0 > nRC )
	{
		printf( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	return RAS_rOK;
}

static int CONFIG_GetMmcConf()
{
	int nRC = 0;

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_enable", &(g_tEnv.nMmtEnable), 0, 0 );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_port", &(g_tEnv.nMmtPort), 0, 0 );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_conn_max", &(g_tEnv.nMmtConnMax), 0, 0 );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_local_only", &(g_tEnv.nMmtLocalOnly), 0, 0 );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_is_quiet", &(g_tEnv.nMmtIsQuiet), 0, 0 );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mml_enable", &(g_tEnv.nMmlEnable), 0, 0 );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpconf_get_int() fail <%d>", nRC );
		return RAS_rErrGetConfValue;
	}

	return RAS_rOK;
}
