/* RAS_Method.h */
#ifndef _RAS_METHOD_H
#define _RAS_METHOD_H

#define EVEN_NUM	0
#define ODD_NUM		1
#define TWO			2

#define MAX_TOKEN	32
#define ATTR_TOKEN	4

#define NOT_INSERT	0
#define INSERT		1

typedef enum
{
	TOKEN_NAME = 0,
	TOKEN_GENDER,
	TOKEN_BIRTH,
	TOKEN_ADDRESS,

	TOKEN_MAX
} Token_e;

int METHOD_Post( DB_t tDBWorker, const char *pszIp,
		struct REQUEST_s *ptRequest,
		struct RESPONSE_s *ptResponse );

int METHOD_Get( DB_t tDBWorker, const char *pszIp,
		struct REQUEST_s *ptRequest,
		struct RESPONSE_s *ptResponse );

int METHOD_Delete( DB_t tDBWorker, const char *pszIp,
		struct REQUEST_s *ptRequest,
		struct RESPONSE_s *ptResponse );

#endif /* _RAS_METHOD_H_ */
