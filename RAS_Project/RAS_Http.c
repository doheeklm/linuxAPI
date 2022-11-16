/* RAS_Http.c */
#include "RAS_Inc.h"

int HTTP_ReadHeader( int nFd, char *pszRequestHeader, int nHeaderSize )
{
	int nByteRead = 0;
	int nTotalByteRead = 0;
	char *pszSearch = NULL;
	char szBuf[50];

	while ( 1 )
	{
		memset( szBuf, 0x00, sizeof(szBuf) );

		nByteRead = read( nFd, szBuf, sizeof(szBuf) );
		if ( -1 == nByteRead )
		{
			LOG_ERR_F( "read (Fd %d) fail <%d>", nFd, nByteRead );
			return RAS_rErrHttpRead;
		}
		else if ( 0 == nByteRead )
		{
			return RAS_rErrHttpRead;
		}

		nTotalByteRead += nByteRead;
		if ( nTotalByteRead > nHeaderSize )
		{
			return RAS_rErrOverflow;
		}

		strlcat( pszRequestHeader, szBuf, nByteRead );

		pszSearch = strstr( pszRequestHeader, HTTP_DELIM_CRLFCRLF );
		if ( NULL != pszSearch )
		{
			break;
		}
	}

	LOG_DBG_F( "<Header> %s", pszRequestHeader );

	return RAS_rOK;
}

int HTTP_GetMethodAndPath( const char *pszRequestHeader, char *pszRequestMethod,
		int nMethodSize, char *pszRequestPath, int nPathSize )
{
	CHECK_PARAM_RC( pszRequestHeader );

	char *pszSearch = NULL;
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	/*
	 *	GET 127.0.0.1:8080/users/33\r\nContent-Type:....
	 */
	pszSearch = strstr( pszRequestHeader, HTTP_DELIM_CRLF );
	if ( NULL == pszSearch )
	{
		return RAS_rErrFail;
	}

	pszToken = strtok_r( pszSearch, HTTP_DELIM_SPACE, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		/*
		 *	GET
		 */
		strlcpy( pszRequestMethod, pszToken, nMethodSize );
		
		/*
		 *	127.0.0.1:8080/users/33
		 */
		pszToken = strtok_r( NULL, HTTP_DELIM_SPACE, &pszDefaultToken );
		strlcpy( pszRequestPath, pszToken, nPathSize );

		break;
	}

	LOG_DBG_F( "<Method> %s <Path> %s", pszRequestMethod, pszRequestPath );

	return RAS_rOK;
}

int HTTP_GetContentLength( const char *pszRequestHeader, int *pnContentLength )
{
	CHECK_PARAM_RC( pszRequestHeader );
	CHECK_PARAM_RC( pnContentLength );

	char *pszSearch = NULL;
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	/*
	 *	Content-Length: 86\r\n
	 */
	pszSearch = strstr( pszRequestHeader, HTTP_DELIM_CONTENTLENGTH );
	if ( NULL == pszSearch )
	{
		return RAS_rErrFail;
	}
	else
	{
		/*
		 *	86\r\n
		 */	
		pszSearch = pszSearch + strlen( HTTP_DELIM_CONTENTLENGTH );
	
		/*
		 *	atoi(86)
		 */
		pszToken = strtok_r( pszSearch, HTTP_DELIM_CRLF, &pszDefaultToken );
	
		*pnContentLength = atoi( pszToken );
		if ( 0 == *pnContentLength )
		{
			return RAS_rErrFail;
		}
	
		LOG_DBG_F( "<Content-Length> %d", *pnContentLength );	
	}

	return RAS_rOK;
}

