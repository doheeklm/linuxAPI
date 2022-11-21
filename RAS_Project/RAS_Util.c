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

int UTIL_CheckPathAndGetId( const char *pszMethod, char *pszPath, int *pnId )
{
	CHECK_PARAM_RC( pszMethod );
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
			if ( 0 == strcmp(HTTP_METHOD_POST, pszMethod) ||
			  	 0 == strcmp(HTTP_METHOD_DELETE, pszMethod) )
			{
				//POST, DELETE는 /user 만 입력받아야함
				return RAS_rErrHttpBadRequest;
			}
			else
			{
				//GET 특정 아이디 조희
				CHECK_PARAM_RC( pnId );

				*pnId = atoi( pszToken );
				if ( 0 >= *pnId || INT_MAX < *pnId )
				{
					LOG_ERR_F( "Id (%d) negative, or greater than INT_MAX", *pnId );
					return RAS_rErrHttpBadRequest;
				}
			}
		}
	
		//GET 모든 아이디 조회

		break;
	}

	return RAS_rOK;
}
