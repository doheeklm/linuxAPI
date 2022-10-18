/* FW_Mmc.c */
#include "FW_Header.h"

/*
 *	EMPL_INFO
 */
oammmc_arg_info_t atArgsAdd[] =
{
	{ 1, EMPL_NAME, "<EMPL_NAME>", OAMMMC_STR, EMPL_NAME_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 2, EMPL_POSITION, "<EMPL_POSITION>", OAMMMC_STR, EMPL_POSITION_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, EMPL_TEAM, "<EMPL_TEAM>", OAMMMC_STR, EMPL_TEAM_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, EMPL_PHONE, "<EMPL_PHONE>", OAMMMC_STR, EMPL_PHONE_ID, 1, 11, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsDis[] =
{
	{ 1, EMPL_ID, "<EMPL_ID>", OAMMMC_INT, EMPL_ID_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 2, EMPL_NAME, "<EMPL_NAME>", OAMMMC_STR, EMPL_NAME_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, EMPL_POSITION, "<EMPL_POSITION>", OAMMMC_STR, EMPL_POSITION_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, EMPL_TEAM, "<EMPL_TEAM>", OAMMMC_STR, EMPL_TEAM_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 5, EMPL_PHONE, "<EMPL_PHONE>", OAMMMC_STR, EMPL_PHONE_ID, 1, 11, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsChg[] =
{
	{ 1, EMPL_ID, "<EMPL_ID>", OAMMMC_INT, EMPL_ID_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 2, EMPL_NAME, "<EMPL_NAME>", OAMMMC_STR, EMPL_NAME_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 3, EMPL_POSITION, "<EMPL_POSITION>", OAMMMC_STR, EMPL_POSITION_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 4, EMPL_TEAM, "<EMPL_TEAM>", OAMMMC_STR, EMPL_TEAM_ID, 1, 32, NULL, "Process Name to (De)Activate" },
	{ 5, EMPL_PHONE, "<EMPL_PHONE>", OAMMMC_STR, EMPL_PHONE_ID, 1, 11, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsDel[] =
{
	{ 1, EMPL_ID, "<EMPL_ID>", OAMMMC_INT, EMPL_ID_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_cmd_t tCommandInfo[] =
{
	{ 1, MMC_ADD_INFO, MMC_ADD_INFO_ID, MMC_Handler_Add, 4, 4, atArgsAdd, "Cmd_Desc: Add Employee Info" },
	{ 2, MMC_DIS_INFO, MMC_DIS_INFO_ID, MMC_Handler_Dis, 0, 5, atArgsDis, "Cmd_Desc: Display Employee Info" },
	{ 3, MMC_CHG_INFO, MMC_CHG_INFO_ID, MMC_Handler_Chg, 1, 5, atArgsChg, "Cmd_Desc: Change Employee Info" },
	{ 4, MMC_DEL_INFO, MMC_DEL_INFO_ID, MMC_Handler_Del, 1, 1, atArgsDel, "Cmd_Desc: Delete Employee Info" }, 
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

/*
 *	EMPL_TRC
 */
oammmc_arg_info_t atArgsAddTrace[] =
{
	{ 1, EMPL_TRC_KEY, "<EMPL_TRC_KEY>", OAMMMC_INT, EMPL_TRC_KEY_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 2, PERIOD_MIN, "<PERIOD_MIN>", OAMMMC_INT, PERIOD_MIN_ID, 1, 43200, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsDisTrace[] =
{
	{ 1, EMPL_TRC_KEY, "<EMPL_TRC_KEY>", OAMMMC_INT, EMPL_TRC_KEY_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_arg_info_t atArgsDelTrace[] =
{
	{ 1, EMPL_TRC_KEY, "<EMPL_TRC_KEY>", OAMMMC_INT, EMPL_TRC_KEY_ID, 1, 1000, NULL, "Process Name to (De)Activate" },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

oammmc_cmd_t tCommandTrace[] =
{
	{ 1, MMC_ADD_TRC, MMC_ADD_TRC_ID, MMC_Handler_AddTrace, 1, 2, atArgsAddTrace, "Cmd_Desc: Add Employee Trace" },
	{ 2, MMC_DIS_TRC, MMC_DIS_TRC_ID, MMC_Handler_DisTrace, 0, 1, atArgsDisTrace, "Cmd_Desc: Display Employee Trace" },
	{ 3, MMC_DEL_TRC, MMC_DEL_TRC_ID, MMC_Handler_DelTrace, 1, 1, atArgsDelTrace, "Cmd_Desc: Delete Employee Trace" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMC_Init( char *pszModule, oammmc_t *ptOammmc, mpipc_t *ptMpipc )
{
	if ( NULL == pszModule )
	{
		MPGLOG_ERR( "%s:: pszModule NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptMpipc )
	{
		MPGLOG_ERR( "%s:: ptMpipc NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;
	int nMmtEnable = 0;
	int nMmlEnable = 0;

	ptOammmc = oammmc_init( pszModule );
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: oammmc_init fail", __func__ );
		return OAMMMC_FAIL;
	}

	nRC = mpconf_get_int( "", PROCESS_INI, pszModule, MMT_ENABLE, &nMmtEnable, 1, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
		return MPCONF_FAIL;
	}

	/*
	 *	설정파일에 mmt_enable=1 일경우 mmt를 사용하도록 구성한다.
	 */
	if ( 1 == nMmtEnable )
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

		nRC = oammmc_bind_mmt( ptOammmc, pszModule, nMmtPort, nMmtConnMax, nMmtLocalOnly, nMmtIsQuiet );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_bind_mmt=%d", __func__, nRC );
			return OAMMMC_FAIL;
		}

		MPGLOG_SVC( "mmt_enabled. name=%s port=%d conn_max=%d local_only=%d is_quiet=%d",
				pszModule, nMmtPort, nMmtConnMax, nMmtLocalOnly, nMmtIsQuiet );
	}
	
	nRC = mpconf_get_int( "", PROCESS_INI, pszModule, MML_ENABLE, &nMmlEnable, 1, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
		return MPCONF_FAIL;
	}

	/*
	 *	설정파일에 mml_enable=1 일경우 mml을 사용하도록 구성한다.
	 */
	if ( 1 == nMmlEnable )
	{
		nRC = oammmc_bind_mml( ptOammmc, mpipc_tap_ipc(ptMpipc), oammmc_name(ptOammmc) );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_bind_mml fail=%d", __func__, nRC );
			return OAMMMC_FAIL;
		}

		MPGLOG_SVC( "mml_enabled. name=%s", pszModule );
	}
	else if ( 0 == nMmlEnable )
	{
		return SYSTEM_FAIL;
	}

	if ( 0 == nMmtEnable && 0 == nMmlEnable )
	{
		MPGLOG_ERR( "%s:: all oammmc channels are disabled", __func__ );
		return OAMMMC_FAIL;
	}

	/*
	 *	위에 정의한 command을 oammmc에 등록한다.
	 */	
	nRC = oammmc_regi_cmd( ptOammmc, tCommandInfo, NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_regi_cmd() fail=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	nRC = oammmc_regi_cmd( ptOammmc, tCommandTrace, NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_regi_cmd() fail=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	/*
	 *	oammmc handler를 mpipc에 등록하여 mmc가 처리되도록 한다.
	 */
	nRC = mpipc_regi_hdlr( ptMpipc, oammmc_mpipc_hdlr, ptOammmc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpipc_regi_hdlr() fail=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	/*
	 *	oammmc를 시작하여 mmt 및 mml이 처리되도록 한다.
	 */
	nRC = oammmc_start( ptOammmc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_start=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	return RC_SUCCESS;
}

void MMC_Destroy( oammmc_t *ptOammmc )
{
	int nRC = 0;
	
	if ( NULL != ptOammmc )
	{
		nRC = oammmc_stop( ptOammmc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_stop fail=%d", __func__, nRC );
		}
		oammmc_destroy( ptOammmc );
	}

	return;
}

int MMC_Handler_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;
	
	int i = 0;
	int nRC = 0;

	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;
	
	oammmc_arg_t *ptArg = NULL;

	for ( i = 0; i < nArgNum; ++i )
	{
		ptArg = &patArgList[i];

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				pszName = OAMMMC_VAL_STR( ptArg );
				
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_NAME, pszName );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
			case 2:
			{
				pszPosition = OAMMMC_VAL_STR( ptArg );
				
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_POSITION, pszPosition );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
			case 3:
			{
				pszTeam = OAMMMC_VAL_STR( ptArg );
			
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_TEAM, pszTeam );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
			case 4:
			{
				pszPhone = OAMMMC_VAL_STR( ptArg );
		
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_PHONE, pszPhone );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
		}
	}

	nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() = 0", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_EXEC_ZERO;
	}

	oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_NAME, pszName );
	oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_POSITION, pszPosition );
	oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_TEAM, pszTeam );
	oammmc_out( ptOammmc, "%-8s = %-11s\n", TABLE_ATT_PHONE, pszPhone );

	return MMC_HANDLER_SUCCESS;
};

int MMC_Handler_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	//TODO 하나로 function화 하기
	
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;

	int nRC = 0;
	int nId = 0;
	int nTotalCnt = 0;

	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;

	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );

	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;
	
	if ( 0 == nArgNum )
	{
		nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtSelectAll, &ptRes );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
			oammmc_out( ptOammmc, "System Fail" );	
			return DAL_FAIL;
		}
		else if ( 0 == nRC )
		{
			nRC = dalResFree( ptRes );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				oammmc_out( ptOammmc, "System Fail" );
				return DAL_FAIL;
			}
			oammmc_out( ptOammmc, "No Infos in DB" );
			return DAL_EXEC_ZERO;
		}

		oammmc_out( ptOammmc, "%4s %32s %32s %32s %11s\n",
				TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );
		oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );

		for ( ptEntry = dalFetchFirst(ptRes); ptEntry != NULL; ptEntry = dalFetchNext(ptRes) )
		{
			nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;			
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			oammmc_out( ptOammmc, "%4d %32s %32s %32s %11s\n",
						nId, pszName, pszPosition, pszTeam, pszPhone );

			nTotalCnt++;
		}
		
		oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );
		oammmc_out( ptOammmc, "TOTAL_CNT = %d", nTotalCnt );

		return MMC_HANDLER_SUCCESS;
	}
	else if ( 1 == nArgNum )
	{
		oammmc_arg_t *ptArg = NULL;

		ptArg = &patArgList[0];

		switch( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				nId = OAMMMC_VAL_INT( ptArg );

				snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s, %s from %s where %s = %d;",
						TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
						TABLE_ATT_ID, nId );
				szQuery[ strlen(szQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptRes );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "id %d not exist", nId );
					return DAL_EXEC_ZERO;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}

				ptEntry = dalFetchFirst( ptRes );
				if ( NULL != ptEntry )
				{
					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "%-8s = %-4d\n", TABLE_ATT_ID, nId );
					oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_NAME, pszName );
					oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_POSITION, pszPosition );
					oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_TEAM, pszTeam );
					oammmc_out( ptOammmc, "%-8s = %-11s\n", TABLE_ATT_PHONE, pszPhone );
				}
			}
				break;
			case 2:
			{
				pszName = OAMMMC_VAL_STR( ptArg );

				snprintf( szQuery, sizeof(szQuery),	"select %s, %s, %s, %s from %s where %s = '%s';",
						TABLE_ATT_ID, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
						TABLE_ATT_NAME, pszName );
				szQuery[ strlen(szQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptRes );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "name %s not exist", pszName );
					return DAL_EXEC_ZERO;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}

				oammmc_out( ptOammmc, "%4s %32s %32s %32s %11s\n",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );

				for ( ptEntry = dalFetchFirst(ptRes); ptEntry != NULL; ptEntry = dalFetchNext(ptRes) )
				{
					nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "%4d %32s %32s %32s %11s\n",
							nId, pszName, pszPosition, pszTeam, pszPhone );

					nTotalCnt++;
				}

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );
				oammmc_out ( ptOammmc, "TOTAL_CNT = %d", nTotalCnt );
			}
				break;
			case 3:
			{
				pszPosition = OAMMMC_VAL_STR( ptArg );

				snprintf( szQuery, sizeof(szQuery),	"select %s, %s, %s, %s from %s where %s = '%s';",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
						TABLE_ATT_POSITION, pszPosition );
				szQuery[ strlen(szQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptRes );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "position %s not exist", pszPosition );
					return DAL_EXEC_ZERO;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}

				oammmc_out( ptOammmc, "%4s %32s %32s %32s %11s\n",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );

				for ( ptEntry = dalFetchFirst(ptRes); ptEntry != NULL; ptEntry = dalFetchNext(ptRes) )
				{
					nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "%4d %32s %32s %32s %11s\n",
							nId, pszName, pszPosition, pszTeam, pszPhone );

					nTotalCnt++;
				}

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );
				oammmc_out ( ptOammmc, "TOTAL_CNT = %d", nTotalCnt );	
			}
				break;
			case 4:
			{
				pszTeam = OAMMMC_VAL_STR( ptArg );

				snprintf( szQuery, sizeof(szQuery),	"select %s, %s, %s, %s from %s where %s = '%s';",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_PHONE, TABLE_NAME,
						TABLE_ATT_TEAM, pszTeam );
				szQuery[ strlen(szQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptRes );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "team %s not exist", pszTeam );
					return DAL_EXEC_ZERO;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}

				oammmc_out( ptOammmc, "%4s %32s %32s %32s %11s\n",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );

				for ( ptEntry = dalFetchFirst(ptRes); ptEntry != NULL; ptEntry = dalFetchNext(ptRes) )
				{
					nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "%4d %32s %32s %32s %11s\n",
							nId, pszName, pszPosition, pszTeam, pszPhone );

					nTotalCnt++;
				}

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );
				oammmc_out ( ptOammmc, "TOTAL_CNT = %d", nTotalCnt );
			}
				break;
			case 5:
			{
				pszPhone = OAMMMC_VAL_STR( ptArg );

				snprintf( szQuery, sizeof(szQuery),	"select %s, %s, %s, %s from %s where %s = '%s';",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_NAME,
						TABLE_ATT_PHONE, pszPhone );
				szQuery[ strlen(szQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptRes );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "phone %s not exist", pszPhone );
					return DAL_EXEC_ZERO;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}

				oammmc_out( ptOammmc, "%4s %32s %32s %32s %11s\n",
						TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );

				for ( ptEntry = dalFetchFirst(ptRes); ptEntry != NULL; ptEntry = dalFetchNext(ptRes) )
				{
					nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "%4d %32s %32s %32s %11s\n",
							nId, pszName, pszPosition, pszTeam, pszPhone );

					nTotalCnt++;
				}

				oammmc_out( ptOammmc, "------------------------------------------------------------------------------------------------------------------------\n" );
				oammmc_out ( ptOammmc, "TOTAL_CNT = %d", nTotalCnt );
			}
				break;
			default:
				break;
		}//switch
	}//else if
	else
	{
		oammmc_out( ptOammmc, "No more than 1 option" );
		return INPUT_FAIL;
	}

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	return MMC_HANDLER_SUCCESS;

