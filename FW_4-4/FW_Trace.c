/* FW_Trace.c */
#include "FW_Header.h" 

//Trace Key 는 사원 번호
int TRACE_Init()
{
	xcomm_ipc_t *tXcomm = NULL;

	int nRC = 0;

	iipc_key_t tKeyMxcomm;
	iipc_ds_t tIpc;

	memset( tKeyMxcomm, 0x00, sizeof(tKeyMxcomm) );
	memset( tIpc, 0x00, sizeof(tIpc) );

	tXcomm->msgid = MI_KEY_TRACE;
	tXcomm->header.extra_id1 = 1;
	tXcomm->header.extra_id2 = 0;

	tKeyMxcomm = TAP_ipc_getkey( &tIpc, "MXCOMM" );
	if ( IPC_NOPROC((iipc_key_t)(0xff)) == tKeyMxcomm )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail", __func__ );
		//return
	}

	//TAP_ipc_msgsnd
	//
}
