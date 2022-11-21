/* RAS_Http.c */
#include "RAS_Inc.h"

int HTTP_ReadHeader( int nFd, struct HTTP_REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	int nRC = 0;
	int nRead = 0;
	char szBuf[50];
	char *pszSearch = NULL;

	while ( 1 )
	{
		memset( szBuf, 0x00, sizeof(szBuf) );

		nRead = read( nFd, szBuf, sizeof(szBuf) - 1 ); //NULL 공간 포함 사이즈
		if ( -1 == nRead )
		{
			LOG_ERR_F( "read fail <%d:%s>", errno, strerror(errno) );
			return RAS_rErrHttpRead;
		}
		else if ( 0 == nRead )
		{
			LOG_DBG_F( "read zero" );
			return RAS_rErrHttpRead;
		}

		//strlcat( dst, src, dstsize )
		//dstsize: dst길이+src길이+NULL공간
		nRC = strlcat( ptRequest->szHeader, szBuf, sizeof(ptRequest->szHeader) + 1 );
		if ( (int)sizeof(ptRequest->szHeader) + 1 <= nRC )
		{
			LOG_ERR_F( "output string has been truncated" );
			return RAS_rErrOverflow;
		}

		pszSearch = strstr( ptRequest->szHeader, HTTP_DELIM_CRLFCRLF );
		if ( NULL != pszSearch )
		{
			//LOG_DBG_F( "found <\\r\\n\\r\\n>" );
			break;
		}
	}

	LOG_DBG_F( "\n%s", ptRequest->szHeader );
	return RAS_rOK;
}

int HTTP_GetMethodAndPath( struct HTTP_REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	char szCopyBuf[sizeof(ptRequest->szHeader)];
	memset( szCopyBuf, 0x00, sizeof(szCopyBuf) );

	strlcpy( szCopyBuf, ptRequest->szHeader, sizeof(szCopyBuf) ); 

	//POST /user
	//GET /user/7
	//GET /user
	//DELETE user/5
	pszToken = strtok_r( szCopyBuf, HTTP_DELIM_SPACE, &pszDefaultToken );
	if ( NULL == pszToken )
	{
		LOG_ERR_F( "not found <(space)>" );
		return RAS_rErrFail;
	}

	while ( NULL != pszToken )
	{
		strlcpy( ptRequest->szMethod, pszToken, sizeof(ptRequest->szMethod) );
		pszToken = strtok_r( NULL, HTTP_DELIM_SPACE, &pszDefaultToken );
		strlcpy( ptRequest->szPath, pszToken, sizeof(ptRequest->szPath) );
		break;
	}
	
	LOG_DBG_F( "%s%s", ptRequest->szMethod, ptRequest->szPath );
	return RAS_rOK;
}

int HTTP_GetContentLength( struct HTTP_REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	char *pszSearch = NULL;
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	char szCopyBuf[sizeof(ptRequest->szHeader)];
	memset( szCopyBuf, 0x00, sizeof(szCopyBuf) );
	
	strlcpy( szCopyBuf, ptRequest->szHeader, sizeof(szCopyBuf) ); 

	pszSearch = strstr( szCopyBuf, HTTP_DELIM_CONTENTLENGTH );
	if ( NULL == pszSearch )
	{
		LOG_ERR_F( "not found <\"Content-Length:\">" );
		return RAS_rErrFail;
	}
	else
	{
		pszSearch = pszSearch + strlen( HTTP_DELIM_CONTENTLENGTH );
	
		pszToken = strtok_r( pszSearch, HTTP_DELIM_CRLF, &pszDefaultToken );
		if ( NULL == pszToken )
		{
			LOG_ERR_F( "not found <\\r\\n>" );
			return RAS_rErrFail;
		}
		else
		{
			ptRequest->nContentLength = atoi( pszToken );
			if ( 0 == ptRequest->nContentLength )
			{
				LOG_ERR_F( "atoi fail <%d>", ptRequest->nContentLength );
				return RAS_rErrFail;
			}
		}
	}

	LOG_DBG_F( "%d", ptRequest->nContentLength );
	return RAS_rOK;
}

