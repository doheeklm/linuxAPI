/* RAS_Event.h */
#ifndef _RAS_EVENT_H_
#define _RAS_EVENT_H_

#define MAX_CLIENT_CONNECT	5
#define TIME_OUT			-1

int EVENT_Init( int *pnEpollFd, int nListenFd );
int EVENT_WaitAndAccept( int nEpollFd, int nListenFd, DB_t tDB );

#endif /* _RAS_EVENT_H_ */