_exit_failure:

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}

	return MMC_HANDLER_FAIL;
};

int MMC_Handler_Chg( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;

	int i = 0;
	int nRC = 0;
	int nId = 0;
	int nCommaFlag = 0;

	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;

	char *pszNewName = NULL;
	char *pszNewPosition = NULL;
	char *pszNewTeam = NULL;
	char *pszNewPhone = NULL;

	char szSelectQuery[256];
	char szUpdateQuery[256];
	char szTemp[256];

	memset( szSelectQuery, 0x00, sizeof(szSelectQuery) );
	memset( szUpdateQuery, 0x00, sizeof(szUpdateQuery) );
	memset( szTemp, 0x00, sizeof(szTemp) );

	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;
	oammmc_arg_t *ptArg = NULL;

	if ( 1 == nArgNum && 1 == OAMMMC_ARG_ID( &patArgList[0] ) )
	{
		oammmc_out( ptOammmc, "Input Option" );
		return INPUT_FAIL;
	}

	for ( i = 0; i < nArgNum; ++i )
	{
		ptArg = &patArgList[i];

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				nId = OAMMMC_VAL_INT( ptArg );

				snprintf( szSelectQuery, sizeof(szSelectQuery), "select * from %s where %s = %d;", TABLE_NAME, TABLE_ATT_ID, nId );
				szSelectQuery[ strlen(szSelectQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szSelectQuery, &ptRes );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptRes );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
					}
					return DAL_EXEC_ZERO;
				}
				else if ( -1 == nRC )
				{
					return DAL_FAIL;
				}

				ptEntry = dalFetchFirst( ptRes );
				if ( NULL != ptEntry )
				{
					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;	
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}
				}

				snprintf( szTemp, sizeof(szTemp), "update %s set ", TABLE_NAME );
				szTemp[ strlen(szTemp) ] = '\0';
				strncat( szUpdateQuery, szTemp, strlen(szTemp) );	
			}
				break;
			case 2:
			{
				pszNewName = OAMMMC_VAL_STR( ptArg );
		
				memset( szTemp, 0x00, sizeof(szTemp) );	
				snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_NAME, pszNewName );
				szTemp[ strlen(szTemp) ] = '\0';
				strncat( szUpdateQuery, szTemp, strlen(szTemp) );

				nCommaFlag = 1;
			}
				break;
			case 3:
			{
				pszNewPosition = OAMMMC_VAL_STR( ptArg );
		
				memset( szTemp, 0x00, sizeof(szTemp) );

				if ( 1 == nCommaFlag )
				{
					snprintf( szTemp, sizeof(szTemp), ", %s = '%s'", TABLE_ATT_POSITION, pszNewPosition );
					szTemp[ strlen(szTemp) ] = '\0';
					strncat( szUpdateQuery, szTemp, strlen(szTemp) );
				}
				else
				{
					snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_POSITION, pszNewPosition );
					szTemp[ strlen(szTemp) ] = '\0';
					strncat( szUpdateQuery, szTemp, strlen(szTemp) );

					nCommaFlag = 1;
				}
			}
				break;
			case 4:
			{
				pszNewTeam = OAMMMC_VAL_STR( ptArg );

				memset( szTemp, 0x00, sizeof(szTemp) );

				if ( 1 == nCommaFlag )
				{
					snprintf( szTemp, sizeof(szTemp), ", %s = '%s'", TABLE_ATT_TEAM, pszNewTeam );
					szTemp[ strlen(szTemp) ] = '\0';
					strncat( szUpdateQuery, szTemp, strlen(szTemp) );
				}
				else
				{
					snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_TEAM, pszNewTeam );
					szTemp[ strlen(szTemp) ] = '\0';
					strncat( szUpdateQuery, szTemp, strlen(szTemp) );

					nCommaFlag = 1;
				}
			}
				break;
			case 5:
			{
				pszNewPhone = OAMMMC_VAL_STR( ptArg );
				
				memset( szTemp, 0x00, sizeof(szTemp) );

				if ( 1 == nCommaFlag )
				{
					snprintf( szTemp, sizeof(szTemp), ", %s = '%s'", TABLE_ATT_PHONE, pszNewPhone );
					szTemp[ strlen(szTemp) ] = '\0';
					strncat( szUpdateQuery, szTemp, strlen(szTemp) );
				}
				else
				{
					snprintf( szTemp, sizeof(szTemp), "%s = '%s'", TABLE_ATT_PHONE, pszNewPhone );
					szTemp[ strlen(szTemp) ] = '\0';
					strncat( szUpdateQuery, szTemp, strlen(szTemp) );

					nCommaFlag = 1;
				}
			}
				break;
		}
	}

	memset( szTemp, 0x00, sizeof(szTemp) );
	snprintf( szTemp, sizeof(szTemp), " where %s = %d;", TABLE_ATT_ID, nId );
	strncat( szUpdateQuery, szTemp, strlen(szTemp) );

	if ( NULL != pszName && NULL != pszNewName &&
		 NULL != pszPosition && NULL != pszNewPosition &&
		 NULL != pszTeam && NULL != pszNewTeam &&
		 NULL != pszPhone && NULL != pszNewPhone )
	{
		if ( strcmp(pszName, pszNewName) == 0 &&
			 strcmp(pszPosition, pszNewPosition) == 0 &&
			 strcmp(pszTeam, pszNewTeam) == 0 &&
			 strcmp(pszPhone, pszNewPhone) == 0 )
		{
			oammmc_out( ptOammmc, "Old Data == New Data\n" ); //단 테이블에 null값이 없어야함
			return INPUT_FAIL;
		}
	}

	nRC = dalExecute( g_ptDalConn, szUpdateQuery, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalExecute() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_DBG( "%s:: id not exist", __func__ );
		oammmc_out( ptOammmc, "id %d not exist", nId );
		return DAL_FAIL;
	}

	oammmc_out( ptOammmc, "======== Old Data ========\n" );
	oammmc_out( ptOammmc, "%-8s = %-4d\n", TABLE_ATT_ID, nId );
	oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_NAME, pszName );
	oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_POSITION, pszPosition );
	oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_TEAM, pszTeam );
	oammmc_out( ptOammmc, "%-8s = %-11s\n", TABLE_ATT_PHONE, pszPhone );
	oammmc_out( ptOammmc, "======== New Data ========\n" );
	oammmc_out( ptOammmc, "%-8s = %-4d\n", TABLE_ATT_ID, nId );

	if ( NULL != pszNewName )
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_NAME, pszNewName );
	else
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_NAME, pszName );
	
	if ( NULL != pszNewPosition )
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_POSITION, pszNewPosition );
	else
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_POSITION, pszPosition );

	if ( NULL != pszNewTeam )
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_TEAM, pszNewTeam );
	else
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_TEAM, pszTeam );

	if ( NULL != pszNewPhone )
		oammmc_out( ptOammmc, "%-8s = %-11s\n", TABLE_ATT_PHONE, pszNewPhone );
	else
		oammmc_out( ptOammmc, "%-8s = %-11s\n", TABLE_ATT_PHONE, pszPhone );

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	
	return MMC_HANDLER_SUCCESS;

