/* RAS_Event.h */
#ifndef _RAS_EVENT_H_
#define _RAS_EVENT_H_

#define MAX_CONNECT		5
#define TIME_OUT		-1

int EVENT_Init( int *pnEpollFd );
int EVENT_AddToList( int nListenFd, int nEpollFd );
int EVENT_Wait( int nListenFd, int nEpollFd, DB_t tDBMain );

#endif /* _RAS_EVENT_H_ */
