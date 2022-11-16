/* RAS_Http.h */
#ifndef _RAS_HTTP_H
#define _RAS_HTTP_H

#define USERS_DIR					"users"
#define ODD_NUMBER					1
#define MAX_INDEX					8

#define HTTP_DELIM_CRLF				"\r\n"
#define HTTP_DELIM_CRLFCRLF			"\r\n\r\n"
#define HTTP_DELIM_SPACE			" "
#define HTTP_DELIM_CONTENTLENGTH	"Content-Length: "
#define HTTP_DELIM_SLASH			"/"
#define HTTP_DELIM_QUOTATION		"\""

#define HTTP_METHOD_POST			"POST"
#define HTTP_METHOD_GET				"GET"
#define HTTP_METHOD_DELETE			"DELETE"

#define STATUS_CODE_200				200
#define STATUS_CODE_201				201
#define STATUS_CODE_400				400
#define STATUS_CODE_404				404
#define STATUS_CODE_405				405
#define STATUS_CODE_500				500
#define STATUS_CODE_NONE			0

#define STATUS_MSG_200				"OK"
#define STATUS_MSG_201				"Created"
#define STATUS_MSG_400				"Bad Request"
#define STATUS_MSG_404				"Not Found"
#define STATUS_MSG_405				"Method Not Allowed"
#define STATUS_MSG_500				"Internal Server Error"

int HTTP_ReadHeader( int nFd, char *pszRequestHeader, int nHeaderSize );

int HTTP_GetMethodAndPath( const char *pszRequestHeader, char *pszRequestMethod, int nMethodSize,
		char *pszRequestPath, int nPathSize );

int HTTP_GetContentLength( const char *pszRequestHeader, int *pnContentLength );

int HTTP_ReadBody( int nFd, char *pszRequestHeader, int nContentLength, char *pszRequestBody, int nBodySize );

int HTTP_ProcessRequestMsg( const char *pszRequestMethod, char *pszRequestPath, char *pszRequestBody, DB_t tDBWorker, char *pszResponseBody, int nResponseBodySize );

char* HTTP_GetStatusMsg( int nStatusCode );

int HTTP_SendResponseMsg( int nFd, char *pszBuf, int nBufSize );

#endif /* _RAS_HTTP_H_ */