_exit_failure:

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	
	return MMC_HANDLER_FAIL;
};

int MMC_Handler_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	nArgNum = nArgNum;
	ptUarg = ptUarg;

	int nRC = 0;
	int nId = 0;

	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;

	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );

	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;
	oammmc_arg_t *ptArg = NULL;

	ptArg = &patArgList[0];

	if ( 1 == OAMMMC_ARG_ID( ptArg ) )
	{
		nId = OAMMMC_VAL_INT( ptArg );

		snprintf( szQuery, sizeof(szQuery), "select * from %s where %s = %d;", TABLE_NAME, TABLE_ATT_ID, nId );
		szQuery[ strlen(szQuery) ] = '\0';

		nRC = dalExecute( g_ptDalConn, szQuery, &ptRes );
		if ( 0 == nRC )
		{
			nRC = dalResFree( ptRes );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				oammmc_out( ptOammmc, "System Fail" );
				return DAL_FAIL;
			}
			oammmc_out( ptOammmc, "id %d not exist", nId );
			return DAL_EXEC_ZERO;
		}
		else if ( -1 == nRC )
		{
			oammmc_out( ptOammmc, "System Fail" );
			return DAL_FAIL;
		}

		ptEntry = dalFetchFirst( ptRes );
		if ( NULL != ptEntry )
		{
			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}	

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}
		}	

		nRC = dalSetIntByKey( g_ptPstmtDelete, TABLE_ATT_ID, nId );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );		
			goto _exit_failure;
		}

		nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtDelete, NULL );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
		else if ( 0 == nRC )
		{
			MPGLOG_DBG( "%s:: id not exist", __func__ );
			oammmc_out( ptOammmc, "id %d not exist", nId );
			return DAL_EXEC_ZERO;
		}

		oammmc_out( ptOammmc, "%-8s = %-4d\n", TABLE_ATT_ID, nId );
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_NAME, pszName );
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_POSITION, pszPosition );
		oammmc_out( ptOammmc, "%-8s = %-32s\n", TABLE_ATT_TEAM, pszTeam );
		oammmc_out( ptOammmc, "%-8s = %-11s\n", TABLE_ATT_PHONE, pszPhone );
	}

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}

	return MMC_HANDLER_SUCCESS;

