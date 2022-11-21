/* RAS_Method.h */
#ifndef _RAS_METHOD_H
#define _RAS_METHOD_H

#define TOK_NAME_ATTR		0
#define TOK_NAME_STR		1
#define TOK_GENDER_ATTR		2
#define TOK_GENDER_STR		3
#define TOK_BIRTH_ATTR		4
#define TOK_BIRTH_STR		5
#define TOK_ADDRESS_ATTR	6
#define TOK_ADDRESS_STR		7

int METHOD_Post( DB_t tDBWorker,
		struct HTTP_REQUEST_s *ptRequest,
		const char *pszIp );

int METHOD_Get( DB_t tDBWorker,
		struct HTTP_REQUEST_s *ptRequest,
		struct HTTP_RESPONSE_s *ptResponse,
		const char *pszIp );

#endif /* _RAS_METHOD_H_ */
