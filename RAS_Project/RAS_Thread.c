/* RAS_Thread.c */
#include "RAS_Inc.h"

extern TRHEAD_t g_tThread;

int THREAD_Init()
{
	int nRC = 0;
	int nIndex = 0;

	for ( nIndex = 0; nIndex < WORKER_THREAD_CNT; i++ )
	{
		nRC = pthread_create( &(g_tThread[ nIndex ].nThreadId), NULL, THREAD_Run, NULL );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_create fail <%d>", nRC );
			goto _exit_failure;
		}
	}

	return RAS_rOK;

_exit_failure:
	for ( nIndex = 0; nIndex < WORKER_THREAD_CNT; i++ )
	{
		nRC = pthread_join( &(g_tThread[ nIndex ].nThreadId), NULL );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_join fail <%d>", nRC );
		}
	}	

	return RAS_rErrThreadInit;
}

int THREAD_Run( void *ptArg )
{
	ptArg = ptArg;

	return RAS_rOK;
}
