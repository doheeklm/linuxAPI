/* RAS_Trace.h */
#ifndef _RAS_TRACE_H_
#define _RAS_TRACE_H_

#define MXCOMM			"MXCOMM"
#define REQUEST_DESC	"SERVER RECV HTTP REQUEST MSG FROM CLIENT"
#define RESPONSE_DESC	"SERVER SEND HTTP RESPONSE MSG TO CLIENT"

int TRACE_MakeTrace( int nHttpType, const char *pszIp,
		struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse );

#endif /* _RAS_TRACE_H_ */