int HTTP_ReadBody( int nFd, char *pszRequestHeader, int nContentLength, char *pszRequestBody, int nBodySize )
{
	CHECK_PARAM_RC( pszRequestHeader );

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;

	int nByteRead = 0;
	int nTotalByteRead = 0;
	char szBuf[50];

	/*
	 *	Content-Length: 86\r\nContent-Type: application/json\r\n\r\n{ "Name" :....
	 */
	pszToken = strtok_r( pszRequestHeader, HTTP_DELIM_CRLFCRLF, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		pszToken = strtok_r( NULL, HTTP_DELIM_CRLFCRLF, &pszDefaultToken );

		/*
		 *	{ "Name" :
		 */
		strlcpy( pszRequestBody, pszToken, nBodySize );
		break;
	}

	while ( 1 )
	{
		memset( szBuf, 0x00, sizeof(szBuf) );
		
		nByteRead = read( nFd, szBuf, sizeof(szBuf) );
		if ( -1 == nByteRead )
		{
			LOG_ERR_F( "read (Fd %d) fail <%d>", nFd, nByteRead );
			return RAS_rErrHttpRead;
		}
		else if ( 0 == nByteRead )
		{
			return RAS_rErrHttpRead;
		}

		nTotalByteRead += nByteRead;
		if ( nTotalByteRead > nBodySize )
		{
			return RAS_rErrOverflow;
		}

		strlcat( pszRequestBody, szBuf, nByteRead );
		
		if ( (unsigned int)nContentLength == strlen(pszRequestBody) )
		{
			break;
		}
	}

	LOG_DBG_F( "<Body>\n%s", pszRequestBody );

	return RAS_rOK;
}

int HTTP_ProcessRequestMsg( const char *pszRequestMethod, char *pszRequestPath, char *pszRequestBody, DB_t tDBWorker, char *pszResponseBody, int nResponseBodySize )
{	
	CHECK_PARAM_RC( pszRequestMethod );
	CHECK_PARAM_RC( pszRequestPath );
	CHECK_PARAM_RC( pszRequestBody );
	CHECK_PARAM_RC( tDBWorker.ptDBConn );
	CHECK_PARAM_RC( tDBWorker.patPstmt );

	int nRC = 0;
	char *pszSearch = NULL;
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	USER_t tUser;
	memset( &tUser, 0x00, sizeof(tUser) );

	//NOTE 
	pszResponseBody = pszResponseBody;
	nResponseBodySize = nResponseBodySize;

	//pszPath : 127.0.0.1:8080/users/70
	pszSearch = strchr( pszRequestPath, '/' );
	if ( NULL == pszSearch )
	{
		return RAS_rErrHttpBadRequest;
	}

	//pszSearch : /users/70
	pszToken = strtok_r( pszSearch, HTTP_DELIM_SLASH, &pszDefaultToken );
	while ( NULL != pszToken )
	{
		//pszToken : users
		if ( 0 == strcmp( USERS_DIR, pszToken ) )
		{
			return RAS_rErrHttpBadRequest;
		}

		//pszToken : 70
		pszToken = strtok_r( NULL, HTTP_DELIM_SLASH, &pszDefaultToken );
		if ( NULL != pszToken )
		{
			tUser.nId = atoi( pszToken );
			//TODO 0 >=
			if ( 0 == tUser.nId || INT_MAX < tUser.nId )
			{
				return RAS_rErrHttpBadRequest;
			}
		}

		break;
	}
	
	if ( 0 == strcmp( HTTP_METHOD_POST, pszRequestMethod ) )
	{
		char aszToken[8][256];
		memset( aszToken, 0x00, sizeof(aszToken) );
		int nCnt = 0;
		int nIndex = 0;

		//TODO METHOD_
		pszToken = strtok_r( pszRequestBody, HTTP_DELIM_QUOTATION, &pszDefaultToken );
		while ( NULL != pszToken )
		{
			if ( ODD_NUMBER == nCnt % 2 )
			{
				if ( ( strlen(pszToken) > sizeof(aszToken[nIndex]) ) ||
				  	 ( 0 == strlen(pszToken) ) )
				{
					return RAS_rErrHttpBadRequest;
				}
				strlcpy( aszToken[nIndex], pszToken, sizeof(aszToken[nIndex]) );
				
				nIndex++;
				if ( MAX_INDEX <= nIndex )
				{
					break;
				}
			}
			pszToken = strtok_r( NULL, HTTP_DELIM_QUOTATION, &pszDefaultToken );
			nCnt++;
		}

		IF_EQUAL_STRLCPY( aszToken[0], aszToken[1], ATTR_NAME, tUser.szName, sizeof(tUser.szName) );
		IF_EQUAL_STRLCPY( aszToken[2], aszToken[3], ATTR_GENDER, tUser.szGender, sizeof(tUser.szGender) );
		IF_EQUAL_STRLCPY( aszToken[4], aszToken[5], ATTR_BIRTH, tUser.szBirth, sizeof(tUser.szBirth) );
		IF_EQUAL_STRLCPY( aszToken[6], aszToken[7], ATTR_ADDRESS, tUser.szAddress, sizeof(tUser.szAddress) );

		DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_NAME, tUser.szName, nRC );
		DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_GENDER, tUser.szGender, nRC );
		DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_BIRTH, tUser.szBirth, nRC );
		DB_SET_STRING_BY_KEY( tDBWorker.patPstmt[PSTMT_INSERT_INFO], ATTR_ADDRESS, tUser.szAddress, nRC );

		DB_PREPARED_EXEC_UPDATE( tDBWorker, tDBWorker.patPstmt[PSTMT_INSERT_INFO], nRC );

		return RAS_rHttpCreated;
	}
	else if ( 0 == strcmp( HTTP_METHOD_GET, pszRequestMethod ) )
	{
		//TODO METHOD
		DAL_RESULT_SET *ptRes = NULL;
		DAL_ENTRY *ptEntry = NULL;
		int nCntTuple = 0;
		char *pszId = NULL;
		char *pszName = NULL;
		char *pszGender = NULL;
		char *pszBirth = NULL;
		char *pszAddress = NULL;

		if ( 0 == tUser.nId )
		{
			DB_PREPARED_EXEC( tDBWorker, tDBWorker.patPstmt[PSTMT_SELECT_INFO_ALL], &ptRes, nRC );
			
			for ( ptEntry = dalFetchFirst( ptRes ); ptEntry != NULL; ptEntry = dalFetchNext( ptRes ) )
			{
				DB_GET_STRING_BY_KEY( ptEntry, ATTR_ID, &pszId, nRC );
				DB_GET_STRING_BY_KEY( ptEntry, ATTR_NAME, &pszName, nRC );
				DB_GET_STRING_BY_KEY( ptEntry, ATTR_GENDER, &pszGender, nRC );
				DB_GET_STRING_BY_KEY( ptEntry, ATTR_BIRTH, &pszBirth, nRC );
				DB_GET_STRING_BY_KEY( ptEntry, ATTR_ADDRESS, &pszAddress, nRC );
				nCntTuple++;

				//TODO tUser.nId, tUser.szName, tUser.szGender, tUser.szBirth, tUser.szAddress 에 넣고
				//json 포맷으로 작성해야함 MAcro.h
			}
	
		}
		else
		{
		}

		//DB Select 후 Json 포맷으로
	}
	else if ( 0 == strcmp( HTTP_METHOD_DELETE, pszRequestMethod ) )
	{
		//UserID로 DB Delete
	}
	else
	{
		return RAS_rErrHttpMethodNotAllowed;
	}

	return RAS_rOK;

