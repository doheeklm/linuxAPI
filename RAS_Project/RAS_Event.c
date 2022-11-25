/* RAS_Event.c */
#include "RAS_Inc.h"

extern WORKER_t	g_tWorker[MAX_WORKER_CNT];
int g_nWorkerIndex = 0; //Main Thread에서만 사용

int EVENT_Init( int *pnEpollFd )
{
	*pnEpollFd = epoll_create( 1 );
	if ( -1 == *pnEpollFd )
	{
		LOG_ERR_F( "epoll_create fail <%d:%s>", errno, strerror(errno) );
		return RAS_rErrEventInit;
	}

	return RAS_rOK;
}

int EVENT_Add( int nListenFd, int nEpollFd )
{
	int nRC = 0;
	struct epoll_event tEvent;
	memset( &tEvent, 0x00, sizeof(tEvent) );

	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;

	nRC = epoll_ctl( nEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "epoll_ctl fail <%d:%s>", errno, strerror(errno) );
		return RAS_rErrEpollFdListenFd;
	}

	return RAS_rOK;
}

int EVENT_Wait( int nListenFd, int nEpollFd )
{
	int nRC = 0;
	int nIndex = 0;
	int nCntFd = 0;
	int nAcceptFd = 0;
	struct epoll_event tEvent;
	memset( &tEvent, 0x00, sizeof(tEvent) );
	struct epoll_event atEvents[MAX_CONNECT];
	memset( atEvents, 0x00, sizeof(atEvents) );
	struct sockaddr_in tAddr;
	memset( &tAddr, 0x00, sizeof(tAddr) );
	socklen_t tAddrLen = sizeof(tAddr);
	char* pszClientIp = NULL;
	
	nCntFd = epoll_wait( nEpollFd, atEvents, MAX_CONNECT, TIME_OUT );
	if ( -1 == nCntFd )
	{
		LOG_ERR_F( "epoll_wait fail <%d:%s>", errno, strerror(errno) );
		if ( EINTR == errno )
		{
			return RAS_rOK;
		}
		else
		{
			return RAS_rErrEpollFd;
		}
	}

	for ( nIndex = 0; nIndex < nCntFd; nIndex++ )
	{
		if ( (EPOLLERR & atEvents[nIndex].events) ||
			 (EPOLLHUP & atEvents[nIndex].events) ||
			 (EPOLLRDHUP & atEvents[nIndex].events) )
		{
			return RAS_rErrListenFd;
		}
		else if ( EPOLLIN & atEvents[nIndex].events )
		{
			//TODO Fd는 보통 0이 아닌 -1로 초기화함
			nAcceptFd = 0;
			memset( &tEvent, 0x00, sizeof(tEvent) );
			memset( &tAddr, 0x00, sizeof(tAddr) );
			pszClientIp = NULL;

			nAcceptFd = accept( nListenFd, &tAddr, &tAddrLen );
			if ( -1 == nAcceptFd )
			{
				LOG_ERR_F( "accept fail <%d:%s>", errno, strerror(errno) );
				return RAS_rErrListenFd;
			}

			pszClientIp = inet_ntoa( tAddr.sin_addr );

			printf( "%s:: (ProcessId %d) (accept %d)\n", __func__, getpid(), nAcceptFd);
			LOG_DBG_F( "accept ip(%s)", pszClientIp );

			nRC = UTIL_CheckClientIp( pszClientIp );
			if ( RAS_rOK != nRC )
			{
				printf( "%s:: close (accept %d)\n", __func__, nAcceptFd );
				LOG_DBG_F( "ip(%s) not in DB <%d>", pszClientIp, nRC );
				FD_CLOSE( nAcceptFd );
				continue;
			}

			tEvent.events = EPOLLIN || EPOLLRDHUP || EPOLLERR || EPOLLHUP;
			tEvent.data.fd = nAcceptFd;

			nRC = epoll_ctl( g_tWorker[g_nWorkerIndex].nEpollFd, EPOLL_CTL_ADD, nAcceptFd, &tEvent );
			if ( -1 == nRC )
			{
				LOG_ERR_F( "epoll_ctl fail <%d:%s>", errno, strerror(errno) );
				printf( "%s:: close (accept %d)\n", __func__, nAcceptFd );
				FD_CLOSE( nAcceptFd );
				continue;
			}

			printf( "\n%s:: Add (accept %d) to Worker[%d] (epoll %d)\n",
				   __func__, nAcceptFd, g_nWorkerIndex, g_tWorker[g_nWorkerIndex].nEpollFd );
		
			g_nWorkerIndex++; //기본값 0
			printf( "Worker Index %d\n", g_nWorkerIndex );
//modu
			INDEX_RESET_WHEN_MAX( MAX_WORKER_CNT, g_nWorkerIndex );
		}
	}

	return RAS_rOK;
} 
