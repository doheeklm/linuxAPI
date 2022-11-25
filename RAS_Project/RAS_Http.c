/* RAS_Http.c */
#include "RAS_Inc.h"

int HTTP_ReadHeader( int nFd, struct REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	int nRC = 0;
	int nRead = 0;
	char szBuf[50];
	char *pszSearch = NULL;

	while ( 1 )
	{
		memset( szBuf, 0x00, sizeof(szBuf) );

		//TODO 소켓이 nonblock block인지 확인해야함
		//nonblock인지 block인지에 따라 계속 지연될 경우 연결 해제할건지 다름
		nRead = read( nFd, szBuf, sizeof(szBuf)-1 );
		if ( -1 == nRead )
		{
			LOG_ERR_F( "read fail <%d:%s>", errno, strerror(errno) );
			return RAS_rErrHttpRead;
		}
		else if ( 0 == nRead )
		{
			LOG_DBG_F( "read End Of File" );
			return RAS_rErrHttpRead;
		}

		LOG_DBG_F( "read (%d)", nRead );

		STRLCAT_OVERFLOW_CHECK( ptRequest->szHeader, szBuf, sizeof(ptRequest->szHeader), nRC );

		/*
		 *	"\r\n\r\n" 검색
		 */
		pszSearch = strstr( ptRequest->szHeader, HTTP_DELIM_CRLFCRLF );
		if ( NULL != pszSearch )
		{
			break;
		}
	}

	LOG_DBG_F( "\n%s\n%s\n%s", LINE, ptRequest->szHeader, LINE );
	return RAS_rOK;

end_of_function:
	return nRC;
}

int HTTP_GetMethodAndPath( struct REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	char szCopy[sizeof(ptRequest->szHeader)];
	memset( szCopy, 0x00, sizeof(szCopy) );

	strlcpy( szCopy, ptRequest->szHeader, sizeof(szCopy) ); 

	/*
	 *	스페이스 토큰화
	 */
	pszToken = strtok_r( szCopy, HTTP_DELIM_SPACE, &pszDefaultToken );
	if ( NULL == pszToken )
	{
		LOG_ERR_F( "not found <(space)>" );
		return RAS_rErrFail;
	}

	while ( NULL != pszToken )
	{
		//Method POST
		strlcpy( ptRequest->szMethod, pszToken, sizeof(ptRequest->szMethod) );
		
		pszToken = strtok_r( NULL, HTTP_DELIM_SPACE, &pszDefaultToken );
		if ( NULL == pszToken )
		{
			LOG_ERR_F( "not found <(space>" );
			return RAS_rErrFail;
		}
		
		//Path /user
		strlcpy( ptRequest->szPath, pszToken, sizeof(ptRequest->szPath) );
		
		break;
	}
	
	LOG_DBG_F( "%s %s", ptRequest->szMethod, ptRequest->szPath );
	return RAS_rOK;
}

int HTTP_GetContentLength( struct REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	char *pszSearch = NULL;
	char *pszToken = NULL;
	char *pszDefaultToken = NULL;
	char szCopy[sizeof(ptRequest->szHeader)];
	memset( szCopy, 0x00, sizeof(szCopy) );
	
	strlcpy( szCopy, ptRequest->szHeader, sizeof(szCopy) ); 

	/*
	 *	"Content-Length: " 검색
	 */
	pszSearch = strstr( szCopy, HTTP_DELIM_CONTENTLENGTH );
	if ( NULL == pszSearch )
	{
		ptRequest->nContentLength = 0;
		goto end_of_function;
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
		}
	}

end_of_function:
	LOG_DBG_F( "%d", ptRequest->nContentLength );
	return RAS_rOK;
}

