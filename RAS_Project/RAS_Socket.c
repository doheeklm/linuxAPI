/* RAS_Socket.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

int SOCKET_Init( int *pnListenFd )
{
	CHECK_PARAM( pnListenFd, return RAS_rErrSocketInit );

	int nRC = 0;
	const int nFlag = 1;
	struct sockaddr_in tSockAddr;
	memset( &tSockAddr, 0x00, sizeof(tSockAddr) );

	*pnListenFd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( -1 == *pnListenFd )
	{
		LOG_ERR_F( "socket fail <%d>", *pnListenFd );
		return RAS_rErrSocketInit;
	}

	tSockAddr.sin_family = AF_INET;
	tSockAddr.sin_port = htons( g_tEnv.nPort );
	
	nRC = inet_aton( g_tEnv.szIp, &(tSockAddr.sin_addr) );
	if ( 0 == nRC )
	{
		LOG_ERR_F( "inet_aton fail <%d>", nRC );
		goto end_of_function;
	}

	nRC = setsockopt( *pnListenFd, SOL_SOCKET, SO_REUSEADDR, &nFlag, sizeof(int) );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "setsockopt (ListenFd %d) fail <%d>", *pnListenFd, nRC );
		goto end_of_function;
	}

	nRC = bind( *pnListenFd, &tSockAddr, sizeof(tSockAddr) );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "bind (ListenFd %d) fail <%d>", *pnListenFd, nRC );
		goto end_of_function;
	}

	nRC = listen( *pnListenFd, BACK_LOG );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "listen (ListenFd %d) fail <%d>", *pnListenFd, nRC );
		goto end_of_function;
	}

	return RAS_rOK;

end_of_function:
	FD_CLOSE( *pnListenFd );
	return RAS_rErrSocketInit;
}
