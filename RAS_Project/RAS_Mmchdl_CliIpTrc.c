/* RAS_Mmchdl_CliIpTrc.c */
#include "RAS_Inc.h"

static oammmc_arg_info_t atArgIp[] =
{
	{ 1, ARG_CLI_IP_DESC, NULL, OAMMMC_STR, ARG_CLI_IP_ID, 7, 15, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgIpPeriodTm[] =
{
	{ 1, ARG_CLI_IP_DESC, NULL, OAMMMC_STR, ARG_CLI_IP_ID, 7, 15, NULL, NULL },
	{ 2, ARG_PERIOD_TM_DESC, NULL, OAMMMC_STR, ARG_PERIOD_TM_ID, 1, 10800, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ 1, "add-cli-ip-trc", MSG_ID_ADD_CLI_IP_TRC, MMCHDL_CLIIPTRC_Add, 1, 2, atArgIpPeriodTm, "CMD_DESC: ADD CLIENT IP TRACE" },
	{ 2, "dis-cli-ip-trc", MSG_ID_DIS_CLI_IP_TRC, MMCHDL_CLIIPTRC_Dis, 0, 1, atArgIp, "CMD_DESC: DISPLAY CLIENT IP TRACE" },
	{ 3, "del-cli-ip-trc", MSG_ID_DEL_CLI_IP_TRC, MMCHDL_CLIIPTRC_Del, 1, 1, atArgIp, "CMD_DESC: DELETE CLIENT IP TRACE" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMCHDL_CLIIPTRC_Init( oammmc_t *ptOammmc )
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

int MMCHDL_CLIIPTRC_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nIndex = 0;
	char *pszIp = NULL;
	char *pszPeriodTm = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 2:
			{
				pszPeriodTm = OAMMMC_VAL_STR( ptArg );	
			}
				break;
			default:
			{
				return RAS_rErrFail;
			}
				break;
		}	
	}
	
	oammmc_out( ptOammmc, "%s %s", pszIp, pszPeriodTm );

	return RAS_rSuccessMmchdl;
}

int MMCHDL_CLIIPTRC_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nIndex = 0;
	char *pszIp = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
			default:
			{
				return RAS_rErrFail;
			}
				break;
		}	
	}

	oammmc_out( ptOammmc, "%s", pszIp );

	return RAS_rSuccessMmchdl;
}

int MMCHDL_CLIIPTRC_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nIndex = 0;
	char *pszIp = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
			default:
			{
				return RAS_rErrFail;
			}
				break;
		}	
	}

	oammmc_out( ptOammmc, "%s", pszIp );

	return RAS_rSuccessMmchdl;
}
