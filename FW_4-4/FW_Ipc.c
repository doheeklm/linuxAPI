/* FW_Ipc.c */
#include "FW_Header.h"

int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptIpcMsg, void *pvData )
{
	if ( NULL == ptMpipc )
	{
		MPGLOG_ERR( "%s:: ptMpipc NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL == ptIpcMsg )
	{
		MPGLOG_ERR( "%s:: ptIpcMsg NULL", __func__ );
		return NULL_FAIL;
	}
	
	pvData = pvData;

	int nMsgId = 0;

	MESSAGE *ptMsg = (MESSAGE *)&ptIpcMsg->buf;
	
	nMsgId = ptMsg->head.msg_id;
	
	switch( nMsgId )
	{
		/*
		 *	Message ID 등록:
		 *	처리해야할 IPC Mesasge ID를 등록한다.
		 *	Message ID는 프로세스 내부에 정의하여 사용해도 되고 기존처럼 msg_id.h 에 등록해서 사용해도 무방하다.
		 */
		
		/* IPC Message Processing */
//		case MMC_ADD_ID:
//		case MMC_DIS_ID:
//		case MMC_CHG_ID:
//		case MMC_DEL_ID:
//			MPGLOG_DBG( "%s:: nMsgId=%d", __func__, nMsgId );
//			break;
		default:
		{
			MPGLOG_DBG( "%s:: unknown IPC message. MMC M=%d", __func__, nMsgId );
			//src=%d | &ptIpcMsg->u.h.src
		
			/*
			 *	OAMMMC Handler 호출:
			 *	OAMMMC에 등록된 Message ID를 처리하기 위해 다음 리턴값을 사용한다.
			 *	OAMMMC는 IPC Handler를 공유하기 때문에 반드시 다음 값으로 리턴해야 한다.
			 */

			return MPIPC_HDLR_RET_NOT_FOR_ME;
		}
			break;
	}
	//에러나면 MPIPC_HDLR_RET_ERROR
	
	/*
	 *	IPC Handler 리턴:
	 *	다음 값으로 리턴하여 OAMMMC Handler가 호출되지 않도록 한다.
	 */
	//	return MPIPC_HDLR_RET_DONE;
	
	return MPIPC_HDLR_RET_NOT_FOR_ME;
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
