/* RAS_Worker.c */
#include "RAS_Inc.h"

extern WORKER_t g_tWorker[MAX_WORKER_CNT];

int WORKER_Init()
{
	int nRC = 0;
	int nIndex = 0;
	int nIndexForExit = 0;

	for ( nIndex = 0; nIndex < MAX_WORKER_CNT; nIndex++ )
	{
		nRC = pthread_create( &g_tWorker[nIndex].nThreadId, NULL, WORKER_Run, &(g_tWorker[nIndex]) );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_create fail <%d>", nRC );
			
			for ( nIndexForExit = 0; nIndexForExit < nIndex; nIndexForExit++ )
			{
				THREAD_CANCEL( g_tWorker[nIndexForExit].nThreadId );
			}
		
			return RAS_rErrWorkerInit;
		}

		LOG_DBG_F( "pthread_create (Tid %lu) success", g_tWorker[nIndex].nThreadId );
	}

	return RAS_rOK;
}

void *WORKER_Run( void *pvArg )
{
	//CHECK_PARAM_RC( pvArg );
	
	WORKER_t *ptWorker = (WORKER_t *)pvArg;

	int nRC = 0;
	int nIndex = 0;
	int nCntFd = 0;

	DB_t tDBWorker;
	memset( &tDBWorker, 0x00, sizeof(tDBWorker) );
	USER_t tUser;
	memset( &tUser, 0x00, sizeof(tUser) );

	struct epoll_event atEvents[MAX_CONNECT];
	memset( atEvents, 0x00, sizeof(atEvents) );	
	
	char szRequestHeader[1024];
	char szRequestMethod[16];
	char szRequestPath[128];
	char szRequestBody[1024];
	int nContentLength = 0;

	int nStatusCode = 0;
	char szStatusMsg[32];
	char szResponseBody[1024];
	char szResponseMsg[1024];

	nRC = DB_Init( &(tDBWorker.ptDBConn) );
	if ( RAS_rOK != nRC )
	{
		goto _exit_worker;
	}	
	
	nRC = DB_InitPreparedStatement( &tDBWorker );
	if ( RAS_rOK != nRC )
	{
		goto _exit_worker;
	}

	nRC = EVENT_Init( &(ptWorker->nEpollFd) );
	if ( RAS_rOK != nRC )
	{
		goto _exit_worker;		
	}

	while ( 1 )
	{
		nCntFd = epoll_wait( ptWorker->nEpollFd, atEvents, MAX_CONNECT, TIME_OUT );
		if ( -1 == nCntFd )
		{
			LOG_ERR_F( "epoll_wait (EpollFd %d) fail <%d>", ptWorker->nEpollFd, errno );
			if ( EINTR == errno )
			{
				LOG_DBG_F( "EINTR 실행재개" );
				continue;
			}
			else
			{
				FD_CLOSE( ptWorker->nEpollFd );
				//NOTE Thread Epoll 재생성?
				goto _exit_worker;
			}
		}

		for ( nIndex = 0; nIndex < nCntFd; nIndex++ )
		{
			if ( (EPOLLERR & atEvents[nIndex].events) ||
				 (EPOLLHUP & atEvents[nIndex].events) ||
				 (EPOLLRDHUP & atEvents[nIndex].events) )
			{
				LOG_ERR_F( "data.fd = %d | events = %u", atEvents[nIndex].data.fd, atEvents[nIndex].events );
				FD_DELETE( ptWorker->nEpollFd, atEvents[nIndex].data.fd );	
				FD_CLOSE( atEvents[nIndex].data.fd );	
				continue;
			}
			else if ( EPOLLIN & atEvents[nIndex].events )
			{
				memset( szRequestHeader, 0x00, sizeof(szRequestHeader) );
				memset( szRequestMethod, 0x00, sizeof(szRequestMethod) );
				memset( szRequestPath, 0x00, sizeof(szRequestPath) );
				memset( szRequestBody, 0x00, sizeof(szRequestBody) );
				memset( szStatusMsg, 0x00, sizeof(szStatusMsg) );
				memset( szResponseMsg, 0x00, sizeof(szResponseMsg) );	
				memset( szResponseBody, 0x00, sizeof(szResponseBody) );	
		
				nRC = HTTP_ReadHeader( atEvents[nIndex].data.fd, szRequestHeader, sizeof(szRequestHeader) );
				if ( RAS_rOK != nRC )
				{
					LOG_ERR_F( "HTTP_ReadHeader fail <%d>", nRC );
					goto _close_continue;
				}
				
				nRC = HTTP_GetMethodAndPath( szRequestHeader, szRequestMethod, sizeof(szRequestMethod), szRequestPath, sizeof(szRequestPath) );
				if ( RAS_rOK != nRC )
				{
					LOG_ERR_F( "HTTP_GetMethodAndPath fail <%d>", nRC );
					goto _close_continue;
				}

				nRC = HTTP_GetContentLength( szRequestHeader, &nContentLength );
				if ( RAS_rOK != nRC )
				{
					LOG_ERR_F( "HTTP_GetContentLength fail <%d>", nRC );
					goto _close_continue;
				}
				
				nRC = HTTP_ReadBody( atEvents[nIndex].data.fd, szRequestHeader, nContentLength, szRequestBody, sizeof(szRequestBody) );
				if ( RAS_rOK != nRC )
				{
					LOG_ERR_F( "HTTP_ReadBody fail <%d>", nRC );
					goto _close_continue;
				}

				/*
				 * TODO Stat
				 * STGEN_DTLITEM_1COUNT( HTTP_TOTAL_REQUEST, ptWorker->szClientIp );
				 * STGEN_DTLITEM_1COUNT( HTTP_REQUEST_POST, IP );
				 * STGEN_DTLITEM_1COUNT( HTTP_REQUEST_GET, IP );
				 * STGEN_DTLITEM_1COUNT( HTTP_REQUEST_DELETE, IP );
				 * STGEN_DTLITEM_1COUNT( HTTP_REQUEST_UNKNOWN, IP );
				 * TODO Trace (Request)
				 */
			
				nRC = HTTP_ProcessRequestMsg( szRequestMethod, szRequestPath, szRequestBody, tDBWorker, szResponseBody, sizeof(szResponseBody) );
				//TODO 
				if ( RAS_rErrDBSetValue == nRC || RAS_rErrDBExecute == nRC )
				{
					nStatusCode = STATUS_CODE_500;
				}
				else if ( RAS_rOK == nRC )
				{
					nStatusCode = STATUS_CODE_200;
				}
				else if ( RAS_rErrHttpBadRequest == nRC )
				{
					nStatusCode = STATUS_CODE_400;
				}
				else if ( RAS_rErrHttpMethodNotAllowed == nRC )
				{
					nStatusCode = STATUS_CODE_405;
				}
				
				strlcpy( szStatusMsg, HTTP_GetStatusMsg(nStatusCode), sizeof(szStatusMsg) );

				HTTP_SET_RESPONSE_MSG( szResponseMsg, sizeof(szResponseMsg),
						nStatusCode, szStatusMsg, (int)strlen(szResponseBody), szResponseBody );

				nRC = HTTP_SendResponseMsg( atEvents[nIndex].data.fd, szResponseMsg, strlen(szResponseMsg) );
				if ( RAS_rOK != nRC )
				{
					LOG_ERR_F( "HTTP_SendResponseMsr (Fd %d) fail <%d>", atEvents[nIndex].data.fd, nRC );
					FD_DELETE( ptWorker->nEpollFd, atEvents[nIndex].data.fd );
					FD_CLOSE( atEvents[nIndex].data.fd );
					continue;
				}

				/*
				 * TODO Stat
				 * STGEN_DTLITEM_1COUNT( HTTP_TOTAL_RESPONSE, ptWorker->szClientIp );
				 * STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_POSTxxx, IP );
				 * POST 201 400 500
				 * GET 200 400 404 500
				 * DELETE 200 400 404 500
				 * TODO Trace (Response)
				 * TODO Alarm
				 */
			
			}//else if EPOLLIN

_close_continue:
			//NOTE POSTMAN
			FD_DELETE( ptWorker->nEpollFd, atEvents[nIndex].data.fd );
			FD_CLOSE( atEvents[nIndex].data.fd );

		}//for

	}//while
	
	FD_CLOSE( ptWorker->nEpollFd );

_exit_worker:
	DB_Close( &tDBWorker );
	pthread_exit( NULL );
}
