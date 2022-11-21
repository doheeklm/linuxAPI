/* RAS_Event.h */
#ifndef _RAS_EVENT_H_
#define _RAS_EVENT_H_

#define MAX_CONNECT		5
#define TIME_OUT		-1

int EVENT_Init( int *pnEpollFd );
int EVENT_Add( int nListenFd, int nEpollFd );
int EVENT_Wait( int nListenFd, int nEpollFd );

#endif /* _RAS_EVENT_H_ */
