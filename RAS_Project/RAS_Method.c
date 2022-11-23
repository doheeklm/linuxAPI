/* RAS_Method.c */
#include "RAS_Inc.h"

extern int g_nUser;

int METHOD_Post( DB_t tDBWorker, const char *pszIp,
		struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( pszIp );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );

	int i = 0;
	int nRC = 0, nCnt = 0, nIndex = 0, nAttrEqual = 0;
	int bInsert[TOKEN_MAX] = { NOT_INSERT, };
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	char aszToken[MAX_TOKEN][256];
	memset( aszToken, 0x00, sizeof(aszToken) );

	nRC = UTIL_CheckPath( ptRequest->szPath );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}
	
	/*
	 *	큰 따옴표로 나누어서 홀수 토큰만 저장함
	 */
	pszToken = strtok_r( ptRequest->szBody, HTTP_DELIM_QUOTATION, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		if ( ODD_NUM == nCnt % TWO )
		{
			/*
			 *	토큰의 길이가 0이거나 너무 긴 경우 에러 리턴
			 */
			if ( ( strlen(pszToken) > sizeof(aszToken[nIndex]) ) ||
				 ( 0 == strlen(pszToken) ) )
			{
				return RAS_rErrHttpBadRequest;
			}
			
			strlcpy( aszToken[nIndex], pszToken, sizeof(aszToken[nIndex]) );	
			
			LOG_DBG_F( "<All> Token[%d]... %s", nIndex, aszToken[nIndex] );

			nIndex++;
			/*
			 *	토큰을 최대 32개까지 받음
			 */
			if ( MAX_TOKEN <= nIndex )
			{
				break;
			}
		}

		pszToken = strtok_r( NULL, HTTP_DELIM_QUOTATION, &pszDefaultToken );

		nCnt++;
	}

	for ( i = 0; i < nIndex - 1; i++ )
	{
		if ( EVEN_NUM == i % TWO )
		{
			LOG_DBG_F( "<Attr> Token[%d]... %s", i, aszToken[i] );

			if ( 0 == strcmp( ATTR_NAME, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_NAME] )
				{
					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
				}
				bInsert[TOKEN_NAME] = INSERT;
				nAttrEqual++;
			}
			else if ( 0 == strcmp( ATTR_GENDER, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_GENDER] )
				{
					if ( 0 != strcmp( ARG_STR_MALE, aszToken[i + 1] ) ||
					  	 0 != strcmp( ARG_STR_FEMALE, aszToken[i + 1] ) )
					{
						LOG_ERR_F( "gender needs to be MALE or FEMALE" );
						DB_ROLLBACK( tDBWorker, nRC );
						return RAS_rErrHttpBadRequest;
					}

					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
				}
				bInsert[TOKEN_GENDER] = INSERT;
				nAttrEqual++;
			}
			else if ( 0 == strcmp( ATTR_BIRTH, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_BIRTH] )
				{
					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
				}
				bInsert[TOKEN_BIRTH] = INSERT;
				nAttrEqual++;
			}
			else if ( 0 == strcmp( ATTR_ADDRESS, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_ADDRESS] )
				{
					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
				}
				bInsert[TOKEN_ADDRESS] = INSERT;
				nAttrEqual++;
			}
		}
	}

	/*
	 *	"name" "gender" "birth" "address"
	 *	중복이거나, 입력 받지 못한 경우 에러 리턴
	 */

	LOG_DBG_F( "정확히 입력받은 Attr는 4개 필요 => %d", nAttrEqual );

	if ( ATTR_TOKEN != nAttrEqual )
	{
		DB_ROLLBACK( tDBWorker, nRC );
		return RAS_rErrHttpBadRequest;
	}
	for ( nIndex = 0; nIndex < TOKEN_MAX; nIndex++ )
	{
		if ( NOT_INSERT == bInsert[nIndex] )
		{
			DB_ROLLBACK( tDBWorker, nRC );
			return RAS_rErrHttpBadRequest;
		}
	}

	DB_PREPARED_EXEC_UPDATE( tDBWorker, tDBWorker.patPstmt[PSTMT_INSERT_INFO], nRC );

	g_nUser++;

	return RAS_rHttpCreated;

end_of_function:
	return nRC;
}

int METHOD_Get( DB_t tDBWorker, const char *pszIp,
		struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( pszIp );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );

	int nRC = 0;
	int nCntTuple = 0;
	int nTotalTuple = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszGender = NULL;
	char *pszBirth = NULL;
	char *pszAddress = NULL;
	char szBuf[512];
	memset( szBuf, 0x00, sizeof(szBuf) );
	DAL_RESULT_SET *ptRes = NULL;
	DAL_ENTRY *ptEntry = NULL;

	nRC = UTIL_GetIdFromPath( ptRequest->szPath, &nId );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	if ( 0 == nId )
	{
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_NUMTUPLE_INFO], &ptRes, nRC );
		if ( NULL != ptEntry )
		{
			DB_GET_INT_BY_KEY( ptEntry, NUMTUPLE, &nTotalTuple, nRC );
		}
		
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_ALL], &ptRes, nRC );

		STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, HTTP_JSON_INFO_BEGIN, sizeof(ptResponse->szBody), nRC );

		for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
		{
			DB_GET_INT_BY_KEY( ptEntry, ATTR_ID, &nId, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
			DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );

			HTTP_JSON_INFO( szBuf, sizeof(szBuf), ATTR_ID, nId,
					ATTR_NAME, pszName, ATTR_GENDER, pszGender,
					ATTR_BIRTH, pszBirth, ATTR_ADDRESS, pszAddress );

			STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, szBuf, sizeof(ptResponse->szBody), nRC );
			
			nCntTuple++;
			if ( nCntTuple < nTotalTuple )
			{
				STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, HTTP_JSON_INFO_AND, sizeof(ptResponse->szBody), nRC );
			}
		}

		STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, HTTP_JSON_INFO_END, sizeof(ptResponse->szBody), nRC );

		LOG_DBG_F( "CntTuple %d TotalTuple %d", nCntTuple, nTotalTuple );
	}
	else
	{
		DB_SET_INT_BY_KEY( tDBWorker.patPstmt[PSTMT_SELECT_INFO_BY_ID], ATTR_ID, nId, nRC );
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_BY_ID], &ptRes, nRC );
		
		ptEntry = dalFetchFirst( ptRes );	
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
		DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );

		HTTP_JSON_INFO( ptResponse->szBody, sizeof(ptResponse->szBody), ATTR_ID, nId,
			   	ATTR_NAME, pszName, ATTR_GENDER, pszGender,
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

int METHOD_Delete( DB_t tDBWorker, const char *pszIp,
		struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( pszIp );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );

	int nRC = 0;
	int nId = 0;

	nRC = UTIL_GetIdFromPath( ptRequest->szPath, &nId );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	if ( 0 == nId )
	{
		return RAS_rErrHttpBadRequest;
	}
		
	DB_SET_INT_BY_KEY( tDBWorker.patPstmt[PSTMT_DELETE_INFO], ATTR_ID, nId, nRC );
	DB_PREPARED_EXEC_UPDATE( tDBWorker, tDBWorker.patPstmt[PSTMT_DELETE_INFO], nRC );
	
	g_nUser--;

	return RAS_rHttpOK;

end_of_function:
	return nRC;
}
