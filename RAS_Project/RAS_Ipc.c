/* RAS_Ipc.c */
#include "RAS_Inc.h"

int IPC_Init( mpipc_t *ptMpipc )
{
	int nRC = 0;

	ptMpipc = mpipc_init( MODULE_NAME, CHECK_INTERVAL_MSEC, STOP_GRACE_TIME_MSEC );
	if ( NULL == ptMpipc )
	{
		LOG_ERR_F( "mpipc_fail" );
		return RAS_rErrFail; 
	}

	nRC = mpipc_regi_hdlr( ptMpipc, IPC_Handler, NULL );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpipc_regi_hdlr fail <%d>", nRC );
		IPC_Destroy( ptMpipc );
		return RAS_rErrFail;
	}

	nRC = mpipc_start( ptMpipc );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpipc_start fail <%d>", nRC );
		IPC_Destroy( ptMpipc );
		return RAS_rErrFail;
	}

	return RAS_rOK;
}

int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptRecvMsg, void *pvData )
{
	CHECK_PARAM_RC( ptMpipc );
	CHECK_PARAM_RC( ptRecvMsg );
	pvData = pvData;

	switch( ptRecvMsg->buf.mtype )
	{
		case MSG_ID_ADD_CLI_IP:
		case MSG_ID_DIS_CLI_IP:
		case MSG_ID_DEL_CLI_IP:
		case MSG_ID_ADD_CLI_IP_TRC:
		case MSG_ID_DIS_CLI_IP_TRC:
		case MSG_ID_DEL_CLI_IP_TRC:
		case MSG_ID_DIS_USR_INFO:
		case MSG_ID_DEL_USR_INFO:
		{
			return MPIPC_HDLR_RET_NOT_FOR_ME;
		}
			break;
		default:
			break;
	}

	return MPIPC_HDLR_RET_DONE;
}

void IPC_Destroy( mpipc_t *ptMpipc )
{
	int nRC = 0;

	if ( NULL != ptMpipc )
	{
		nRC = mpipc_stop( ptMpipc );
		if ( 0 > nRC )
		{
			LOG_ERR_F( "mpipc_stop fail <%d>", nRC );
		}
		LOG_DBG_F( "mpipc_stop" );
		mpipc_destroy( ptMpipc );
		LOG_DBG_F( "mpipc_destroy" );
	}
	
	return;
}
