/* RAS_Trace.c */
#include "RAS_Inc.h"

extern mpipc_t *g_ptMpipc;

int TRACE_MakeTrace( int nHttpType, const char *pszIp,
		struct REQUEST_s *ptRequest, struct RESPONSE_s *ptResponse )
{
	CHECK_PARAM_RC( pszIp );
	CHECK_PARAM_RC( ptRequest );
	CHECK_PARAM_RC( ptResponse );
	
	int nRC = 0;
	int nSizeMXCOMM = 0;
	iipc_key_t tKeyMXCOMM = IPC_NOPROC;
	iipc_msg_t tMsg;
	xcomm_ipc_t *ptXcomm = NULL;
	ptXcomm = (xcomm_ipc_t *)&tMsg.buf;
	
	tKeyMXCOMM = TAP_ipc_getkey( mpipc_tap_ipc(g_ptMpipc), MXCOMM );
	if ( IPC_NOPROC == tKeyMXCOMM )
	{
		LOG_ERR_F( "TAP_ipc_getkey fail <%d>", ipc_errno );
		return RAS_rErrFail;
	}

	/* keylen */
	if ( 128 < strlen(pszIp) )
	{
		LOG_ERR_F( "key length over 128" );
		return RAS_rErrOverflow;
	}
	ptXcomm->keylen = htons( strlen(pszIp) );
	/* msg_id */
	ptXcomm->msg_id = MI_KEY_TRACE;
	snprintf( ptXcomm->proto, sizeof(ptXcomm->proto), "TRC_IP" );
	/* proto */
	ptXcomm->proto[ strlen(ptXcomm->proto) ] = '\0';
	/* body */
	if ( HTTP_TYPE_REQUEST == nHttpType )
	{
		snprintf( ptXcomm->body, sizeof(ptXcomm->body),
				"%s%s\nMethod = %s\nPath = %s\nContent-Length = %d\n%s",
				pszIp, REQUEST_DESC,
				ptRequest->szMethod, ptRequest->szPath,
				ptRequest->nContentLength, ptRequest->szBody );
		ptXcomm->body[ strlen(ptXcomm->body) ] = '\0';
	}
	else if ( HTTP_TYPE_RESPONSE == nHttpType )
	{
		snprintf( ptXcomm->body, sizeof(ptXcomm->body),
				"%s%s\nStatus Code = %d\nStatus Msg = %s\nContent-Length = %d\n%s",
				pszIp, RESPONSE_DESC,
				ptResponse->nStatusCode, HTTP_GetStatusMsg(ptResponse->nStatusCode),
				ptResponse->nContentLength, ptResponse->szBody );
		ptXcomm->body[ strlen(ptXcomm->body) ] = '\0';
	}
	else
	{
		return RAS_rErrFail;
	}
	
	LOG_DBG_F( "\n%s\n%s\n%s", LINE, ptXcomm->body, LINE );
	/* header.src_sys */
	ptXcomm->header.src_sys = oam_get_system_id(NULL);
	if ( 0 > ptXcomm->header.src_sys )
	{
		LOG_ERR_F( "oam_get_system_id fail" );
		return RAS_rErrFail;
	}
	/* header.src_proc */
	snprintf( ptXcomm->header.src_proc, OAM_PROCNAME_MAX, "%s", PROCESS_NAME );
	ptXcomm->header.src_proc[ strlen(ptXcomm->header.src_proc) ] = '\0';
	/* header.dst_sys */
	ptXcomm->header.dst_sys = oam_get_omp_system_id();
	if ( 0 > ptXcomm->header.dst_sys )
	{
		LOG_ERR_F( "oam_get_omp_system_id fail");
		return RAS_rErrFail;
	}
	/* header.dst_proc */
	snprintf( ptXcomm->header.dst_proc, OAM_PROCNAME_MAX, "OTRACE" );
	ptXcomm->header.dst_proc[ strlen(ptXcomm->header.dst_proc) ] = '\0';
	/* header.msg_len */
	ptXcomm->header.msg_len = sizeof(ptXcomm->proto) + ptXcomm->keylen + strlen(ptXcomm->body) + 1;
	//LOG_DBG_F( "ptXcomm->header.msg_len (%d)", ptXcomm->header.msg_len );
	/* header.extra_id1 */
	ptXcomm->header.extra_id1 = 1;
	/* header.extra_id2 */
	ptXcomm->header.extra_id2 = 0;
	/* ptXcomm->header.dummy =  */

	nSizeMXCOMM = (int)sizeof(ptXcomm->msg_id) + (int)sizeof(ptXcomm->header) + ptXcomm->header.msg_len;
	//LOG_DBG_F( "nSizeMXCOMM (%d)", nSizeMXCOMM );

	nRC = TAP_ipc_msgsndm( mpipc_tap_ipc(g_ptMpipc), tKeyMXCOMM, ptXcomm, nSizeMXCOMM, IPC_NONBLOCK );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "TAP_ipc_msgsnd fail <%d>", ipc_errno );
		return RAS_rErrIpcSend;
	}

	//LOG_DBG_F( "Send Trace" );
	return RAS_rOK;
}
