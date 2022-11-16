/* RAS_Worker.h */
#ifndef _RAS_WORKER_H
#define _RAS_WORKER_H



int WORKER_Init();
void WORKER_Destroy();
void *WORKER_Run( void *pvArg );

#endif /* _RAS_WORKER_H_ */
