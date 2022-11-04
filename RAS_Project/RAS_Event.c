/* RAS_Event.c */
#include "RAS_Inc.h"

extern int g_nThreadIndex;
extern THREAD_t g_tThread[THREAD_CNT];
extern DB_t g_tDBMain;

int EVENT_Init( int *pnEpollFd, int nListenFd )
{
	int nRC = 0;
	
	struct epoll_event tEvent;
	memset( &tEvent, 0x00, sizeof(tEvent) );

	*pnEpollFd = epoll_create( 1 );
	if ( -1 == *pnEpollFd )
	{
		LOG_ERR_F( "epoll_create fail <%d>", *pnEpollFd );
		return RAS_rErrEpollInit;
	}

	LOG_SVC_F( "epoll_create success" );
	
	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;

	nRC = epoll_ctl( *pnEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "epoll_ctl fail <%d>", nRC );
		return RAS_rErrEpollInit;
	}

	LOG_SVC_F( "epoll_ctl success <ADD LISTENFD TO EPOLL>" );

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
			goto _remove_and_recreate;
		}
	}

	for ( nIndex = 0; nIndex < nCntFd; nIndex++ )
	{
		if ( (atEvents[nIndex].events & EPOLLERR)
				|| (atEvents[nIndex].events & EPOLLHUP)
				|| (atEvents[nIndex].events & EPOLLRDHUP) )
		{
			LOG_ERR_F( "data.fd=%d events=%u", atEvents[nIndex].data.fd, atEvents[nIndex].events );
			goto _remove_and_recreate;
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
				LOG_ERR_F( "accept fail <%d>", nRC );
				goto _remove_and_recreate;
			}

			pszClientIp = inet_ntoa( tClientAddr.sin_addr );
			LOG_SVC_F( "Accept ClientIp <%s>", pszClientIp );

			nRC = DB_CheckClientIp( g_tDBMain, pszClientIp );
			if ( RAS_rOK != nRC )
			{
				LOG_ERR_F( "DB_CheckIp fail <%d> (%s)", nRC, pszClientIp );
				/*
				 * Telcobase DB에 연동 허용되지 않은 Client IP면 즉시 연결 해제
				 */
				goto _remove_and_recreate;
			}
	
			LOG_SVC_F( "Connected ClientIp (%s)", pszClientIp );

			strlcpy( g_tThread[g_nThreadIndex].szClientIp, pszClientIp, sizeof(g_tThread[g_nThreadIndex].szClientIp) );
			g_tThread[g_nThreadIndex].szClientIp[ strlen(g_tThread[g_nThreadIndex].szClientIp) ] = '\0';

			LOG_SVC_F( "Assigned to Thread ClientIp (%s)", g_tThread[g_nThreadIndex].szClientIp );

			tEvent.events = EPOLLIN || EPOLLRDHUP || EPOLLERR || EPOLLHUP;
			tEvent.data.fd = nAcceptFd;

			nRC = epoll_ctl( g_tThread[g_nThreadIndex].nEpollFd, EPOLL_CTL_ADD, nAcceptFd, &tEvent );
			if ( -1 == nRC )
			{
				LOG_ERR_F( "epoll_ctl fail <%d>", nRC );
				goto _remove_and_recreate;
			}
		
			LOG_SVC_F( "epoll_ctl success <ADD ACCEPTFD TO EPOLL>" );

			g_nThreadIndex++;
			
			if ( THREAD_CNT == g_nThreadIndex )
			{
				g_nThreadIndex = 0;
			}
		}
	}

	return RAS_rOK;

_remove_and_recreate:
	nRC = epoll_ctl( nEpollFd, EPOLL_CTL_DEL, nListenFd, &tEvent );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "epoll_ctl fail <%d>", nRC );
	}

	LOG_SVC_F( "epoll_ctl success <DELETE LISTENFD FROM EPOLL>" );
			
	return RAS_rErrEventRecreateYes;
} 
