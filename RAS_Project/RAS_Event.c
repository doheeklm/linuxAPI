/* RAS_Event.c */
#include "RAS_Inc.h"

extern int g_nWorkerIndex;
extern WORKER_t g_tWorker[MAX_WORKER_CNT];

int EVENT_Init( int *pnEpollFd )
{
	*pnEpollFd = epoll_create( 1 );
	if ( -1 == *pnEpollFd )
	{
		LOG_ERR_F( "epoll_create fail <%d>", *pnEpollFd );
		return RAS_rErrEventInit;
	}

	return RAS_rOK;
}

int EVENT_AddToList( int nListenFd, int nEpollFd )
{
	int nRC = 0;
	struct epoll_event tEvent;
	memset( &tEvent, 0x00, sizeof(tEvent) );

	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;

	nRC = epoll_ctl( nEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent );
	if ( -1 == nRC )
	{
		if ( EEXIST == errno )
		{
			LOG_ERR_F( "epoll_ctl ok<%d:The supplied file descriptor fd(%d) is already registered with this epoll instance.>", nRC, nListenFd );
			return RAS_rOK;
		}
		else if ( EPERM == errno )
		{
			LOG_ERR_F( "epoll_ctl fail <%d:The target file fd(%d) does not support epoll. "
					"This error can occur if fd refers to, for example, a regular file or a directory>",
					nRC, nListenFd );
			return RAS_rErrListenFd;
		}
		else
		{
			LOG_ERR_F( "epoll_ctl (EPOLL_CTL_ADD fd %d to epoll %d) fail <%d>", nListenFd, nEpollFd, nRC );
			return RAS_rErrEpollFdListenFd;
		}
	}

	return RAS_rOK;
}

int EVENT_Wait( int nListenFd, int nEpollFd, DB_t tDBMain )
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
		LOG_ERR_F( "epoll_wait (EpollFd %d) fail <%d>", nEpollFd, errno );
		if ( EINTR == errno )
		{
			LOG_DBG_F( "EINTR 실행재개" );
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
			LOG_ERR_F( "data.fd = %d | events = %u", atEvents[nIndex].data.fd, atEvents[nIndex].events );
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
				LOG_ERR_F( "accept (ListenFd %d) fail <%d>", nListenFd, nRC );
				return RAS_rErrListenFd;
			}

			pszClientIp = inet_ntoa( tClientAddr.sin_addr );
			LOG_DBG_F( "Accept ClientIp <%s>", pszClientIp );

			nRC = DB_CheckClientIp( tDBMain, pszClientIp );
			if ( RAS_rOK != nRC )
			{
				LOG_ERR_F( "DB_CheckIp (%s) fail <%d>", pszClientIp, nRC );
				nRC = close( nAcceptFd );
				if ( -1 == nRC )
				{
					LOG_ERR_F( "close (AcceptFd %d) fail <%d>", nAcceptFd, nRC );
				}
				continue;
			}
	
			strlcpy( g_tWorker[g_nWorkerIndex].szClientIp, pszClientIp,
					sizeof(g_tWorker[g_nWorkerIndex].szClientIp) );
			g_tWorker[g_nWorkerIndex].szClientIp[ strlen(g_tWorker[g_nWorkerIndex].szClientIp) ] = '\0';

			tEvent.events = EPOLLIN || EPOLLRDHUP || EPOLLERR || EPOLLHUP;
			tEvent.data.fd = nAcceptFd;

			nRC = epoll_ctl( g_tWorker[g_nWorkerIndex].nEpollFd, EPOLL_CTL_ADD, nAcceptFd, &tEvent );
			if ( -1 == nRC )
			{
				LOG_ERR_F( "epoll_ctl (ADD AcceptFd(%d) to Thread[%d]'s EpollFd %d) fail <%d>",
						nAcceptFd, g_nWorkerIndex, g_tWorker[g_nWorkerIndex].nEpollFd, nRC );
				nRC = close( nAcceptFd );
				if ( -1 == nRC )
				{
					LOG_ERR_F( "close (AcceptFd %d) fail <%d>", nAcceptFd, nRC );
				}
				continue;
			}
		
			g_nWorkerIndex++;
			if ( MAX_WORKER_CNT == g_nWorkerIndex )
			{
				g_nWorkerIndex = 0;
			}
		}
	}

	return RAS_rOK;
} 
