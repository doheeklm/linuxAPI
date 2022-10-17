/* FW_Regi.c */
#include "FW_Header.h"

int REGI_CheckKey( char* pszKey )
{
	int nRC = 0;

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	char szKey[TAP_REGI_KEY_SIZE];
	char szBuf[1024];

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szBuf, 0x00, sizeof(szBuf) );

	nRC = TAP_Registry_udp_enum_key_node( REGI_KEY_DIR, strlen(REGI_KEY_DIR), szBuf, sizeof(szBuf), REGI_MAN_SYSTEM_ID );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Registry_udp_enum_key_node() fail=%d", __func__, nRC );
		return TAP_REGI_FAIL;
	}

	pszToken = strtok_r( szBuf, DELIM, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		MPGLOG_DBG( "%s", pszToken );

		if ( 0 == strcmp(pszKey, pszToken) )
		{
			MPGLOG_DBG( "%s:: key exist", __func__ );
			return RC_SUCCESS;
		}

		pszToken = strtok_r( NULL, DELIM, &pszDefaultToken );
	}

	MPGLOG_DBG( "%s:: key not exist", __func__ );

	return RC_FAIL;
}
