/* RAS_Mmchdl_Ip.c */
#include "RAS_Inc.h"

extern DB_t g_tDBIpc;

//CLEAR define arg number
static oammmc_arg_info_t atArgIp[] =
{
	{ ARG_NUM_IP, ARG_DESC_IP, NULL, OAMMMC_STR, ARG_ID_IP, 7, 15, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgIpAndDesc[] =
{
	{ ARG_NUM_IP, ARG_DESC_IP, NULL, OAMMMC_STR, ARG_ID_IP, 7, 15, NULL, NULL },
	{ ARG_NUM_DESC,	ARG_DESC_DESC, NULL, OAMMMC_STR, ARG_ID_DESC, 1, 32, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ CMD_NUM_ADD_IP, CMD_DESC_ADD_IP, MSG_ID_ADD_CLI_IP, MMCHDL_IP_Add, 1, 2, atArgIpAndDesc, "ADD CLIENT IP" },
	{ CMD_NUM_DIS_IP, CMD_DESC_DIS_IP, MSG_ID_DIS_CLI_IP, MMCHDL_IP_Dis, 0, 1, atArgIp, "DISPLAY CLIENT IP" },
	{ CMD_NUM_DEL_IP, CMD_DESC_DEL_IP, MSG_ID_DEL_CLI_IP, MMCHDL_IP_Del, 1, 1, atArgIp, "DELETE CLIENT IP" },
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

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{	
			case ARG_NUM_IP:
			{
				//TODO Mandatory Arg Check
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case ARG_NUM_DESC:
			{
				pszDesc = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}	
	}

	//TODO CLIENT IP 중복

	//CLEAR DB SET/GET/EXEC 실패하면 mmc print out
	DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_INSERT_CLI_IP], ATTR_IP, pszIp, nRC );
	
	if ( NULL != pszDesc )
	{
		DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_INSERT_CLI_IP], ATTR_DESC, pszDesc, nRC );
	}	
	else
	{
		DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_INSERT_CLI_IP], ATTR_DESC, EMPTY_STRING, nRC );
	}

	DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_INSERT_CLI_IP], nRC );
	
	PRT_IP_ONE( ptOammmc, ARG_DESC_IP, pszIp, ARG_DESC_DESC, pszDesc );
	//CLEAR IPC Handler에서 Init
	DB_Close( &g_tDBIpc );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_Close( &g_tDBIpc );		
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
	int nCntTuple = 0;
	char *pszIp = NULL;
	char *pszDesc = NULL;
	oammmc_arg_t *ptArg = NULL;
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;
	
	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_IP:
			{
				//TODO Mandatory Arg Check
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}	
	}

	if ( NULL == pszIp )
	{
		DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_CLI_IP_ALL], &ptRes, nRC );	

		PRT_IP_ALL_HEADER( ptOammmc, ARG_DESC_IP, ARG_DESC_DESC );

		for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
		{
			//CLEAR 여기서 GET 실패 시 mmc clear
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_IP, &pszIp, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_DESC, &pszDesc, nRC );
			PRT_IP_ALL_BODY( ptOammmc, pszIp, pszDesc );
			nCntTuple++;
		}
		
		PRT_IP_ALL_CNT( ptOammmc, nCntTuple );
	}
	else
	{
		DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_SELECT_CLI_IP_BY_IP], ATTR_IP, pszIp, nRC );
	
		DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_CLI_IP_BY_IP], &ptRes, nRC );

		ptEntry = dalFetchFirst( ptRes );
		if ( NULL != ptEntry )
		{
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_DESC, &pszDesc, nRC );
		}
		
		PRT_IP_ONE( ptOammmc, ARG_DESC_IP, pszIp, ARG_DESC_DESC, pszDesc );
	}

	DB_FREE( ptRes );
	DB_Close( &g_tDBIpc );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	DB_Close( &g_tDBIpc );
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

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_IP:
			{
				//TODO Mandatory Arg Check
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}	
	}

	DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_SELECT_CLI_IP_BY_IP], ATTR_IP, pszIp, nRC );
	DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_CLI_IP_BY_IP], &ptRes, nRC );
	
	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_DESC, &pszDesc, nRC );
	}

	DB_SET_STRING_BY_KEY( g_tDBIpc.patPstmt[PSTMT_DELETE_CLI_IP], ATTR_IP, pszIp, nRC );
	DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_DELETE_CLI_IP], nRC );
	
	PRT_IP_ONE( ptOammmc, ARG_DESC_IP, pszIp, ARG_DESC_DESC, pszDesc );

	DB_FREE( ptRes );
	DB_Close( &g_tDBIpc );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	DB_Close( &g_tDBIpc );
	return nRC;
}