_exit_failure:

	nRC = dalResFree( ptRes );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}

	return MMC_HANDLER_FAIL;
}

int MMC_Handler_AddTrace( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	nArgNum = nArgNum;
	ptUarg = ptUarg;
	
	int nRC = 0;
	int nKey = 0;

	char szKey[TAP_REGI_KEY_SIZE];
 	memset( szKey, 0x00, sizeof(szKey) );

	oammmc_arg_t *ptArg = NULL;
	ptArg = &patArgList[0];

	if ( 1 == OAMMMC_ARG_ID( ptArg ) )
	{
		nKey = OAMMMC_VAL_INT( ptArg );

		snprintf( szKey, sizeof(szKey), "%s/%d", REGI_KEY_DIR, nKey );
		szKey[ strlen(szKey) ] = '\0';

		nRC = TAP_Registry_udp_key_create( szKey, strlen(szKey), TAP_REGISTRY_FILE, REGI_MAN_SYSTEM_ID );
		if ( TAP_REGI_ALREADY_EXIST == nRC )
		{
			MPGLOG_DBG( "%s:: create key %s", __func__, szKey );
			oammmc_out( ptOammmc, "create key %s", szKey );
		}
		else if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_Registry_udp_key_create() fail=%d", __func__ , nRC );
			oammmc_out( ptOammmc, "System Fail" );
			return REGI_FAIL;
		}
	}

	return MMC_HANDLER_SUCCESS;
}

