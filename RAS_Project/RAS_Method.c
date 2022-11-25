/* RAS_Method.c */
#include "RAS_Inc.h"

extern int				g_nUser;
extern pthread_mutex_t	g_tMutex;

int METHOD_Post( DB_t tDBWorker, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );

	int i = 0;
	int nRC = 0;
	int nCnt = 0;
	int nIndex = 0;
	int bInsert[TOKEN_MAX] = { NOT_INSERT, };
	char *pszInput = NULL;
	char *pszDefaultToken = NULL;
	char aszToken[MAX_TOKEN][256];
	memset( aszToken, 0x00, sizeof(aszToken) );

	nRC = UTIL_CheckPath( ptRequest->szPath );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}

	/*
	 *	{
	 *		"name": "Dohee",
	 *		"gender": "FEMALE",
	 *		"birth": "950101",
	 *		"address": "Korea"
	 *	}
	 *
	 *	Token by Quotation
	 *	{"name": "Dohee", "gender": "FEMALE", "birth": "950101", "address": "Korea" }
	 *
	 *	EVEN_NUM : {
	 *	ODD _NUM : name		| name -> aszToken[0]
	 *	EVEN_NUM : :
	 *	ODD _NUM : Dohee	| Dohee -> aszToken[1]
	 */

	pszInput = strtok_r( ptRequest->szBody, HTTP_DELIM_QUOTATION, &pszDefaultToken );
	while ( NULL != pszInput )
	{
		if ( ODD_NUM == nCnt % TWO )
		{ 
			if ( strlen(pszInput) >= sizeof(aszToken[nIndex]) )
			{
				/*
				 *	Input				: name
				 *	strlen(Input)		: 4
				 *	sizeof(Token[0])	: 5
				 *
				 *	strlcpy( dst, src, dstsize )
				 *	=> dstsize - 1 만큼 복사가 이루어짐
				 *
				 *	strlcpy( Token[0], Input, sizeof(Token[0]) )
				 *	=> Token[0]에 Input의 4만큼을 복사함
				 */
				LOG_ERR_F( "size overflow" );
				return RAS_rErrHttpBadRequest;
			}

			strlcpy( aszToken[nIndex], pszInput, sizeof(aszToken[nIndex]) );
			
			//LOG_DBG_F( "Token[%d] : %s", nIndex, aszToken[nIndex] );

			nIndex++;

			if ( MAX_TOKEN < nIndex )
			{
				LOG_ERR_F( "Client input more than MAX_TOKEN" );
				return RAS_rErrHttpBadRequest;
			}
		}

		pszInput = strtok_r( NULL, HTTP_DELIM_QUOTATION, &pszDefaultToken );

		nCnt++;
	}

	/*
	 *	"name" "gender" "birth" "address"
	 *	중복 또는 하나라도 입력 받지 않은 경우 400 리턴
	 */
	for ( i = 0; i < nIndex - 1; i++ )
	{
		if ( EVEN_NUM == i % TWO )
		{
			//LOG_DBG_F( "<Attr> Token[%d] : %s", i, aszToken[i] );

			if ( 0 == strcmp( ATTR_NAME, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_NAME] )
				{
					HTTP_VALID_RANGE( strlen(aszToken[i + 1]), LENGTH_ONE, SIZE_NAME );
					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
					bInsert[TOKEN_NAME] = INSERT;
				}
				else
				{
					LOG_ERR_F( "name already inserted" );
					return RAS_rErrHttpBadRequest;
				}
			}
			else if ( 0 == strcmp( ATTR_GENDER, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_GENDER] )
				{
					if ( 0 != strcmp( ARG_STR_MALE, aszToken[i + 1] ) &&
						 0 != strcmp( ARG_STR_FEMALE, aszToken[i + 1] ) )
					{
						LOG_ERR_F( "gender needs to be MALE or FEMALE" );
						return RAS_rErrHttpBadRequest;
					}

					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
					bInsert[TOKEN_GENDER] = INSERT;
				}
				else
				{
					LOG_ERR_F( "gender already inserted" );
					return RAS_rErrHttpBadRequest;
				}
			}
			else if ( 0 == strcmp( ATTR_BIRTH, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_BIRTH] )
				{
					HTTP_VALID_RANGE( strlen(aszToken[i + 1]), LENGTH_ONE, SIZE_BIRTH );
					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
					bInsert[TOKEN_BIRTH] = INSERT;
				}
				else
				{
					LOG_ERR_F( "birth already inserted" );
					return RAS_rErrHttpBadRequest;
				}
			}
			else if ( 0 == strcmp( ATTR_ADDRESS, aszToken[i] ) )
			{
				if ( NOT_INSERT == bInsert[TOKEN_ADDRESS] )
				{
					HTTP_VALID_RANGE( strlen(aszToken[i + 1]), LENGTH_ONE, SIZE_ADDRESS );
					DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], aszToken[i], aszToken[i + 1], nRC );
					bInsert[TOKEN_ADDRESS] = INSERT;
				}
				else
				{
					LOG_ERR_F( "address already inserted" );
					return RAS_rErrHttpBadRequest;
				}
			}
		}
	}

	for ( nIndex = 0; nIndex < TOKEN_MAX; nIndex++ )
	{
		if ( NOT_INSERT == bInsert[nIndex] )
		{
			LOG_ERR_F( "bInsert[%d] == NOT_INSERT", nIndex );
			return RAS_rErrHttpBadRequest;
		}
	}

	DB_PREPARED_EXEC_UPDATE( tDBWorker, tDBWorker.patPstmt[PSTMT_INSERT_INFO], nRC );

	pthread_mutex_lock( &g_tMutex );
	g_nUser++; //Alarm
	pthread_mutex_unlock( &g_tMutex );

	return RAS_rHttpCreated;

