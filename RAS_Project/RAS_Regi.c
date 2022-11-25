/* RAS_Regi.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;
REGI_t g_tRegi[MAX_REGI_CNT];

int REGI_Init()
{
	int nRC = 0;

	nRC = TAP_Registry_udp_open( g_tEnv.szRegiIp, g_tEnv.nRegiPort, '0', REGI_SYS_ID );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "TAP_Registry_udp_open fail <%d>", nRC );
		return RAS_rErrRegiInit;
	}

	nRC = TAP_Registry_udp_manager_check_alive( REGI_SYS_ID );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "TAP_Registry_udp_manager_check_alive fail <%d>", nRC );
		return RAS_rErrRegiInit;
	}

	return RAS_rOK;
}

int REGI_GetAll( int *pnCnt )
{
	CHECK_PARAM_RC( pnCnt );

	int nRC = 0;
	int nIndex = 0;
	char szKeyList[1024];
	memset( szKeyList, 0x00, sizeof(szKeyList) );
	char *pszKey = NULL;
	char *pszDefaultKey = NULL;
	char *pszVal = NULL;
	char *pszDefaultVal = NULL;

	memset( g_tRegi, 0x00, sizeof(g_tRegi) );

	REGI_GET_ENUM_KEY_VALUE( REGI_DIR, strlen(REGI_DIR), szKeyList, sizeof(szKeyList), nRC );

	/*
	 *	(keylist)
	 *	/RAS_CLI_IP_TRC/255.255.255.255|60:/RAS_CLI_IP_TRC/127.0.0.1|10800:/RAS_CLI_IP_TRC/0.0.0.0|60
	 *
	 *	REGI_DELIM_KEY : :/RAS_CLI_IP_TRC/
	 *	REGI_DELIM_VAL : | 
	 */
	pszKey = strtok_r( szKeyList, REGI_DELIM_KEY, &pszDefaultKey );

	while ( NULL != pszKey )
	{
		pszVal = strtok_r( pszKey, REGI_DELIM_VAL, &pszDefaultVal );
		strlcpy( g_tRegi[nIndex].szClientIp, pszVal, sizeof(g_tRegi[nIndex].szClientIp) );

		pszVal = strtok_r( NULL, REGI_DELIM_VAL, &pszDefaultVal );
		strlcpy( g_tRegi[nIndex].szPeriodTm, pszVal, sizeof(g_tRegi[nIndex].szPeriodTm) );

		pszKey = strtok_r( NULL, REGI_DELIM_KEY, &pszDefaultKey );

		if ( nIndex > MAX_REGI_CNT )
		{
			LOG_ERR_F( "key cnt more than MAX_REGI_CNT" );
			return RAS_rErrOutOfRange; 
		}
		nIndex++;
	}

	*pnCnt = nIndex;
	if ( 0 == *pnCnt )
	{
		return RAS_rErrRegiNotFound;
	}

	return RAS_rOK;

end_of_function:
	return nRC;
}

int REGI_CheckKeyExist( const char *pszIp )
{
	CHECK_PARAM_RC( pszIp );

	int nRC = 0;
	int nCnt = 0;
	int nIndex = 0;	

	nRC = REGI_GetAll( &nCnt );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	for ( nIndex = 0; nIndex < nCnt; nIndex++ )
	{
		if ( 0 == strcmp(pszIp, g_tRegi[nIndex].szClientIp) )
		{
			LOG_DBG_F( "found key (%s)", pszIp );
			return RAS_rOK;
		}
	}
	
	LOG_DBG_F( "not found key (%s)", pszIp );
	return RAS_rErrRegiNotFound; 
}
