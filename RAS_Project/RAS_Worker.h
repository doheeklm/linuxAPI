/* RAS_Worker.h */
#ifndef _RAS_WORKER_H
#define _RAS_WORKER_H

typedef struct WORKER_s
{
	pthread_t	nThreadId;
	int			nEpollFd;
	DB_t		tDB;

} WORKER_t;

int		WORKER_Init();
void	WORKER_Destroy();
void	*WORKER_Run( void *pvArg );
void	WORKER_CleanUp( void *pvArg );

#endif /* _RAS_WORKER_H_ */
