/* FW_Regi.c */
#include "FW_Header.h"

int REGI_CheckKey( int nId )
{
	int nRC = 0;

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	char szKey[TAP_REGI_KEY_SIZE];
	char szTempBuf[1024];

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szTempBuf, 0x00, sizeof(szTempBuf) );

	snprintf( szKey, sizeof(szKey), "%d", nId );
	szKey[ strlen(szKey) ] = '\0';

	nRC = TAP_Registry_udp_enum_key_node( REGI_KEY_DIR, strlen(REGI_KEY_DIR), szTempBuf, sizeof(szTempBuf), REGI_MAN_SYSTEM_ID );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_enum_key_node() fail=%d", __func__, nRC );
		return TAP_REGI_FAIL;
	}

	pszToken = strtok_r( szTempBuf, STRTOK_KEY_DELIM, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		MPGLOG_DBG( "%s", pszToken );

		if ( 0 == strcmp(szKey, pszToken) )
		{
			MPGLOG_DBG( "%s:: key %s exist", __func__, szKey );
			return RC_SUCCESS;
		}

		pszToken = strtok_r( NULL, STRTOK_KEY_DELIM, &pszDefaultToken );
	}

	MPGLOG_DBG( "%s:: key %s not exist", __func__, szKey );

	return RC_FAIL;
}
