/* RAS_Mmchdl_Ip.c */
#include "RAS_Inc.h"

extern DB_t g_tDBIpc;

static oammmc_arg_info_t atArgIp[] =
{
	{ ARG_NUM_IP, ARG_STR_IP, NULL, OAMMMC_STR, ARG_ID_IP, 7, 15, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgIpAndDesc[] =
{
	{ ARG_NUM_IP, ARG_STR_IP, NULL, OAMMMC_STR, ARG_ID_IP, 7, 15, NULL, NULL },
	{ ARG_NUM_DESC,	ARG_STR_DESC, NULL, OAMMMC_STR, ARG_ID_DESC, 1, 32, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ CMD_NUM_ADD_IP, CMD_STR_ADD_IP, MSG_ID_ADD_CLI_IP, MMCHDL_IP_Add, 1, 2, atArgIpAndDesc, "ADD CLIENT IP" },
	{ CMD_NUM_DIS_IP, CMD_STR_DIS_IP, MSG_ID_DIS_CLI_IP, MMCHDL_IP_Dis, 0, 1, atArgIp, "DISPLAY CLIENT IP" },
	{ CMD_NUM_DEL_IP, CMD_STR_DEL_IP, MSG_ID_DEL_CLI_IP, MMCHDL_IP_Del, 1, 1, atArgIp, "DELETE CLIENT IP" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMCHDL_IP_Init( oammmc_t *ptOammmc )
{
	CHECK_PARAM_RC( ptOammmc );

	int nRC = 0;

	nRC = oammmc_regi_cmd( ptOammmc, atCmd, NULL );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "oammmc_regi_cmd fail <%d>", nRC );
		return RAS_rErrFail;
	}

	return RAS_rOK;
}

int MMCHDL_IP_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	
	int nRC = 0;
	int nIndex = 0;
	char *pszIp = NULL;
	char *pszDesc = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; nIndex++ )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{	
			case ARG_NUM_IP:
			{
				pszIp = OAMMMC_VAL_STR( ptArg ); //M
				CHECK_PARAM_GOTO( ptArg, nRC );
			}
				break;
			case ARG_NUM_DESC:
			{
				pszDesc = OAMMMC_VAL_STR( ptArg ); //O
			}
				break;
		}	
	}

	DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_INSERT_IP], ATTR_IP, pszIp, nRC );

	if ( NULL != pszDesc )
	{
		DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_INSERT_IP], ATTR_DESC, pszDesc, nRC );

		DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_INSERT_IP], nRC );

		PRT_IP_ONE( ptOammmc, ARG_STR_IP, pszIp, ARG_STR_DESC, pszDesc );
	}	
	else
	{
		DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_INSERT_IP], ATTR_DESC, EMPTY_STRING, nRC );

		DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_INSERT_IP], nRC );

		PRT_IP_ONE( ptOammmc, ARG_STR_IP, pszIp, ARG_STR_DESC, EMPTY_STRING );
	}
	
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	return nRC;
}

int MMCHDL_IP_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;

	int nRC = 0;
	int nIndex = 0;
	int nTuple = 0;
	char *pszIp = NULL;
	char *pszDesc = NULL;
	oammmc_arg_t *ptArg = NULL;
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;
	
	for ( nIndex = 0; nIndex < nArgNum; nIndex++ )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_IP:
			{
				pszIp = OAMMMC_VAL_STR( ptArg ); //O
			}
				break;
		}	
	}

	if ( NULL == pszIp )
	{
		DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_IP_ALL], &ptRes, nRC );	

		PRT_IP_ALL_HEADER( ptOammmc, ARG_STR_IP, ARG_STR_DESC );

		for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
		{
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_IP, &pszIp, nRC );

			DB_GET_STRING_BY_KEY( ptEntry, ATTR_DESC, &pszDesc, nRC );

			PRT_IP_ALL_BODY( ptOammmc, pszIp, pszDesc );

			nTuple++;
		}

		PRT_LINE( ptOammmc );	
		PRT_CNT( ptOammmc, nTuple );
	}
	else
	{
		DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_SELECT_IP_BY_IP], ATTR_IP, pszIp, nRC );
	
		DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_IP_BY_IP], &ptRes, nRC );

		ptEntry = dalFetchFirst( ptRes );
		if ( NULL != ptEntry )
		{
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_DESC, &pszDesc, nRC );
		}
		else
		{
			nRC = RAS_rErrDBFetch;
			goto end_of_function;
		}
		
		PRT_IP_ONE( ptOammmc, ARG_STR_IP, pszIp, ARG_STR_DESC, pszDesc );
	}

	DB_FREE( ptRes );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	return nRC;
}

int MMCHDL_IP_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;

	int nRC = 0;
	int nIndex = 0;
	char *pszIp = NULL;
	char *pszDesc = NULL;
	oammmc_arg_t *ptArg = NULL;
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	for ( nIndex = 0; nIndex < nArgNum; nIndex++ )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_IP:
			{
				pszIp = OAMMMC_VAL_STR( ptArg );
				CHECK_PARAM_GOTO( ptArg, nRC );
			}
				break;
		}	
	}

	DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_SELECT_IP_BY_IP], ATTR_IP, pszIp, nRC );

	DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_IP_BY_IP], &ptRes, nRC );
	
	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_DESC, &pszDesc, nRC );
	}
	else
	{
		nRC = RAS_rErrDBFetch;
		goto end_of_function;
	}

	DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_DELETE_IP], ATTR_IP, pszIp, nRC );

	DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_DELETE_IP], nRC );
	
	PRT_IP_ONE( ptOammmc, ARG_STR_IP, pszIp, ARG_STR_DESC, pszDesc );

	DB_FREE( ptRes );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	return nRC;
}
