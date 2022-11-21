/* RAS_Regi.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;
extern TRC_t g_tTrc[MAX_TRC_CNT];

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

int REGI_GetAll()
{
	int nRC = 0;
	int nIndex = 0;
	char szKeyList[1024];
	memset( szKeyList, 0x00, sizeof(szKeyList) );
	char *pszTokenKey = NULL;
	char *pszDefaultTokenKey = NULL;
	char *pszTokenVal = NULL;
	char *pszDefaultTokenVal = NULL;

	memset( g_tTrc, 0x00, sizeof(g_tTrc) );

	REGI_GET_ENUM_KEY_VALUE( REGI_DIR, strlen(REGI_DIR), szKeyList, sizeof(szKeyList), nRC );

	pszTokenKey = strtok_r( szKeyList, REGI_DELIM_KEY, &pszDefaultTokenKey );

	while ( NULL != pszTokenKey )
	{
		pszTokenVal = strtok_r( pszTokenKey, REGI_DELIM_VAL, &pszDefaultTokenVal );
		//LOG_DBG_F( "Key: %s", pszTokenVal );
		strlcpy( g_tTrc[nIndex].szClientIp, pszTokenVal, sizeof(g_tTrc[nIndex].szClientIp) );

		pszTokenVal = strtok_r( NULL, REGI_DELIM_VAL, &pszDefaultTokenVal );
		//LOG_DBG_F( "Value: %s", pszTokenVal );
		strlcpy( g_tTrc[nIndex].szPeriodTm, pszTokenVal, sizeof(g_tTrc[nIndex].szPeriodTm) );

		pszTokenKey = strtok_r( NULL, REGI_DELIM_KEY, &pszDefaultTokenKey );
		
		if ( nIndex > MAX_TRC_CNT )
		{
			break;
		}
		nIndex++;
	}

	return RAS_rOK;

end_of_function:
	return nRC;
}

int REGI_CheckKeyExist( const char *pszIp )
{
	CHECK_PARAM_RC( pszIp );

	int nRC = 0;
	int nIndex = 0;	

	nRC = REGI_GetAll();
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	for ( nIndex = 0; nIndex < MAX_TRC_CNT; nIndex++ )
	{
		if ( 0 == strcmp(pszIp, g_tTrc[nIndex].szClientIp) )
		{
			LOG_DBG_F( "found key (%s)", pszIp );
			return RAS_rOK;
		}
	}
	
	LOG_DBG_F( "not found key (%s)", pszIp );
	return RAS_rErrRegiNotFound; 
}
