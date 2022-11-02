/* RAS_Socket.h */
#ifndef _RAS_SOCKET_H_
#define _RAS_SOCKET_H_

#define BACK_LOG	5 //연결 대기 제한 수
/*
 *	Client가 connect()를 하게 되면 Client는 Server에게 connect request를 보내게 되는데,
 *	Server는 자신과 연결 되어있는 Client가 없다면 대기 큐에 있는 Clinet를 한명 잡아다가 연결을 하게 된다.
 *	이때 대기 큐의 제한을 걸어주는 것이 Backlog 큐이다.
 *	대기 큐가 5일 때, 10개의 Client가 연결 신청하고 대기 상태에 있다고 해서 전부 연결대기가 아니고 5개만이 연결 대기 상태이다.
 */

int SOCKET_Init( int *pnFd );

#endif /* _RAS_SOCKET_H_ */