end_of_function:

	return nRC;
}

char* HTTP_GetStatusMsg( int nStatusCode )
{
	switch( nStatusCode )
	{
		STR_CASE( STATUS_CODE_200, STATUS_MSG_200 );
		STR_CASE( STATUS_CODE_201, STATUS_MSG_201 );
		STR_CASE( STATUS_CODE_400, STATUS_MSG_400 );
		STR_CASE( STATUS_CODE_404, STATUS_MSG_404 );
		STR_CASE( STATUS_CODE_405, STATUS_MSG_405 );
		STR_CASE( STATUS_CODE_500, STATUS_MSG_500 );
		
		STR_CASE_DFLT_UKN;
	}
}

int HTTP_SendResponseMsg( int nFd, char *pszBuf, int nBufSize )
{
	int nByteWrite = 0;
	int nTotalByteWrite = 0;

	while ( 1 )
	{
		nByteWrite = write( nFd, pszBuf + nTotalByteWrite, nBufSize - nTotalByteWrite );
		if ( -1 == nByteWrite )
		{
			LOG_ERR_F( "write (Fd %d) fail <%d>", nFd, nByteWrite );
			return RAS_rErrHttpWrite;
		}
		
		nTotalByteWrite += nByteWrite;

		if ( nBufSize == nTotalByteWrite )
		{
			break;
		}
		else if ( 0 == nTotalByteWrite )
		{
			LOG_DBG_F( "write (Fd %d) 0", nFd );
			return RAS_rErrFail;
		}
	}

	return RAS_rOK;
}
