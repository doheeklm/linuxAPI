/* RAS_Thread.h */
#ifndef _RAS_THREAD_H
#define _RAS_THREAD_H

int THREAD_Init();
void THREAD_Destroy();
void *THREAD_Run( void *pvArg );

#endif /* _RAS_THREAD_H_ */
