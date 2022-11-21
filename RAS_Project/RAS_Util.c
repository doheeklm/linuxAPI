/* RAS_Util.c */
#include "RAS_Inc.h"

extern DB_t g_tDBMain;

int UTIL_CheckClientIp( char *pszIp )
{
	CHECK_PARAM_RC( pszIp );

	int nRC = 0;
	DAL_RESULT_SET *ptRes = NULL;

	DB_SET_STRING_BY_KEY( g_tDBMain.patPstmt[PSTMT_SELECT_IP_BY_IP], ATTR_IP, pszIp, nRC );

	DB_PREPARED_EXEC( g_tDBMain, g_tDBMain.patPstmt[PSTMT_SELECT_IP_BY_IP], &ptRes, nRC );

	DB_FREE( ptRes );
	return RAS_rOK;

end_of_function:
	DB_FREE( ptRes );
	return nRC;
}

int UTIL_CheckPath( char *pszPath )
{
	CHECK_PARAM_RC( pszPath );

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	
	pszToken = strtok_r( pszPath, HTTP_DELIM_SLASH, &pszDefaultToken );
	if ( NULL == pszToken )
	{
		LOG_ERR_F( "not found <//> in path" );
		return RAS_rErrHttpBadRequest;
	}

	while ( NULL != pszToken )
	{
		if ( 0 != strcmp( USER_DIR, pszToken ) )
		{
			LOG_ERR_F( "not found <user> in path" );
			return RAS_rErrHttpBadRequest;
		}

		pszToken = strtok_r( NULL, HTTP_DELIM_SLASH, &pszDefaultToken );
		if ( NULL != pszToken )
		{
			return RAS_rErrHttpBadRequest;
		}
		
		break;
	}

	return RAS_rOK;
}

int UTIL_GetIdFromPath( char *pszPath, int *pnId )
{
	CHECK_PARAM_RC( pszPath );
	CHECK_PARAM_RC( pnId );

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	
	pszToken = strtok_r( pszPath, HTTP_DELIM_SLASH, &pszDefaultToken );
	if ( NULL == pszToken )
	{
		LOG_ERR_F( "not found <//> in path" );
		return RAS_rErrHttpBadRequest;
	}

	while ( NULL != pszToken )
	{
		if ( 0 != strcmp( USER_DIR, pszToken ) )
		{
			LOG_ERR_F( "not found <user> in path" );
			return RAS_rErrHttpBadRequest;
		}

		pszToken = strtok_r( NULL, HTTP_DELIM_SLASH, &pszDefaultToken );
		if ( NULL != pszToken )
		{
			*pnId = atoi( pszToken );
			LOG_DBG_F( "/user/%d", *pnId );

			if ( 0 >= *pnId || INT_MAX < *pnId )
			{
				LOG_ERR_F( "Id (%d) negative, or greater than INT_MAX", *pnId );
				return RAS_rErrHttpBadRequest;
			}
		}
	
		//*pnId still zero	
		
		break;
	}

	return RAS_rOK;
}
