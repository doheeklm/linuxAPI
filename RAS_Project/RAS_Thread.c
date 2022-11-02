/* RAS_Thread.c */
#include "RAS_Inc.h"

extern THREAD_t g_tThread[THREAD_CNT];

int THREAD_Init()
{
	int nRC = 0;
	int nIndex = 0;

	for ( nIndex = 0; nIndex < THREAD_CNT; nIndex++ )
	{
		nRC = pthread_create( &g_tThread[nIndex].nThreadId, NULL, THREAD_Run, NULL );
		if ( nRC != 0 )
		{
			LOG_ERR_F( "pthread_create fail <%d>", nRC );
			return RAS_rErrThreadInit;
		}
	}

	return RAS_rOK;
}

void *THREAD_Run( void *pvArg )
{
	pvArg = pvArg;

	printf( "Thread Run\n" );

	pthread_exit( NULL );	
}
