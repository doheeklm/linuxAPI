/* FW_Ipc.c */
#include "FW_Header.h"

int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptRecvMsg, void *pvData )
{
	if ( NULL == ptMpipc )
	{
		MPGLOG_ERR( "%s:: ptMpipc NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptRecvMsg )
	{
		MPGLOG_ERR( "%s:: ptRecvMsg NULL", __func__ );
		return NULL_FAIL;
	}
	
	pvData = pvData;

	int nRC = 0;

	REQUEST_t *ptRequestFromClient = NULL;
	RESPONSE_t *ptResponseToClient = NULL;

	iipc_msg_t tSendMsg;
	iipc_key_t tKey = IPC_NOPROC;

	tKey = TAP_ipc_getkey( mpipc_tap_ipc(ptMpipc), PROCNAME_SERVER );
	if ( IPC_NOPROC == tKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail=%d", __func__, ipc_errno );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}
	
	ptRequestFromClient = (REQUEST_t *)ptRecvMsg->buf.msgq_buf;
	ptResponseToClient = (RESPONSE_t *)tSendMsg.buf.msgq_buf;

	tSendMsg.buf.mtype = ptRecvMsg->buf.mtype;

	tSendMsg.u.h.dst = ptRecvMsg->u.h.src;
	tSendMsg.u.h.src = tKey;
	tSendMsg.u.h.len = sizeof(struct RESPONSE_s);

	switch ( ptRecvMsg->buf.mtype )
	{
		case MMC_ADD_INFO_ID:
		case MMC_DIS_INFO_ID:
		case MMC_CHG_INFO_ID:
		case MMC_DEL_INFO_ID:
		case MMC_ADD_TRC_ID:
		case MMC_DIS_TRC_ID:
		case MMC_DEL_TRC_ID:
		{
			return MPIPC_HDLR_RET_NOT_FOR_ME;
		}
			break;
	}

	nRC = REGI_CheckKey( ptRequestFromClient->nId );
	if ( RC_SUCCESS != nRC )
	{
		MPGLOG_DBG( "%s:: id %d not exist in trace", __func__, ptRequestFromClient->nId  );
		return MPIPC_HDLR_RET_DONE;
	}

	nRC = TRACE_MakeTrace( ptRequestFromClient->nId, ptRequestFromClient->nMsgType );
	if ( RC_SUCCESS != nRC )
	{
		MPGLOG_DBG( "%s:: TRACE_MakeTrace() fail=%d", __func__, nRC );
		return MPIPC_HDLR_RET_DONE;
	}

	switch ( ptRequestFromClient->nMsgType )
	{
		case MTYPE_INSERT:
		{
			MPGLOG_DBG( "[RECV] mtype = %d | name = %s | position = %s | team = %s | phone = %s",
					ptRequestFromClient->nMsgType, ptRequestFromClient->szName, ptRequestFromClient->szPosition,
					ptRequestFromClient->szTeam, ptRequestFromClient->szPhone );
	
			nRC = DB_Insert( ptRequestFromClient );			
			if ( RC_SUCCESS != nRC )
			{
				ptResponseToClient->nRC = RC_FAIL;
				break;
			}

			ptResponseToClient->nRC = RC_SUCCESS;
		}
			break;
		case MTYPE_SELECTALL:
		case MTYPE_SELECTONE:
		{
			MPGLOG_DBG( "[RECV] mtype = %d", ptRequestFromClient->nMsgType );

			nRC = DB_Select( ptRequestFromClient, ptResponseToClient );
			if ( DAL_EXEC_ZERO == nRC || RC_SUCCESS != nRC )
			{
				ptResponseToClient->nRC = RC_FAIL;
				break;
			}

			ptResponseToClient->nRC = RC_SUCCESS;
		}
			break;
		case MTYPE_UPDATE:
		{
			MPGLOG_DBG( "[RECV] mtype = %d | id = %d | name = %s | position = %s | team = %s | phone = %s",
					ptRequestFromClient->nMsgType, ptRequestFromClient->nId, ptRequestFromClient->szName,
					ptRequestFromClient->szPosition, ptRequestFromClient->szTeam,
					ptRequestFromClient->szPhone );

			nRC = DB_Update( ptRequestFromClient );
			if ( DAL_EXEC_ZERO == nRC || RC_SUCCESS != nRC || INPUT_FAIL == nRC )
			{
				ptResponseToClient->nRC = RC_FAIL;
				break;
			}

			ptResponseToClient->nRC = RC_SUCCESS;
		}
			break;
		case MTYPE_DELETE:
		{
			MPGLOG_DBG( "[RECV] mtype = %d", ptRequestFromClient->nMsgType );
			
			nRC = DB_Delete( ptRequestFromClient );
			if ( DAL_EXEC_ZERO == nRC || RC_SUCCESS != nRC )
			{
				ptResponseToClient->nRC = RC_FAIL;
				break;
			}

			ptResponseToClient->nRC = RC_SUCCESS;
		}
			break;
		default:
		{
			MPGLOG_DBG( "%s:: unknown IPC message", __func__ );
			
			return MPIPC_HDLR_RET_NOT_FOR_ME;
		}
			break;
	}

	ptResponseToClient->nMsgType = ptRequestFromClient->nMsgType;

	nRC = TAP_ipc_msgsnd( mpipc_tap_ipc(ptMpipc), &tSendMsg, IPC_BLOCK );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_msgsnd() fail=%d", __func__, ipc_errno );
		return MPIPC_HDLR_RET_DONE;	
	}

	MPGLOG_DBG( "[SEND] mtype = %d", ptResponseToClient->nMsgType );
	
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
			MPGLOG_ERR( "%s:: mpipc_stop fail=%d", __func__, nRC );
		}
		mpipc_destroy( ptMpipc );
	}

	return;
}