int HTTP_ReadBody( int nFd, struct REQUEST_s *ptRequest )
{
	CHECK_PARAM_RC( ptRequest );

	int nRC = 0;
	int nRead = 0;
	char *pszSearch = NULL;
	char szBuf[50];
	char szCopy[sizeof(ptRequest->szHeader)];
	memset( szCopy, 0x00, sizeof(szCopy) );

	if ( 0 == ptRequest->nContentLength )
	{
		return RAS_rOK;
	}

	strlcpy( szCopy, ptRequest->szHeader, sizeof(szCopy) ); 

	/*
	 *	"\r\n\r\n" 검색
	 */
	pszSearch = strstr( szCopy, HTTP_DELIM_CRLFCRLF );
	if ( NULL == pszSearch )
	{
		LOG_ERR_F( "not found <\\r\\n\\r\\n>" );
		return RAS_rErrFail;
	}

	/*
	 *	"\r\n\r\n" 이후로 위치 옮기기
	 */
	pszSearch += strlen(HTTP_DELIM_CRLFCRLF);

	strlcpy( ptRequest->szBody, pszSearch, sizeof(ptRequest->szBody) );

	LOG_DBG_F( "read (%d)", (int)strlen(ptRequest->szBody) );

	//TODO 실제 읽은 byte count를 하지 않고 큰 Buffer size를 줬을 때 문제는
	//만약 POST GET 두 request가 연속해서 TCP buffer에 있으면,
	//두 request를 다 읽어버릴 수 있기 때문에
	//후속 request 처리 로직이 없다면 POST만 처리하고 GET은 처리 못하는 문제점이 있음
	//NOTE TCP buffer에서 read 함수로 내부 buffer를 읽어버리면, 다음 epoll에서 EPOLL_IN이 뜨지 않음
	//참고 자료: https://www.linuxtoday.com/blog/blocking-and-non-blocking-i-0/
	
	while ( 1 )
	{
		memset( szBuf, 0x00, sizeof(szBuf) );
	
		//TODO contentLength가 잘못된 값으로 오면 반복적으로 read 함
		if ( (int)strlen(ptRequest->szBody) == ptRequest->nContentLength )
		{
			LOG_DBG_F( "read all body (%d)", ptRequest->nContentLength );
			break;
		}
	
		nRead = read( nFd, szBuf, sizeof(szBuf)-1 );
		if ( -1 == nRead )
		{
			LOG_ERR_F( "read fail <%d:%s>", errno, strerror(errno) );
			return RAS_rErrHttpRead;
		}
		else if ( 0 == nRead )
		{
			LOG_DBG_F( "read End Of File" );
			break;
		}

		LOG_DBG_F( "read (%d)", nRead );

		STRLCAT_OVERFLOW_CHECK( ptRequest->szBody, szBuf, sizeof(ptRequest->szBody), nRC );
	}

	LOG_DBG_F( "\n%s\n%s\n%s", LINE, ptRequest->szBody, LINE );
	return RAS_rOK;

end_of_function:
	return nRC;
}

int HTTP_GetStatusCode( int nRC )
{
	switch( nRC )
	{
		CASE_RETURN( RAS_rOK,						STATUS_CODE_200 );
		CASE_RETURN( RAS_rHttpOK,					STATUS_CODE_200 );
		CASE_RETURN( RAS_rHttpCreated,				STATUS_CODE_201 );
		CASE_RETURN( RAS_rErrHttpBadRequest,		STATUS_CODE_400 );
		CASE_RETURN( RAS_rErrDBNotFound,			STATUS_CODE_404 );
		CASE_RETURN( RAS_rErrHttpMethodNotAllowed,	STATUS_CODE_405 );
		CASE_RETURN( RAS_rErrDBSetValue,			STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrDBExecUpdate,			STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrDBFetch,				STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrDBDuplicate,			STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrRegiGetEnumKeyValue,	STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrRegiNotFound,			STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrInvalidParam,			STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrFail,					STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrOverflow,				STATUS_CODE_500 );
		CASE_RETURN( RAS_rErrIpcSend,				STATUS_CODE_500 );
		CASE_DEFAULT_500;
	}
}

char* HTTP_GetStatusMsg( int nCode )
{
	switch( nCode )
	{
		CASE_RETURN( STATUS_CODE_200, STATUS_MSG_200 );
		CASE_RETURN( STATUS_CODE_201, STATUS_MSG_201 );
		CASE_RETURN( STATUS_CODE_400, STATUS_MSG_400 );
		CASE_RETURN( STATUS_CODE_404, STATUS_MSG_404 );
		CASE_RETURN( STATUS_CODE_405, STATUS_MSG_405 );
		CASE_RETURN( STATUS_CODE_500, STATUS_MSG_500 );
		CASE_DEFAULT_UNKNOWN;
	}
}

int HTTP_SendResponse( int nFd, void *pvBuf )
{
	CHECK_PARAM_RC( pvBuf );

	int nByteToWrite = 0;
	int nWrite = 0;
	int nTotal = 0;

	nByteToWrite = (int)strlen((char*)pvBuf);

	while ( 1 )
	{
		nWrite = write( nFd, pvBuf + nTotal, nByteToWrite - nTotal );
		if ( -1 == nWrite )
		{
			LOG_ERR_F( "write (fd %d) fail <%d:%s>", nFd, errno, strerror(errno) );
			return RAS_rErrHttpWrite;
		}
		else if ( 0 == nWrite )
		{
			LOG_DBG_F( "write End Of File" );
			break;
		}

		nTotal += nWrite;

		LOG_DBG_F( "write (%d) total (%d)", nWrite, nTotal );
	}

	LOG_DBG_F( "\n%s\n%s\n%s", LINE, (char*)pvBuf, LINE );

	return RAS_rOK;
}
