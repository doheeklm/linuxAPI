/* RAS_Method.c */
#include "RAS_Inc.h"

int METHOD_Post( DB_t tDBWorker, struct HTTP_REQUEST_s *ptRequest, const char *pszIp )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( pszIp );

	int nRC = 0;
	int nCnt = 0;
	int nIndex = 0;

	char aszToken[8][256];
	memset( aszToken, 0x00, sizeof(aszToken) );
	
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	nRC = REGI_CheckKeyExist( pszIp );
	if ( RAS_rOK == nRC )
	{
		nRC = TRACE_MakeTrace( HTTP_TYPE_REQUEST, pszIp, ptRequest );
		if ( RAS_rOK != nRC )
		{
			return nRC;
		}
	}

	nRC = UTIL_CheckPathAndGetId( ptRequest->szMethod, ptRequest->szPath, NULL );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	pszToken = strtok_r( ptRequest->szBody, HTTP_DELIM_QUOTATION, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		if ( ODD_NUMBER == nCnt % 2 )
		{
			//토큰의 길이가 0이거나, 정해진 토큰버퍼의 사이즈보다 토큰의 길이가 길 경우
			if ( ( strlen(pszToken) > sizeof(aszToken[nIndex]) ) ||
				 ( 0 == strlen(pszToken) ) )
			{
				return RAS_rErrHttpBadRequest;
			}
			strlcpy( aszToken[nIndex], pszToken, sizeof(aszToken[nIndex]) );
			LOG_DBG_F( "%s", aszToken[nIndex] );

			nIndex++;
			if ( MAX_INDEX <= nIndex )
			{
				break;
			}
		}

		pszToken = strtok_r( NULL, HTTP_DELIM_QUOTATION, &pszDefaultToken );

		nCnt++;
	}

	STRCMP_ATTR( aszToken[TOK_NAME_ATTR], ATTR_NAME, nRC );
	STRCMP_ATTR( aszToken[TOK_GENDER_ATTR], ATTR_GENDER, nRC );
	STRCMP_ATTR( aszToken[TOK_BIRTH_ATTR], ATTR_BIRTH, nRC );
	STRCMP_ATTR( aszToken[TOK_ADDRESS_ATTR], ATTR_ADDRESS, nRC );

	DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_NAME, aszToken[TOK_NAME_STR], nRC );
	DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_GENDER, aszToken[TOK_GENDER_STR], nRC );
	DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_BIRTH, aszToken[TOK_BIRTH_STR], nRC );
	DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_ADDRESS, aszToken[TOK_ADDRESS_STR], nRC );

	DB_PREPARED_EXEC_UPDATE( tDBWorker, tDBWorker.patPstmt[PSTMT_INSERT_INFO], nRC );

	return RAS_rHttpCreated;

end_of_function:
	return nRC;
}

int METHOD_Get( DB_t tDBWorker, struct HTTP_REQUEST_s *ptRequest,
		struct HTTP_RESPONSE_s *ptResponse, const char *pszIp )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );
	CHECK_PARAM_RC( pszIp );

	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	int nRC = 0;
	int nCntTuple = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszGender = NULL;
	char *pszBirth = NULL;
	char *pszAddress = NULL;

	nRC = REGI_CheckKeyExist( pszIp );
	if ( RAS_rOK == nRC )
	{
		nRC = TRACE_MakeTrace( HTTP_TYPE_REQUEST, pszIp, ptRequest );
		if ( RAS_rOK != nRC )
		{
			return nRC;
		}
	}

	nRC = UTIL_CheckPathAndGetId( ptRequest->szMethod, ptRequest->szPath, &nId );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	if ( 0 == nId )
	{
		//SELECT ALL
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_ALL], &ptRes, nRC );
		
		for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
		{
			DB_GET_INT_BY_KEY( ptEntry, ATTR_ID, &nId, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );

			LOG_DBG_F( "%d %s %s %s %s", nId, pszName, pszGender, pszBirth, pszAddress );
			nCntTuple++;
		}

		LOG_DBG_F( "nCntTuple %d", nCntTuple );
	}
	else
	{
		//SELECT ONE
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_BY_ID], &ptRes, nRC );
		
		ptEntry = dalFetchFirst( ptRes );	
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );

		HTTP_INFO_IN_JSON( ptResponse->szBody, sizeof(ptResponse->szBody),
				ATTR_ID, nId, ATTR_NAME, pszName, ATTR_GENDER, pszGender,
				ATTR_BIRTH, pszBirth, ATTR_ADDRESS, pszAddress );
	}

	LOG_DBG_F( "\n%s", ptResponse->szBody );

	ptResponse->nContentLength = (int)strlen(ptResponse->szBody);

	DB_FREE( ptRes );
	return RAS_rHttpOK;

end_of_function:
	DB_FREE( ptRes );
	return nRC;
}
