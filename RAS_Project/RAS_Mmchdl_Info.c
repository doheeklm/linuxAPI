/* RAS_Mmchdl_Info.c */
#include "RAS_Inc.h"

extern DB_t g_tDBIpc;

static mpenumx_t atGenderEnum[] =
{
	{ ARG_DESC_M, ARG_ID_M, MSG_ID_DIS_USR_INFO },
	{ ARG_DESC_F, ARG_ID_F, MSG_ID_DIS_USR_INFO },
	{ NULL, 0, 0 }
};

static oammmc_arg_info_t atArgInfo[] =
{
	{ ARG_NUM_ID, ARG_DESC_ID, NULL, OAMMMC_INT, ARG_ID_ID, 1, INT_MAX, NULL, NULL },
	{ ARG_NUM_NAME, ARG_DESC_NAME, NULL, OAMMMC_STR, ARG_ID_NAME, 1, 32, NULL, NULL },
	{ ARG_NUM_GENDER, ARG_DESC_GENDER, NULL, OAMMMC_ENUM, ARG_ID_GENDER, 0, 0, atGenderEnum, NULL },
	{ ARG_NUM_BIRTH, ARG_DESC_BIRTH, NULL, OAMMMC_STR, ARG_ID_BIRTH, 1, 6, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgId[] =
{
	{ ARG_NUM_ID, ARG_DESC_ID, NULL, OAMMMC_INT, ARG_ID_ID, 1, INT_MAX, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ CMD_NUM_DIS_INFO, CMD_DESC_DIS_INFO, MSG_ID_DIS_USR_INFO, MMCHDL_INFO_Dis, 0, 4, atArgInfo, "DISPLAY INFO" },
	{ CMD_NUM_DEL_INFO, CMD_DESC_DEL_INFO, MSG_ID_DEL_USR_INFO, MMCHDL_INFO_Del, 1, 1, atArgId, "DELETE INFO" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMCHDL_INFO_Init( oammmc_t *ptOammmc )
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

int MMCHDL_INFO_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nIndex = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszGender = NULL;
	char *pszBirth = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_ID:
			{
				nId = OAMMMC_VAL_INT( ptArg );
			}
				break;
			case ARG_NUM_NAME:
			{
				pszName = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case ARG_NUM_GENDER:
			{
				pszGender = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case ARG_NUM_BIRTH:
			{
				pszBirth = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}	
	}

	oammmc_out( ptOammmc, "%d %s %s %s", nId, pszName, pszGender, pszBirth );

	return RAS_rSuccessMmchdl;
}

int MMCHDL_INFO_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nRC = 0;
	int nIndex = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszGender = NULL;
	char *pszBirth = NULL;
	char *pszAddress = NULL;
	oammmc_arg_t *ptArg = NULL;
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_ID:
			{
				//TODO Mandatory Arg Check
				nId = OAMMMC_VAL_INT( ptArg );
			}
				break;
		}	
	}
	
	DB_SET_INT_BY_KEY( g_tDBIpc.patPstmt[PSTMT_SELECT_USR_INFO_BY_ID], ATTR_ID, nId, nRC );
	DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_USR_INFO_BY_ID], &ptRes, nRC );

	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );
	}

	DB_SET_INT_BY_KEY( g_tDBIpc.patPstmt[PSTMT_DELETE_USR_INFO], ATTR_ID, nId, nRC );
	DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_DELETE_USR_INFO], nRC );
	PRT_INFO_ONE( ptOammmc, ATTR_ID, nId, ATTR_NAME, pszName,
			ATTR_GENDER, pszGender, ATTR_BIRTH, pszBirth, ATTR_ADDRESS, pszAddress );

	DB_FREE( ptRes );
	DB_Close( &g_tDBIpc );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	DB_Close( &g_tDBIpc );
	return nRC;
}
