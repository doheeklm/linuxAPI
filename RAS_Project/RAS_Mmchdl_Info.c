/* RAS_Mmchdl_Info.c */
#include "RAS_Inc.h"

extern DB_t				g_tDBIpc;
extern int				g_nUser;
extern pthread_mutex_t	g_tMutex;

static mpenumx_t atGenderEnum[] =
{
	{ ARG_STR_MALE, ARG_ID_MALE, MSG_ID_DIS_USR_INFO },
	{ ARG_STR_FEMALE, ARG_ID_FEMALE, MSG_ID_DIS_USR_INFO },
	{ NULL, 0, 0 }
};

static oammmc_arg_info_t atArgInfo[] =
{
	{ ARG_NUM_ID, ARG_STR_ID, NULL, OAMMMC_INT, ARG_ID_ID, 1, INT_MAX, NULL, NULL },
	{ ARG_NUM_NAME, ARG_STR_NAME, NULL, OAMMMC_STR, ARG_ID_NAME, 1, 32, NULL, NULL },
	{ ARG_NUM_GENDER, ARG_STR_GENDER, NULL, OAMMMC_ENUM, ARG_ID_GENDER, 0, 0, atGenderEnum, NULL },
	{ ARG_NUM_BIRTH, ARG_STR_BIRTH, NULL, OAMMMC_STR, ARG_ID_BIRTH, 1, 6, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_arg_info_t atArgId[] =
{
	{ ARG_NUM_ID, ARG_STR_ID, NULL, OAMMMC_INT, ARG_ID_ID, 1, INT_MAX, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL, NULL }
};

static oammmc_cmd_t atCmd[] =
{
	{ CMD_NUM_DIS_INFO, CMD_STR_DIS_INFO, MSG_ID_DIS_USR_INFO, MMCHDL_INFO_Dis, 0, 4, atArgInfo, "DISPLAY INFO" },
	{ CMD_NUM_DEL_INFO, CMD_STR_DEL_INFO, MSG_ID_DEL_USR_INFO, MMCHDL_INFO_Del, 1, 1, atArgId, "DELETE INFO" },
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

	int nRC = 0;
	int nAddFlag = RAS_FALSE;
	int nIndex = 0;
	int nTuple = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszGender = NULL;
	char *pszBirth = NULL;
	char *pszAddress = NULL;
	oammmc_arg_t *ptArg = NULL;
	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );
	char szTemp[256];
	memset( szTemp, 0x00, sizeof(szTemp) );
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	snprintf( szQuery, sizeof(szQuery), "%s%s", SQL_SELECT_CUSTOM, USR_INFO_TBL );
	szQuery[ strlen(szQuery) ] = '\0';

	if ( 0 < nArgNum )
	{
		STRLCAT_OVERFLOW_CHECK( szQuery, SQL_WHERE, sizeof(szQuery), nRC );
	}

	for ( nIndex = 0; nIndex < nArgNum; nIndex++ )
	{
		ptArg = &patArgList[ nIndex ];
	
		if ( RAS_TRUE == nAddFlag )
		{
			STRLCAT_OVERFLOW_CHECK( szQuery, SQL_AND, sizeof(szQuery), nRC );
			
			memset( szTemp, 0x00, sizeof(szTemp) );
		}

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_ID:
			{
				nId = OAMMMC_VAL_INT( ptArg );
				SNPRINTF_QUERY_INT( szTemp, sizeof(szTemp), ATTR_ID, nId );
			}
				break;
			case ARG_NUM_NAME:
			{
				pszName = OAMMMC_VAL_STR( ptArg );
				SNPRINTF_QUERY_STR( szTemp, sizeof(szTemp), ATTR_NAME, pszName );				
			}
				break;
			case ARG_NUM_GENDER:
			{
				pszGender = OAMMMC_VAL_STR( ptArg );
				SNPRINTF_QUERY_STR( szTemp, sizeof(szTemp), ATTR_GENDER, pszGender );
			}
				break;
			case ARG_NUM_BIRTH:
			{
				pszBirth = OAMMMC_VAL_STR( ptArg );
				SNPRINTF_QUERY_STR( szTemp, sizeof(szTemp), ATTR_BIRTH, pszBirth );
			}
				break;
		}

		STRLCAT_OVERFLOW_CHECK( szQuery, szTemp, sizeof(szQuery), nRC );
		
		nAddFlag = RAS_TRUE;
	}

	nAddFlag = RAS_FALSE;

	STRLCAT_OVERFLOW_CHECK( szQuery, SQL_SEMICOLON, sizeof(szQuery), nRC );

	DB_EXECUTE( g_tDBIpc, szQuery, &ptRes, nRC );

	PRT_INFO_ALL_HEADER( ptOammmc, ATTR_ID, ATTR_NAME, ATTR_GENDER, ATTR_BIRTH, ATTR_ADDRESS );

	for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
	{
		DB_GET_INT_BY_KEY( ptEntry, ATTR_ID, &nId, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );	
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );	
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );	
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );	

		PRT_INFO_ALL_BODY( ptOammmc, nId, pszName, pszGender, pszBirth, pszAddress );	
		nTuple++;
	}

	PRT_LINEx3( ptOammmc );
	PRT_CNT( ptOammmc, nTuple );

	DB_FREE( ptRes );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	return nRC;
}

int MMCHDL_INFO_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg )
{
	CHECK_PARAM_RC( ptOammmc );
	CHECK_PARAM_RC( ptCmd );
	CHECK_PARAM_RC( patArgList );
	ptUarg = ptUarg;

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

	for ( nIndex = 0; nIndex < nArgNum; nIndex++ )
	{
		ptArg = &patArgList[ nIndex ];
		
		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case ARG_NUM_ID:
			{
				nId = OAMMMC_VAL_INT( ptArg );
				if ( 0 == nId )
				{
					nRC = RAS_rErrInvalidParam;
					goto end_of_function;
				}
			}
				break;
		}	
	}
	
	DB_SET_INT_BY_KEY( g_tDBIpc.patPstmt[PSTMT_SELECT_INFO_BY_ID], ATTR_ID, nId, nRC );
	DB_PREPARED_EXEC( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_SELECT_INFO_BY_ID], &ptRes, nRC );

	ptEntry = dalFetchFirst( ptRes );
	if ( NULL != ptEntry )
	{
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );
	}
	else
	{
		nRC = RAS_rErrDBFetch;
		goto end_of_function;
	}

	DB_SET_INT_BY_KEY( g_tDBIpc.patPstmt[PSTMT_DELETE_INFO], ATTR_ID, nId, nRC );
	DB_PREPARED_EXEC_UPDATE( g_tDBIpc, g_tDBIpc.patPstmt[PSTMT_DELETE_INFO], nRC );
	PRT_INFO_ONE( ptOammmc, ATTR_ID, nId, ATTR_NAME, pszName,
			ATTR_GENDER, pszGender, ATTR_BIRTH, pszBirth, ATTR_ADDRESS, pszAddress );

	pthread_mutex_lock( &g_tMutex );
	g_nUser--;
	pthread_mutex_unlock( &g_tMutex );

	DB_FREE( ptRes );
	return RAS_rSuccessMmchdl;

end_of_function:
	PRT_FAIL( ptOammmc, "%s\n", ERR_GetDesc(nRC) );
	DB_FREE( ptRes );
	return nRC;
}