end_of_function:
	return nRC;
}

int METHOD_Get( DB_t tDBWorker, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );

	int nRC = 0;
	int nCnt = 0;
	int nTotal = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszGender = NULL;
	char *pszBirth = NULL;
	char *pszAddress = NULL;
	char szBuf[512];
	memset( szBuf, 0x00, sizeof(szBuf) );
	DAL_RESULT_SET *ptResNumtuples = NULL;
	DAL_RESULT_SET *ptResSelect = NULL;
	DAL_ENTRY *ptEntry = NULL;

	nRC = UTIL_GetIdFromPath( ptRequest->szPath, &nId );
	if ( RAS_rOK != nRC )
	{
		return nRC;
	}
	
	if ( 0 == nId )
	{
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_NUMTUPLES_INFO], &ptResNumtuples, nRC );
	
		ptEntry = dalFetchFirst( ptResNumtuples );	
		if ( NULL != ptEntry )
		{
			DB_GET_INT_BY_KEY( ptEntry, NUMTUPLES, &nTotal, nRC );
		}
		else
		{
			nRC = RAS_rErrDBFetch; 
			goto end_of_function;
		}
		
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_ALL], &ptResSelect, nRC );

		STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, HTTP_JSON_INFO_BEGIN, sizeof(ptResponse->szBody), nRC );

		for ( ptEntry = dalFetchFirst( ptResSelect ); ptEntry != NULL; ptEntry = dalFetchNext( ptResSelect ) )
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
			
			nCnt++;
		
			if ( nCnt < nTotal )
			{
				STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, HTTP_JSON_INFO_AND, sizeof(ptResponse->szBody), nRC );
			}
		}

		STRLCAT_OVERFLOW_CHECK( ptResponse->szBody, HTTP_JSON_INFO_END, sizeof(ptResponse->szBody), nRC );
	}
	else
	{
		DB_SET_INT_BY_KEY( tDBWorker.patPstmt[PSTMT_SELECT_INFO_BY_ID], ATTR_ID, nId, nRC );
		DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_BY_ID], &ptResSelect, nRC );
		
		ptEntry = dalFetchFirst( ptResSelect );	
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

		HTTP_JSON_INFO( ptResponse->szBody, sizeof(ptResponse->szBody), ATTR_ID, nId,
			   	ATTR_NAME, pszName, ATTR_GENDER, pszGender,
				ATTR_BIRTH, pszBirth, ATTR_ADDRESS, pszAddress );
	}

	LOG_DBG_F( "\n%s\n%s\n%s", LINE, ptResponse->szBody, LINE );

	ptResponse->nContentLength = (int)strlen(ptResponse->szBody);
	
	DB_FREE( ptResNumtuples );
	DB_FREE( ptResSelect );
	return RAS_rHttpOK;

end_of_function:
	DB_FREE( ptResNumtuples );
	DB_FREE( ptResSelect );
	return nRC;
}

int METHOD_Delete( DB_t tDBWorker, struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );
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
	
	pthread_mutex_lock( &g_tMutex );
	g_nUser--; //Alarm
	pthread_mutex_unlock( &g_tMutex );

	return RAS_rHttpOK;

end_of_function:
	return nRC;
}