int HTTP_ReadBody( int nFd, struct HTTP_REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	int nRC = 0;
	int nRead = 0;
	char *pszSearch = NULL;
	char szBuf[50];
	char szCopyBuf[sizeof(ptRequest->szHeader)];
	memset( szCopyBuf, 0x00, sizeof(szCopyBuf) );

	strlcpy( szCopyBuf, ptRequest->szHeader, sizeof(szCopyBuf) ); 
	
	pszSearch = strstr( szCopyBuf, HTTP_DELIM_CRLFCRLF );
	if ( NULL == pszSearch )
	{
		LOG_ERR_F( "not found <\\r\\n\\r\\n>" );
		return RAS_rErrFail;
	}

	pszSearch += strlen(HTTP_DELIM_CRLFCRLF);

	strlcpy( ptRequest->szBody, pszSearch, sizeof(ptRequest->szBody) );

	while ( 1 )
	{
		memset( szBuf, 0x00, sizeof(szBuf) );
		
		nRead = read( nFd, szBuf, sizeof(szBuf) - 1 );
		if ( -1 == nRead )
		{
			LOG_ERR_F( "read fail <%d:%s>", errno, strerror(errno) );
			return RAS_rErrHttpRead;
		}
		else if ( 0 == nRead )
		{
			LOG_DBG_F( "read zero" );
			return RAS_rErrHttpRead;
		}

		nRC = strlcat( ptRequest->szBody, szBuf, sizeof(ptRequest->szBody) + 1 );
		if ( (int)sizeof(ptRequest->szBody) + 1 <= nRC )
		{
			LOG_ERR_F( "output string has been truncated" );
			return RAS_rErrOverflow;
		}
		
		if ( ptRequest->nContentLength == (int)strlen(ptRequest->szBody) )
		{
			break;
		}
	}

	LOG_DBG_F( "\n%s", ptRequest->szBody );
	return RAS_rOK;
}

int HTTP_GetStatusCode( int nRC )
{
	switch( nRC )
	{
		RC_TO_CODE( STATUS_CODE_200, RAS_rOK );
		RC_TO_CODE( STATUS_CODE_201, RAS_rHttpCreated );
		RC_TO_CODE( STATUS_CODE_400, RAS_rErrHttpBadRequest );
		RC_TO_CODE( STATUS_CODE_404, RAS_rErrDBNotFound);
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrDBSetValue );
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrDBExecUpdate );
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrRegiGetEnumKeyValue );
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrRegiNotFound );
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrFail );
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrOverflow );
		RC_TO_CODE( STATUS_CODE_500, RAS_rErrIpcSend );
		RC_TO_CODE_DFLT;
	}
}

char* HTTP_GetStatusMsg( int nCode )
{
	switch( nCode )
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

int HTTP_SendResponse( int nFd, void *pvBuf, int nBufSize )
{
	CHECK_PARAM_RC( pvBuf );

	int nWrite = 0;
	int nTotalWrite = 0;

	while ( 1 )
	{
		nWrite = write( nFd, pvBuf + nTotalWrite, nBufSize - nTotalWrite );
		if ( -1 == nWrite )
		{
			LOG_ERR_F( "write (Fd %d) fail <%d>", nFd, nWrite );
			return RAS_rErrHttpWrite;
		}
		
		nTotalWrite += nWrite;

		if ( nBufSize == nTotalWrite )
		{
			printf( "nBufSize == nTotalWrite (%d)", nBufSize );
			break;
		}
		else if ( 0 == nTotalWrite )
		{
			LOG_DBG_F( "write (Fd %d) zero", nFd );
			return RAS_rErrFail;
		}
	}

	return RAS_rOK;
}
