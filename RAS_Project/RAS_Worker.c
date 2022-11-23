/* RAS_Worker.c */
#include "RAS_Inc.h"

extern WORKER_t g_tWorker[MAX_WORKER_CNT];

int WORKER_Init()
{
	int nRC = 0;
	int nIndex = 0;
	int nCancel = 0;

	for ( nIndex = 0; nIndex < MAX_WORKER_CNT; nIndex++ )
	{
		nRC = pthread_create( &g_tWorker[nIndex].nThreadId, NULL, WORKER_Run, &(g_tWorker[nIndex]) );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_create fail <%d>", nRC );
			
			for ( nCancel = 0; nCancel < nIndex; nCancel++ )
			{
				THREAD_CANCEL( g_tWorker[nCancel].nThreadId );
			}
		
			return RAS_rErrWorkerInit;
		}
		LOG_DBG_F( "pthread_create (Tid %lu) success", g_tWorker[nIndex].nThreadId );
	}

	return RAS_rOK;
}

void *WORKER_Run( void *pvArg )
{
	CHECK_PARAM( pvArg, pthread_exit( NULL ) );
	
	WORKER_t *ptWorker = (WORKER_t *)pvArg;

	int nRC = 0;
	int nIndex = 0;
	int nCntFd = 0;
	int nClientFd = 0;
	int nMethod = 0;

	struct epoll_event atEvents[MAX_CONNECT];
	memset( atEvents, 0x00, sizeof(atEvents) );	
	
	DB_t tDBWorker;
	memset( &tDBWorker, 0x00, sizeof(tDBWorker) );

	REQUEST_t tRequest;
	memset( &tRequest, 0x00, sizeof(tRequest) );

	RESPONSE_t tResponse;
	memset( &tResponse, 0x00, sizeof(tResponse) );
	
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
		printf( "worker thread epoll_wait..\n" );

		nCntFd = epoll_wait( ptWorker->nEpollFd, atEvents, MAX_CONNECT, TIME_OUT );
		if ( -1 == nCntFd )
		{
			LOG_ERR_F( "epoll_wait fail <%d:%s>", errno, strerror(errno) );
			if ( EINTR == errno )
			{
				LOG_DBG_F( "Worker EINTR" );
				continue;
			}
			else
			{
				FD_CLOSE( ptWorker->nEpollFd );
				goto _exit_worker;
			}
		}

		LOG_DBG_F( "nCntFd <%d>", nCntFd );

		for ( nIndex = 0; nIndex < nCntFd; nIndex++ )
		{
			if ( (EPOLLERR & atEvents[nIndex].events) ||
				 (EPOLLHUP & atEvents[nIndex].events) ||
				 (EPOLLRDHUP & atEvents[nIndex].events) )
			{
				nClientFd = atEvents[nIndex].data.fd;
				LOG_ERR_F( "fd(%d) events(%u)", nClientFd, atEvents[nIndex].events );
				FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );	
				continue;
			}
			else if ( EPOLLIN & atEvents[nIndex].events )
			{
				nClientFd = atEvents[nIndex].data.fd;
				LOG_DBG_F( "fd(%d) events(%u)", nClientFd, atEvents[nIndex].events );

				nMethod = 0;
				memset( &tRequest, 0x00, sizeof(tRequest) );
				memset( &tResponse, 0x00, sizeof(tResponse) );

				/*
				 *	Read Header
				 */
				nRC = HTTP_ReadHeader( nClientFd, &tRequest );
				if ( RAS_rOK != nRC )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}
		
				nRC = HTTP_GetMethodAndPath( &tRequest );
				if ( RAS_rOK != nRC )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}
				
				nRC = HTTP_GetContentLength( &tRequest );
				if ( RAS_rOK != nRC )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}
			
				/*
				 *	Read Body
				 */	
				nRC = HTTP_ReadBody( nClientFd, &tRequest );
				if ( RAS_rOK != nRC )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}
		
				HTTP_STR_TO_NUM_METHOD( tRequest.szMethod, nMethod );

				/*
				 *	Stat
				 */
				STAT_Count( HTTP_TYPE_REQUEST, nMethod, 0, ptWorker->szClientIp );

				/*
				 *	Trace
				 */
				nRC = REGI_CheckKeyExist( ptWorker->szClientIp );
				if ( RAS_rOK == nRC )
				{
					nRC = TRACE_MakeTrace( HTTP_TYPE_REQUEST, ptWorker->szClientIp,	&tRequest, &tResponse );
					if ( RAS_rOK != nRC )
					{
						goto _send_response;
					}
				}

				/*
				 *	Method
				 */
				switch ( nMethod )
				{
					case HTTP_METHOD_POST_NUM:
						nRC = METHOD_Post( tDBWorker, ptWorker->szClientIp, &tRequest, &tResponse );
						break;
					case HTTP_METHOD_GET_NUM:
						nRC = METHOD_Get( tDBWorker, ptWorker->szClientIp, &tRequest, &tResponse );
						break;
					case HTTP_METHOD_DEL_NUM:
						nRC = METHOD_Delete( tDBWorker, ptWorker->szClientIp, &tRequest, &tResponse );
						break;
					default:
						nRC = RAS_rErrHttpMethodNotAllowed;
						break;
				}

_send_response:
				tResponse.nStatusCode = HTTP_GetStatusCode( nRC );
			
				HTTP_SET_RESPONSE( tResponse );
				
				LOG_DBG_F( "\n%s\n%s\n%s", LINE, tResponse.szMsg, LINE );
			
				/*
				 *	Send Response
				 */	
				nRC = HTTP_SendResponse( nClientFd, tResponse.szMsg, sizeof(tResponse.szMsg) );
				if ( RAS_rOK != nRC )
				{
					LOG_ERR_F( "HTTP_SendResponse (fd %d) fail <%d>", nClientFd, nRC );
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}
				
				//NOTE <Postman> Parse Error: Server returned a malformed response

				STAT_Count( HTTP_TYPE_RESPONSE, nMethod, tResponse.nStatusCode, ptWorker->szClientIp );

				nRC = REGI_CheckKeyExist( ptWorker->szClientIp );
				if ( RAS_rOK == nRC )
				{
					nRC = TRACE_MakeTrace( HTTP_TYPE_RESPONSE, ptWorker->szClientIp, &tRequest, &tResponse );
					if ( RAS_rOK != nRC )
					{
						FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
						continue;
					}
				}

				//TODO Alarm
			
			}//else if(events & EPOLLIN)

		}//for(nFdCnt)

	}//while(1)

	printf( "end worker thread\n" );
	FD_CLOSE( ptWorker->nEpollFd );

_exit_worker:
	DB_Close( &tDBWorker );
	pthread_exit( NULL );
}
