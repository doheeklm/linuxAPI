/* RAS_Socket.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

int SOCKET_Init( int *pnListenFd )
{
	CHECK_PARAM_RC( pnListenFd );

	int nRC = 0;

	struct sockaddr_in tSockAddr;
	memset( &tSockAddr, 0x00, sizeof(tSockAddr) );

	/*
	 *	Create Socket
	 */
	*pnListenFd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( -1 == *pnListenFd )
	{
		LOG_ERR_F( "socket fail" );
		return RAS_rErrSocketInit; 
	}

	tSocketAddr.sin_family = AF_INET;
	tSockAddr.sin_port = htons( g_tEnv.nPort );
	if ( -1 == tSocketAddr.sin_port )
	{
		LOG_ERR_F( "sin_port fail" );
		goto _exit_failure;
	}
	
	nRC = inet_aton( g_tEnv.szIp, &(tSockAddr.sin_addr) );
	if ( 0 == nRC )
	{
		LOG_ERR_F( "inet_aton fail <%d>", nRC );
		goto _exit_failure;
	}

	nRC = setsockopt( *pnListenFd, SOL_SOCKET, SO_REUSEADDR, 1, sizeof(int) );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "setsockopt fail <%d>", nRC );
		goto _exit_failure;
	}

	/*
	 *	Bind Socket to address
	 */
	nRC = bind( *pnListenFd, &tSockAddr, sizeof(tSockAddr) );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "bind fail <%d>", nRC );
		goto _exit_failure;
	}

	/*
	 *	Listen for connection
	 */
	nRC = listen( *pnListenFd, BACK_LOG );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "listen fail <%d>", nRC );
		goto _exit_failure;
	}

	return RAS_rOK;

_exit_failure:
	nRC = close( *pnListenFd );
	if ( -1 == nRC )
	{
		LOG_ERR_F( "close <%d>", nRC );
	}

	return RAS_rErrSocketInit;
}
