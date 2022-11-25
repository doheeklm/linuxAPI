/* RAS_Worker.c */
#include "RAS_Inc.h"

WORKER_t	g_tWorker[MAX_WORKER_CNT];

void WORKER_CleanUp( void *pvArg )
{
	CHECK_PARAM( pvArg, return );
	WORKER_t *ptWorker = (WORKER_t *)pvArg;

	FD_CLOSE( ptWorker->nEpollFd );
	DB_Close( &ptWorker->tDB );
}

int WORKER_Init()
{
	int nRC = 0;
	int nIndex = 0;
	int nCancel = 0;

	for ( nIndex = 0; nIndex < MAX_WORKER_CNT; nIndex++ )
	{
		nRC = pthread_create( &g_tWorker[nIndex].nThreadId, NULL,
							  WORKER_Run, &(g_tWorker[nIndex]) );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_create fail <%d>", nRC );
			
			for ( nCancel = 0; nCancel < nIndex; nCancel++ )
			{
				THREAD_CANCEL( g_tWorker[nCancel].nThreadId );
			}
		
			return RAS_rErrWorkerInit;
		}
	}

	return RAS_rOK;
}

void *WORKER_Run( void *pvArg )
{
	CHECK_PARAM( pvArg, pthread_exit( NULL ) );
	
	WORKER_t *ptWorker = (WORKER_t *)pvArg;
	
	PRT_TID( syscall(SYS_gettid) );
	
	int nRC = 0;
	int nIndex = 0;
	int nCntFd = 0;
	int nClientFd = 0;
	int nMethod = 0;
	struct sockaddr_in tSockAddr;
	memset( &tSockAddr, 0x00, sizeof(tSockAddr) );
	socklen_t tAddrLen = sizeof(tSockAddr);
	struct epoll_event atEvents[MAX_CONNECT];
	memset( atEvents, 0x00, sizeof(atEvents) );	
	char *pszIp = NULL;

	REQUEST_t tRequest;
	memset( &tRequest, 0x00, sizeof(tRequest) );

	RESPONSE_t tResponse;
	memset( &tResponse, 0x00, sizeof(tResponse) );
	
	nRC = DB_Init( &(ptWorker->tDB.ptDBConn) );
	if ( RAS_rOK != nRC )
	{
		pthread_exit( NULL );
	}	

	pthread_cleanup_push( WORKER_CleanUp, ptWorker );

	nRC = DB_InitPreparedStatement( &(ptWorker->tDB) );
	if ( RAS_rOK != nRC )
	{
		DB_Close( &ptWorker->tDB );
		pthread_exit( NULL );
	}

	nRC = EVENT_Init( &(ptWorker->nEpollFd) );
	if ( RAS_rOK != nRC )
	{
		DB_Close( &ptWorker->tDB );
		pthread_exit( NULL );
	}
	PRT_CREATE_EPOLL( syscall(SYS_gettid), ptWorker->nEpollFd );

	while ( 1 )
	{
		nCntFd = epoll_wait( ptWorker->nEpollFd, atEvents, MAX_CONNECT, TIME_OUT );
		if ( -1 == nCntFd )
		{
			LOG_ERR_F( "epoll_wait fail <%d:%s>", errno, strerror(errno) );
			if ( EINTR == errno )
			{
				continue;
			}
			else
			{
				//NOTE worker epoll에 등록된 client fd를 관리하지 않아서 epoll 재생성 하지 않고 close함
				FD_CLOSE( ptWorker->nEpollFd );
				goto _exit_worker;
			}
		}

		for ( nIndex = 0; nIndex < nCntFd; nIndex++ )
		{
			nClientFd = atEvents[nIndex].data.fd;
			printf( "%s:: (epoll %d) (fd %d) (events %u)\n",
					__func__, ptWorker->nEpollFd, nClientFd, atEvents[nIndex].events );

			if ( (EPOLLERR & atEvents[nIndex].events) ||
				 (EPOLLHUP & atEvents[nIndex].events) ||
				 (EPOLLRDHUP & atEvents[nIndex].events) )
			{
				LOG_ERR_F( "(epoll %d) (fd %d) (events %u)",
						ptWorker->nEpollFd, nClientFd, atEvents[nIndex].events );
				FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );	
				continue;
			}
			else if ( EPOLLIN & atEvents[nIndex].events )
			{
				nMethod = 0;
				memset( &tRequest, 0x00, sizeof(tRequest) );
				memset( &tResponse, 0x00, sizeof(tResponse) );

				LOG_DBG_F( "(epoll %d) (fd %d) (events %u)",
						ptWorker->nEpollFd, nClientFd, atEvents[nIndex].events );

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

				nRC = HTTP_ReadBody( nClientFd, &tRequest );
				if ( RAS_rOK != nRC )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}

				HTTP_STR_TO_NUM_METHOD( tRequest.szMethod, nMethod );

				nRC = getpeername( nClientFd, &tSockAddr, &tAddrLen );
				if ( -1 == nRC )
				{
					LOG_ERR_F( "getpeername fail <%d:%s>", errno, strerror(errno) );
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}

				pszIp = inet_ntoa( tSockAddr.sin_addr );
				LOG_DBG_F( "Client Ip (%s)", pszIp );

				STAT_Count( HTTP_TYPE_REQUEST, nMethod, 0, pszIp );

				nRC = REGI_CheckKeyExist( pszIp );
				if ( RAS_rOK == nRC )
				{
					nRC = TRACE_MakeTrace( HTTP_TYPE_REQUEST, pszIp, &tRequest, &tResponse );
					if ( RAS_rOK != nRC )
					{
						goto _send_response;
					}
				}

				switch ( nMethod )
				{
					case HTTP_METHOD_POST_NUM:
					{
						nRC = METHOD_Post( ptWorker->tDB, &tRequest, &tResponse );
					}
						break;
					case HTTP_METHOD_GET_NUM:
					{
						nRC = METHOD_Get( ptWorker->tDB, &tRequest, &tResponse );
					}
						break;
					case HTTP_METHOD_DEL_NUM:
					{
						nRC = METHOD_Delete( ptWorker->tDB,&tRequest, &tResponse );
					}
						break;
					default:
					{
						nRC = RAS_rErrHttpMethodNotAllowed;
					}
						break;
				}

_send_response:
				if ( SIZE_ZERO > sizeof(tResponse.szBody) || sizeof(tResponse.szMsg) < sizeof(tResponse.szBody) )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					goto _exit_worker;
				}

				tResponse.nStatusCode = HTTP_GetStatusCode( nRC );

				HTTP_SET_RESPONSE( nMethod, tResponse );

				nRC = HTTP_SendResponse( nClientFd, tResponse.szMsg );
				if ( RAS_rOK != nRC )
				{
					FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
					continue;
				}

				STAT_Count( HTTP_TYPE_RESPONSE, nMethod, tResponse.nStatusCode, pszIp );

				nRC = REGI_CheckKeyExist( pszIp );
				if ( RAS_rOK == nRC )
				{
					nRC = TRACE_MakeTrace( HTTP_TYPE_RESPONSE, pszIp, &tRequest, &tResponse );
					if ( RAS_rOK != nRC )
					{
						FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );
						continue;
					}
				}
		
				FD_DELETE_AND_CLOSE( nClientFd, ptWorker->nEpollFd );

			}//else if(events & EPOLLIN)
			
		}//for(nFdCnt)

	}//while(1)

_exit_worker:
	PRT_EXIT;
	pthread_cleanup_pop( 0 );
	FD_CLOSE( ptWorker->nEpollFd );
	DB_Close( &ptWorker->tDB );
	pthread_exit( NULL );
}
