/* FW_Regi.c */
#include "FW_Header.h"

int REGI_CheckKey( int nId )
{
	int nRC = 0;

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	char szRegiKey[TAP_REGI_KEY_SIZE];
	char szTempBuf[1024];

	memset( szRegiKey, 0x00, sizeof(szRegiKey) );
	memset( szTempBuf, 0x00, sizeof(szTempBuf) );

	snprintf( szRegiKey, sizeof(szRegiKey), "%d", nId );
	szRegiKey[ strlen(szRegiKey) ] = '\0';

	nRC = TAP_Registry_udp_enum_key_node( REGI_KEY_DIR, strlen(REGI_KEY_DIR), szTempBuf, sizeof(szTempBuf), REGI_MAN_SYSTEM_ID );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_enum_key_node() fail<%d>", __func__, nRC );
		return TAP_REGI_FAIL;
	}

	pszToken = strtok_r( szTempBuf, REGI_KEY_DELIM, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		MPGLOG_DBG( "%s", pszToken );

		if ( 0 == strcmp(szRegiKey, pszToken) )
		{
			MPGLOG_DBG( "%s:: key %s exist", __func__, szRegiKey );
			return RC_SUCCESS;
		}

		pszToken = strtok_r( NULL, REGI_KEY_DELIM, &pszDefaultToken );
	}

	MPGLOG_DBG( "%s:: key %s not exist", __func__, szRegiKey );

	return RC_FAIL;
}
