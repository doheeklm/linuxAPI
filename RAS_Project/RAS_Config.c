/* RAS_Config.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

static int CONF_GetConnConf();
static int CONF_GetLogConf();
static int CONF_GetMmcConf();

int CONF_Init()
{
	CONF_GetConnConf();
	CONF_GetLogConf();
	CONF_GetMmcConf();

	return RAS_rOK;
}

static int CONF_GetConnConf()
{
	int nRC = 0;

	nRC = mpconf_get_str( NULL, CONFIG_PATH, CONN_SECTION, "IP", g_tEnv.szIp, sizeof( g_tEnv.szIp ), NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_str() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, CONN_SECTION, "PORT", &(g_tEnv.nPort), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	return RAS_rOK;
}

static int CONF_GetLogConf()
{
	int nRC = 0;

	nRC = mpconf_get_int( NULL, CONFIG_PATH, LOG_SECTION, "log_mode", &(g_tEnv.nLogMode), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, LOG_SECTION, "log_level", &(g_tEnv.nLogLevel), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}


	return RAS_rOK;
}

static int CONF_GetMmcConf()
{
	int nRC = 0;

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_enable", &(g_tEnv.nMmtEnable), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_port", &(g_tEnv.nMmtPort), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_conn_max", &(g_tEnv.nMmtConnMax), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_local_only", &(g_tEnv.nMmtLocalOnly), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mmt_is_quiet", &(g_tEnv.nMmtIsQuiet), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	nRC = mpconf_get_int( NULL, CONFIG_PATH, MMC_SECTION, "mml_enable", &(g_tEnv.nMmlEnable), 0, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "mpconf_get_int() fail" );
		return RAS_rErrGetValue;
	}

	return RAS_rOK;
}
