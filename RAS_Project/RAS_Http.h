/* RAS_Http.h */
#ifndef _RAS_HTTP_H
#define _RAS_HTTP_H

#define HTTP_TYPE_REQUEST			0
#define HTTP_TYPE_RESPONSE			1

#define USER_DIR					"user"
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

int HTTP_ReadHeader( int nFd, struct HTTP_REQUEST_s *ptRequest );
int HTTP_GetMethodAndPath( struct HTTP_REQUEST_s *ptRequest );
int HTTP_GetContentLength( struct HTTP_REQUEST_s *ptRequest );
int HTTP_ReadBody( int nFd, struct HTTP_REQUEST_s *ptRequest );

int HTTP_GetStatusCode( int nRC );
char* HTTP_GetStatusMsg( int nCode );
int HTTP_SendResponse( int nFd, void *pvBuf, int nBufSize );

#endif /* _RAS_HTTP_H_ */
