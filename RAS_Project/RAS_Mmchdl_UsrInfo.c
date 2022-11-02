/* RAS_Mmchdl_UsrInfo.c */
#include "RAS_Inc.h"

static mpenumx_t atGenderEnum[] =
{
	{ "MALE", ARG_ID_MALE, MSG_ID_DIS_USR_INFO },
	{ "FEMALE", ARG_ID_FEMALE, MSG_ID_DIS_USR_INFO },
	{ NULL, 0, 0 }
};

static oammmc_arg_info_t atArgInfo[] =
{
	{ 1, "USR_ID", NULL, OAMMMC_INT, ARG_ID_USR_ID, 1, INT_MAX, NULL, NULL },
	{ 2, "USR_NAME", NULL, OAMMMC_STR, ARG_ID_USR_NAME, 1, 32, NULL, NULL },
	{ 3, "USR_GENDER", NULL, OAMMMC_ENUM, ARG_ID_USR_GENDER, 0, 0, atGenderEnum, NULL },
	{ 4, "USR_BIRTH", NULL, OAMMMC_STR, ARG_ID_USR_BIRTH, 1, 6, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgId[] =
{
	{ 1, "USR_ID", NULL, OAMMMC_INT, ARG_ID_USR_ID, 1, INT_MAX, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ 1, "dis-usr-info", MSG_ID_DIS_USR_INFO, MMCHDL_USRINFO_Dis, 0, 4, atArgInfo, "CMD_DESC: DISPLAY USER INFO" },
	{ 2, "del-usr-info", MSG_ID_DEL_USR_INFO, MMCHDL_USRINFO_Del, 1, 1, atArgId, "CMD_DESC: DELETE USER INFO" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int MMCHDL_USRINFO_Init( oammmc_t *ptOammmc )
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

int MMCHDL_USRINFO_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
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
				pszGender = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 4:
			{
				pszBirth = OAMMMC_VAL_STR( ptArg );
			}
				break;
			default:
			{
				return RAS_rErrFail;
			}
				break;
		}	
	}

	oammmc_out( ptOammmc, "%8s = %d\n", ATTR_ID, nId );
	oammmc_out( ptOammmc, "%8s = %s\n", ATTR_NAME, pszName );
	oammmc_out( ptOammmc, "%8s = %s\n", ATTR_GENDER, pszGender );
	oammmc_out( ptOammmc, "%8s = %s\n", ATTR_BIRTH, pszBirth );

	return RAS_rSuccessMmchdl;
}

int MMCHDL_USRINFO_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;
	nArgNum = nArgNum;

	int nIndex = 0;
	int nId = 0;
	oammmc_arg_t *ptArg = NULL;

	for ( nIndex = 0; nIndex < nArgNum; ++nIndex )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				nId = OAMMMC_VAL_INT( ptArg );
			}
				break;
			default:
			{
				return RAS_rErrFail;
			}
				break;
		}	
	}

	oammmc_out( ptOammmc, "%8s = %d\n", ATTR_ID, nId );
	
	return RAS_rSuccessMmchdl;
}
