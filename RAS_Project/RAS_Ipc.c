/* RAS_Ipc.c */
#include "RAS_Inc.h"

extern mpipc_t *g_ptMpipc;

int IPC_Init()
{
	int nRC = 0;

	g_ptMpipc = mpipc_init( PROCESS_NAME, CHECK_INTERVAL_MSEC, STOP_GRACE_TIME_MSEC );
	if ( NULL == g_ptMpipc )
	{
		LOG_ERR_F( "mpipc_fail" );
		return RAS_rErrFail; 
	}

	LOG_SVC_F( "mpipc_init" );

	nRC = mpipc_regi_hdlr( g_ptMpipc, IPC_Handler, NULL );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpipc_regi_hdlr fail <%d>", nRC );
		return RAS_rErrFail;
	}

	LOG_SVC_F( "mpipc_regi_hdlr" );
	
	nRC = mpipc_start( g_ptMpipc );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "mpipc_start fail <%d>", nRC );
		return RAS_rErrFail;
	}

	LOG_SVC_F( "mpipc_start" );

	return RAS_rOK;
}

int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptRecvMsg, void *pvData )
{
	CHECK_PARAM_RC( ptMpipc );
	CHECK_PARAM_RC( ptRecvMsg );
	pvData = pvData;

	MESSAGE *ptMsg = (MESSAGE *)&ptRecvMsg->buf;

	switch( ptMsg->head.msg_id )
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
			LOG_DBG_F( "msg_id = %d", ptMsg->head.msg_id );
			return MPIPC_HDLR_RET_NOT_FOR_ME;
		}
			break;
		default:
		{
			LOG_ERR_F( "unknown message. src_proc=%d id=%d", ptRecvMsg->u.h.src, ptMsg->head.msg_id );
			return MPIPC_HDLR_RET_NOT_FOR_ME;
		}
			break;
	}

	LOG_DBG_F( "msg_id = %d", ptMsg->head.msg_id );
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
