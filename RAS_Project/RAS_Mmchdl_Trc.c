/* RAS_Mmchdl_Trc.c */
#include "RAS_Inc.h"

static oammmc_arg_info_t atArgIp[] =
{
	{ ARG_NUM_IP, ARG_DESC_IP, NULL, OAMMMC_STR, ARG_ID_IP, 7, 15, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgIpAndTime[] =
{
	{ ARG_NUM_IP, ARG_DESC_IP, NULL, OAMMMC_STR, ARG_ID_IP, 7, 15, NULL, NULL },
	{ ARG_NUM_TIME, ARG_DESC_TIME, NULL, OAMMMC_INT, ARG_ID_TIME, 1, 10800, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ CMD_NUM_ADD_TRC, CMD_DESC_ADD_TRC, MSG_ID_ADD_CLI_IP_TRC, MMCHDL_TRC_Add, 1, 2, atArgIpAndTime, "ADD CLIENT IP TRACE" },
	{ CMD_NUM_DIS_TRC, CMD_DESC_DIS_TRC, MSG_ID_DIS_CLI_IP_TRC, MMCHDL_TRC_Dis, 0, 1, atArgIp, "DISPLAY CLIENT IP TRACE" },
	{ CMD_NUM_DEL_TRC, CMD_DESC_DEL_TRC, MSG_ID_DEL_CLI_IP_TRC, MMCHDL_TRC_Del, 1, 1, atArgIp, "DELETE CLIENT IP TRACE" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMCHDL_TRC_Init( oammmc_t *ptOammmc )
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

int MMCHDL_TRC_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nRC = 0;	
	int nIndex = 0;
	char *pszIp = NULL;
	int nTime = 0;
	char szKey[REGI_KEY_MAX];
	memset( szKey, 0x00, sizeof(szKey) );	
	char szTime[32];
	memset( szTime, 0x00, sizeof(szTime) );
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_IP:
			{
				//TOOD Mandatory Arg Check
				pszIp = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case ARG_NUM_TIME:
			{
				nTime = OAMMMC_VAL_INT( ptArg );
			}
				break;
		}	
	}

	REGI_STR_KEY( szKey, sizeof(szKey), pszIp );
	REGI_CREATE( szKey, strlen(szKey), nRC );	

	if ( 0 != nTime )
	{
		REGI_STR_VALUE( szTime, sizeof(szTime), nTime );
		REGI_SET_VALUE( szKey, strlen(szKey), szTime, strlen(szTime), nRC );
	}
	else
	{
		REGI_SET_VALUE( szKey, strlen(szKey), " ", 1, nRC );
	}

	PRT_IP_ONE( ptOammmc, ARG_DESC_IP, pszIp, ARG_DESC_TIME, szTime ); 
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	return nRC;
}

int MMCHDL_TRC_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nRC = 0;
	int nIndex = 0;
	char *pszIp = NULL;
	char szKey[REGI_KEY_MAX];
	memset( szKey, 0x00, sizeof(szKey) );
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
		}	
	}

	if ( NULL == pszIp )
	{
		char szKeyList[256];
		memset( szKeyList, 0x00, sizeof(szKeyList) );

		//NOTE Value 없는 Key는 Keylist 버퍼에 포함되지 않아서 Period Time 받지 않으면 공백 넣었음
		REGI_GET_KEY_AND_VALUE( REGI_DIR, strlen(REGI_DIR), szKeyList, sizeof(szKeyList), nRC );
		oammmc_out( ptOammmc, "%s\n", szKeyList );
	}
	else
	{
		char szTime[32];
		memset( szTime, 0x00, sizeof(szTime) );

		REGI_STR_KEY( szKey, sizeof(szKey), pszIp );
		REGI_GET_VALUE( szKey, strlen(szKey), szTime, sizeof(szTime), nRC );
		PRT_IP_ONE( ptOammmc, ARG_DESC_IP, pszIp, ARG_DESC_TIME, szTime );
	}

	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	return nRC;
}

int MMCHDL_TRC_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nRC = 0;
	int nIndex = 0;
	char *pszIp = NULL;
	char szKey[REGI_KEY_MAX];
	memset( szKey, 0x00, sizeof(szKey) );	
	char szTime[32];
	memset( szTime, 0x00, sizeof(szTime) );
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
		}	
	}

	REGI_STR_KEY( szKey, sizeof(szKey), pszIp );
	REGI_GET_VALUE( szKey, strlen(szKey), szTime, sizeof(szTime), nRC );
	REGI_DELETE( szKey, strlen(szKey), nRC );
	PRT_IP_ONE( ptOammmc, ARG_DESC_IP, pszIp, ARG_DESC_TIME, szTime );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	return nRC;
}