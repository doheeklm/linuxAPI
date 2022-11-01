/* RAS_Mmchdl_CliIp.c */
#include "RAS_Inc.h"

static oammmc_arg_info_t atArgIpDesc[] =
{
	{ 1, "CLI_IP", NULL, OAMMMC_STR, ARG_ID_CLI_IP, 7, 39, NULL, NULL },
	{ 2, "DESC", NULL, OAMMMC_STR, ARG_ID_DESC, 1, 32, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgIp[] =
{
	{ 1, "CLI_IP", NULL, OAMMMC_STR, ARG_ID_CLI_IP, 7, 39, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ 1, "add-cli-ip", MSG_ID_ADD_CLI_IP, MMCHDL_CLIIP_Add, 1, 2, atArgIpDesc, "CMD_DESC: ADD CLIENT IP" },
	{ 2, "dis-cli-ip", MSG_ID_DIS_CLI_IP, MMCHDL_CLIIP_Dis, 0, 1, atArgIp, "CMD_DESC: DISPLAY CLIENT IP" },
	{ 3, "del-cli-ip", MSG_ID_DEL_CLI_IP, MMCHDL_CLIIP_Del, 1, 1, atArgIp, "CMD_DESC: DELETE CLIENT IP" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMCHDL_CLIIP_Init( oammmc_t *ptOammmc )
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

int MMCHDL_CLIIP_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;

	int nIndex = 0;
	char *pszIp = NULL;
	char *pszDesc = NULL;
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
				pszDesc = OAMMMC_VAL_STR( ptArg );	
			}
				break;
			default:
			{
				return RAS_rErrFail;
			}
				break;
		}	
	}

	oammmc_out( ptOammmc, "%8s = %s\n", ATT_IP, pszIp );
	oammmc_out( ptOammmc, "%8s = %s", ATT_DESC, pszDesc );

	return RAS_rSuccessMmchdl;
}

int MMCHDL_CLIIP_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
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

	oammmc_out( ptOammmc, "%8s = %s", ATT_IP, pszIp );

	return RAS_rSuccessMmchdl;
}

int MMCHDL_CLIIP_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
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

	oammmc_out( ptOammmc, "%8s = %s", ATT_IP, pszIp );

	return RAS_rSuccessMmchdl;
}
