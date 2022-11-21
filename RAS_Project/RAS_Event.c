/* RAS_Event.c */
#include "RAS_Inc.h"

extern int		g_nWorkerIndex;
extern WORKER_t	g_tWorker[MAX_WORKER_CNT];

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
	struct sockaddr_in tClientAddr;
	memset( &tClientAddr, 0x00, sizeof(tClientAddr) );
	socklen_t tAddrLen = 0;
	char* pszClientIp = NULL;

	nCntFd = epoll_wait( nEpollFd, atEvents, MAX_CONNECT, TIME_OUT );
	if ( -1 == nCntFd )
	{
		LOG_ERR_F( "epoll_wait fail <%d:%s>", errno, strerror(errno) );
		if ( EINTR == errno )
		{
			LOG_ERR_F( "Main EINTR" );
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
			LOG_ERR_F( "events fail : events = %u | data.fd = %d",
					atEvents[nIndex].events, atEvents[nIndex].data.fd );
			return RAS_rErrListenFd;
		}
		else if ( EPOLLIN & atEvents[nIndex].events )
		{
			nAcceptFd = 0;
			memset( &tEvent, 0x00, sizeof(tEvent) );
			memset( &tClientAddr, 0x00, sizeof(tClientAddr) );
			tAddrLen = 0;
			pszClientIp = NULL;

			nAcceptFd = accept( nListenFd, &tClientAddr, &tAddrLen );
			if ( -1 == nAcceptFd )
			{
				LOG_ERR_F( "accept fail <%d:%s>", errno, strerror(errno) );
				return RAS_rErrListenFd;
			}

			pszClientIp = inet_ntoa( tClientAddr.sin_addr );
			LOG_DBG_F( "accept ip(%s)", pszClientIp );

			nRC = UTIL_CheckClientIp( pszClientIp );
			if ( RAS_rOK != nRC )
			{
				LOG_DBG_F( "ip(%s) is not registered in DB <%d>", pszClientIp, nRC );
				FD_CLOSE( nAcceptFd );
				continue;
			}
	
			strlcpy( g_tWorker[g_nWorkerIndex].szClientIp, pszClientIp, sizeof(g_tWorker[g_nWorkerIndex].szClientIp) );

			tEvent.events = EPOLLIN || EPOLLRDHUP || EPOLLERR || EPOLLHUP;
			tEvent.data.fd = nAcceptFd;

			nRC = epoll_ctl( g_tWorker[g_nWorkerIndex].nEpollFd, EPOLL_CTL_ADD, nAcceptFd, &tEvent );
			if ( -1 == nRC )
			{
				LOG_ERR_F( "epoll_ctl fail <%d:%s>", errno, strerror(errno) );
				FD_CLOSE( nAcceptFd );
				continue;
			}

			LOG_DBG_F( "Thread(%d) IP(%s) AcceptFd(%d) added to Thread's Epoll(%d)",
					g_nWorkerIndex, g_tWorker[g_nWorkerIndex].szClientIp,
					nAcceptFd, g_tWorker[g_nWorkerIndex].nEpollFd );
		
			g_nWorkerIndex++;
			if ( MAX_WORKER_CNT == g_nWorkerIndex )
			{
				g_nWorkerIndex = 0;
			}
		}
	}

	return RAS_rOK;
} 
