/* FW_4-3/FW_Mmc.c */
#include "FW_Inc.h"

static oammmc_arg_info_t atArgsAdd[] =
{
	{ 1, NAME, "<Name>", OAMMMC_STR, ENUM_NAME, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 2, POSITION, "<Position>", OAMMMC_STR, ENUM_POSITION, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, TEAM, "<Team>", OAMMMC_STR, ENUM_TEAM, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, PHONE, "<Phone>", OAMMMC_STR, ENUM_PHONE, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgsDis[] =
{
	{ 1, ID, "<Id>", OAMMMC_INT, ENUM_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgsChg[] =
{
	{ 1, ID, "<Id>", OAMMMC_INT, ENUM_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 2, NAME, "<Name>", OAMMMC_STR, ENUM_NAME, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, POSITION, "<Position>", OAMMMC_STR, ENUM_POSITION, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, TEAM, "<Team>", OAMMMC_STR, ENUM_TEAM, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 5, PHONE, "<Phone>", OAMMMC_STR, ENUM_PHONE, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgsDel[] =
{
	{ 1, ID, "<Id>", OAMMMC_INT, ENUM_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static int MMC_Handler_Add( oammmc_t *ptMmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	ptCmd = ptCmd;
	ptUarg = ptUarg; 

	int i = 0;
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( i = 0; i < nArg; ++i )
	{
		ptArg = &ptArgList[i];

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				pszName = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 2:
			{
				pszPosition = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 3:
			{
				pszTeam = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 4:
			{
				pszPhone = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}
	}

	oammmc_out( ptMmc, "Name = %s", pszName );
	oammmc_out( ptMmc, "Position = %s", pszPosition );
	oammmc_out( ptMmc, "Team = %s", pszTeam );
	oammmc_out( ptMmc, "Phone = %s", pszPhone );

	return SUCCESS;
};

static int MMC_Handler_Dis( oammmc_t *ptMmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	ptCmd = ptCmd;
	ptUarg = ptUarg; 

	int nId = 0;

	if ( 0 == nArg ) //All
	{
		oammmc_out( ptMmc, "Display All" );
	}
	else if ( 1 == nArg ) //One
	{
		oammmc_out( ptMmc, "Display One" );

		nId = OAMMMC_VAL_INT( &ptArgList[0] );
	}

	oammmc_out( ptMmc, "Id = %d", nId );

	return SUCCESS;
};

static int MMC_Handler_Chg( oammmc_t *ptMmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	ptCmd = ptCmd;
	ptUarg = ptUarg;

	int i = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( i = 0; i < nArg; ++i )
	{
		ptArg = &ptArgList[i];

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				nId = OAMMMC_VAL_INT( ptArg );
			}
				break;
			case 2:
			{
				pszName = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 3:
			{
				pszPosition = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 4:
			{
				pszTeam = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 5:
			{
				pszPhone = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}
	}

	oammmc_out( ptMmc, "Id = %d", nId );
	oammmc_out( ptMmc, "Name = %s", pszName );
	oammmc_out( ptMmc, "Position = %s", pszPosition );
	oammmc_out( ptMmc, "Team = %s", pszTeam );
	oammmc_out( ptMmc, "Phone = %s", pszPhone );

	return SUCCESS;
};

static int MMC_Handler_Del( oammmc_t *ptMmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	ptCmd = ptCmd;
	ptUarg = ptUarg; 

	int nId = 0;

	if ( 1 == nArg )
	{
		nId = OAMMMC_VAL_INT( &ptArgList[0] );
	}

	oammmc_out( ptMmc, "Id = %d", nId );

	return SUCCESS;
};

static oammmc_cmd_t tCommandTable[] =
{
	{ 1, "add-emp-info", MSG_ID_ADD, MMC_Handler_Add, 4, 4, atArgsAdd, "Add Employee Info" },
	{ 2, "dis-emp-info", MSG_ID_DIS, MMC_Handler_Dis, 1, 1, atArgsDis, "Display Employee Info" },
	{ 3, "chg-emp-info", MSG_ID_CHG, MMC_Handler_Chg, 1, 5, atArgsChg, "Change Employee Info" },
	{ 4, "del-emp-info", MSG_ID_DEL, MMC_Handler_Del, 1, 1, atArgsDel, "Delete Employee Info" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMC_Init( char *pszModule, oammmc_t *ptMmc, iipc_ds_t *ptIpc )
{
	int nRC = 0;
	int nMmtEnable = 0;
	int nMmlEnable = 0;

	/*
	 *	Init
	 */
	ptMmc = oammmc_init( pszModule );
	if ( NULL == ptMmc )
	{
		MPGLOG_ERR( "%s:: oammmc_init fail", __func__ );
		return MMC_FAIL;
	}
	
	/*
	 *	Bind MMT channel
	 */
	nRC = mpconf_get_int( "", PROCESS_INI, pszModule, MMT_ENABLE, &nMmtEnable, 1, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
		return MPCONF_FAIL;
	}

	if ( nMmtEnable == 1 )
	{
		int nMmtPort = 0;
		int nMmtConnMax = 0;
		int nMmtLocalOnly = 0;
		int nMmtIsQuiet = 0;
		char szBuf[MPCONF_STR_MAX];
		memset( szBuf, 0x00, sizeof(szBuf) );

		nRC = mpconf_get_int( NULL, PROCESS_INI, pszModule, MMT_PORT, &nMmtPort, 1, 0 );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_get_int( NULL, PROCESS_INI, pszModule, MMT_CONN_MAX, &nMmtConnMax, 1, 0 );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_get_str( NULL, PROCESS_INI, pszModule, MMT_LOCAL_ONLY, szBuf, sizeof(szBuf), "True" );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str fail", __func__ );
			return MPCONF_FAIL;
		}

		if ( strcmp(szBuf, "True") == 0 )
		{
			nMmtLocalOnly = 1;
		}
		else if ( strcmp(szBuf, "False") == 0 )
		{
			nMmtLocalOnly = 0;
		}

		memset( szBuf, 0x00, sizeof(szBuf) );
		nRC = mpconf_get_str( NULL, PROCESS_INI, pszModule, MMT_IS_QUIET, szBuf, sizeof(szBuf), "False" );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str fail", __func__ );
			return MPCONF_FAIL;
		}

		if ( strcmp(szBuf, "True") == 0 )
		{
			nMmtIsQuiet = 1;
		}
		else if ( strcmp(szBuf, "False") == 0 )
		{
			nMmtIsQuiet = 0;
		}

		nRC = oammmc_bind_mmt( ptMmc, pszModule, nMmtPort, nMmtConnMax, nMmtLocalOnly, nMmtIsQuiet );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_bind_mmt=%d", __func__, nRC );
			return MMC_FAIL;
		}
		
		MPGLOG_SVC( "mmt_enabled. name=%s port=%d conn_max=%d local_only=%d is_quiet=%d",
				pszModule, nMmtPort, nMmtConnMax, nMmtLocalOnly, nMmtIsQuiet );
	}

	/*
	 *	Bind MML Channel
	 */
	nRC = mpconf_get_int( "", PROCESS_INI, pszModule, MML_ENABLE, &nMmlEnable, 1, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
		return MPCONF_FAIL;
	}

	if ( nMmlEnable == 1 )
	{
		nRC = oammmc_bind_mml( ptMmc, ptIpc, oammmc_name(ptMmc) );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_bind_mml fail=%d", __func__, nRC );
			return MMC_FAIL;
		}

		MPGLOG_SVC( "mml_enabled. name=%s", pszModule );
	}

	/*
	 *	Command table
	 */
	nRC = oammmc_regi_cmd( ptMmc, tCommandTable, NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_regi_cmd fail=%d", __func__, nRC );
		return MMC_FAIL;
	}

	/*
	 *	Start
	 */
	nRC = oammmc_start( ptMmc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_start=%d", __func__, nRC );
		return MMC_FAIL;
	}

	return SUCCESS;
}

void MMC_Destroy( oammmc_t *ptMmc )
{
	int nRC = 0;
	
	if ( NULL != ptMmc )
	{
		nRC = oammmc_stop( ptMmc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_stop fail=%d", __func__, nRC );
		}
		oammmc_destroy( ptMmc );
	}

	return;
}
