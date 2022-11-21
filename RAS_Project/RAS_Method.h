/* RAS_Method.h */
#ifndef _RAS_METHOD_H
#define _RAS_METHOD_H

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
