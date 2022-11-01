/* RAS_Log.c */
#include "RAS_Inc.h"

extern Env_t g_tEnv;

int LOG_Init()
{
	int nRC = 0;

	nRC = MPGLOG_INIT( PROCESS_NAME, NULL, g_tEnv.nLogMode, g_tEnv.nLogLevel );
	if ( 0 > nRC )
	{
		printf( "MPGLOG_INIT fail <%d>\n", nRC );
		return RAS_rErrLogInit;
	}

	return RAS_rOK;
}