int MMC_Handler_DisTrace( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	nArgNum = nArgNum;
	ptUarg = ptUarg;

	int nRC = 0;
	//int nKey  0;

	//char szKey[TAP_REGI_KEY_SIZE];
	char szTempBuf[1024];
	
	//memset( szKey, 0x00, sizeof(szKey) );
	memset( szTempBuf, 0x00, sizeof(szTempBuf) );

	//oammmc_arg_t *ptArg = NULL;
	//ptArg = &patArgList[0];

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	//if ( 1 == OAMMMC_ARG_ID( ptArg ) )
	
	//Key 입력받으면 key만 조회?
	//아무것도 입력받지 않으면 key 다 조회
//	nKey = OAMMMC_VAL_INT( ptArg );

//	snprintf( szKey, sizeof(szKey), "%s/%d", REGI_KEY_DIR, nKey );
//	szKey[ strlen(szKey) ] = '\0';

	nRC = TAP_Registry_udp_enum_key_node( REGI_KEY_DIR, strlen(REGI_KEY_DIR),
			szTempBuf, sizeof(szTempBuf), REGI_MAN_SYSTEM_ID );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_enum_key_node() fail=%d", __func__, nRC );
		return TAP_REGI_FAIL;
	}

	pszToken = strtok_r( szTempBuf, STRTOK_KEY_DELIM, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		MPGLOG_DBG( "%s:: key %s", __func__, pszToken );
		oammmc_out( ptOammmc, "key %s", pszToken );

		pszToken = strtok_r( NULL, STRTOK_KEY_DELIM, &pszDefaultToken );
	}

	return MMC_HANDLER_SUCCESS;
}

int MMC_Handler_DelTrace( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == patArgList )
	{
		MPGLOG_ERR( "%s:: patArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	nArgNum = nArgNum;
	ptUarg = ptUarg;

	int nRC = 0;
	int nId = 0;

	char szKey[TAP_REGI_KEY_SIZE];
	memset( szKey, 0x00, sizeof(szKey) );
	
	oammmc_arg_t *ptArg = NULL;
	ptArg = &patArgList[0];

	if ( 1 == OAMMMC_ARG_ID( ptArg ) )
	{
		nId = OAMMMC_VAL_INT( ptArg );

		snprintf( szKey, sizeof(szKey), "%s/%d", REGI_KEY_DIR, nId );
		szKey[ strlen(szKey) ] = '\0';

		nRC = TAP_Registry_udp_key_delete( szKey, strlen(szKey), TAP_REGISTRY_UDP_REMOVEALL, REGI_MAN_SYSTEM_ID );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: TAP_Regsitry_udp_key_delete() fail=%d", __func__, nRC );
			return RC_FAIL;
		}
	}

	oammmc_out( ptOammmc, "delete key %s", szKey );

	return MMC_HANDLER_SUCCESS;
}
