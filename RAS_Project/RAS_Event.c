/* RAS_Event.c */
#include "RAS_Inc.h"

extern int g_nThreadIndex;
extern THREAD_t g_tThread[THREAD_CNT];

int EVENT_Init( int *pnEpollFd, int nListenFd )
{
	int nRC = 0;
	
	struct epoll_event tEvent;
	memset( &tEvent, 0x00, sizeof(tEvent) );

	/*
	 *	Create Epoll
	 */
	*pnEpollFd = epoll_create( 1 );
	if ( -1 == *pnEpollFd )
	{
		LOG_ERR_F( "epoll_create fail" );
		return RAS_rErrEpollInit;
	}

	LOG_SVC_F( "epoll_create" );

	/*
	 *	Add ListenFd to Epoll List
	 */
	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;

	nRC = epoll_ctl( *pnEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "epoll_ctl_add fail" );
		return RAS_rErrEpollInit;
	}

	LOG_SVC_F( "epoll_ctl_add" );

	return RAS_rOK;
}

int EVENT_WaitAndAccept( int nEpollFd, int nListenFd )
{
	int nRC = 0;
	int nIndex = 0;
	int nCntFd = 0;
	int nAcceptFd = 0;

	struct epoll_event tEvent;
	struct epoll_event atEvents[MAX_CLIENT_CONNECT];
	struct sockaddr_in tClientAddr;

	memset( &tEvent, 0x00, sizeof(tEvent) );
	memset( atEvents, 0x00, sizeof(atEvents) );
	memset( &tClientAddr, 0x00, sizeof(tClientAddr) );
	
	socklen_t tAddrLen = 0;

	char* pszClientIp = NULL;

	nCntFd = epoll_wait( nEpollFd, atEvents, MAX_CLIENT_CONNECT, TIME_OUT );
	if ( -1 == nCntFd )
	{
		LOG_ERR_F( "epoll_wait fail <%d>", errno );
		if ( EINTR == errno )
		{
			return RAS_rErrEventRecreateNo;
		}
		else
		{
			return RAS_rErrEventRecreateYes;
		}
	}

	for ( nIndex = 0; nIndex < nCntReadyFd; nIndex++ )
	{
		if ( (atEvents[nIndex].events & EPOLLERR)
				|| (atEvents[nIndex].events & EPOLLHUP)
				|| (atEvents[nIndex].events & EPOLLRDHUP) )
		{
			nRC = epoll_ctl( nEpollFd, EPOLL_CTL_DEL, nListenFd, &tEvent );
			if ( -1 == nRC )
			{
				LOG_ERR_F( "epoll_ctl_del fail" )
			}

			LOG_SVC_F( "Fd %d fail <%u>", atEvents[nIndex].data.fd, atEvents[nIndex].events );

			return RAS_rErrEventRecreateYes;
		}
		else if ( atEvents[nIndex].events & EPOLLIN )
		{
			nAcceptFd = 0;
			memset( &tEvent, 0x00, sizeof(tEvent) );
			memset( &tClientAddr, 0x00, sizeof(tClientAddr) );
			tAddrLen = 0;
			pszClientIp = NULL;

			nAcceptFd = accept( nListenFd, &tClientAddr, &tAddrLen );
			if ( -1 == nAcceptFd )
			{
				LOG_ERR_F( "accept fail" );
				return RAS_rErrEventRecreateYes;
			}

			pszClientIp = inet_ntoa( tClientAddr.sin_addr );
			LOG_SVC_F( "Accept ClientIp<%s>", pszClientIp );

			//TODO RAS_DB.h RAS_DB.c
			nRC = DB_CheckIp( pszClientIp );
			if ( RAS_rOK != nRC )
			{
				LOG_ERR_F( "DB_CheckIp fail <%d>", nRC );
				return RAS_rErrEventRecreateYes;
			}

			tEvent.events = EPOLLIN || EPOLLRDHUP || EPOLLERR || EPOLLHUP;
			tEvent.data.fd = nAcceptFd;

			nRC = epoll_ctl( g_tThread[g_nThreadIndex].nEpollFd, EPOLL_CTL_ADD, nAcceptFd, &tEvent );
			if ( -1 == nRC )
			{
				LOG_ERR_F( "epoll_ctl_add fail" );
				return RAS_rErrEventRecreateYes;				
			}
			
			g_nThreadIndex++;
			
			if ( THREAD_CNT == g_nThreadIndex )
			{
				//TODO
			}
		}
	}

	return RAS_rOK;
} 
