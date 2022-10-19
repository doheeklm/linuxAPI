/* FW_Trace.c */
#include "FW_Header.h" 

int TRACE_MakeTrace( struct REQUEST_s * ptRequestFromClient )
{
	if ( NULL == ptRequestFromClient )
	{
		MPGLOG_ERR( "%s:: ptRequestFromClient NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;
	int nSizeMXCOMM = 0;

	char szRegiKey[256];
	memset( szRegiKey, 0x00, sizeof(szRegiKey) );

	iipc_key_t tKeyMXCOMM = IPC_NOPROC;
	iipc_msg_t tMsg;

	xcomm_ipc_t *ptXcomm = NULL;
	ptXcomm = (xcomm_ipc_t *)&tMsg.buf;

	tKeyMXCOMM = TAP_ipc_getkey( mpipc_tap_ipc(g_ptMpipc), "MXCOMM" );
	if ( IPC_NOPROC == tKeyMXCOMM )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail<%d>", __func__, ipc_errno );
		return IPC_FAIL;
	}

	/* keylen */
	snprintf( szRegiKey, sizeof(szRegiKey), "%d", ptRequestFromClient->nId );
	szRegiKey[ strlen(szRegiKey) ] = '\0';
	if ( 128 < strlen(szRegiKey) )
	{
		MPGLOG_ERR( "%s:: key length over 128", __func__ );
		return DATA_OVERFLOW_FAIL;
	}
	ptXcomm->keylen = htons( strlen(szRegiKey) );
	/* msg_id */
	ptXcomm->msg_id = MI_KEY_TRACE;
	snprintf( ptXcomm->proto, sizeof(ptXcomm->proto), "TRCTEST" );
	/* proto */
	ptXcomm->proto[ strlen(ptXcomm->proto) ] = '\0';
	/* body */
	snprintf( ptXcomm->body, sizeof(ptXcomm->body),
			"%smsgtype<%d>name<%s>position<%s>team<%s>phone<%s>",
			szRegiKey, ptRequestFromClient->nMsgType,
		   	ptRequestFromClient->szName, ptRequestFromClient->szPosition,
			ptRequestFromClient->szTeam, ptRequestFromClient->szPhone );
	ptXcomm->body[ strlen(ptXcomm->body) ] = '\0';
	MPGLOG_DBG( "%s:: ptXcomm->body = %s", __func__, ptXcomm->body );
	/* header.src_sys */
	ptXcomm->header.src_sys = oam_get_system_id(NULL);
	if ( 0 > ptXcomm->header.src_sys )
	{
		MPGLOG_ERR( "%s:: oam_get_system_id() fail", __func__ );
		return RC_FAIL;
	}
	/* header.src_proc */
	snprintf( ptXcomm->header.src_proc, OAM_PROCNAME_MAX, "%s", PROCNAME_SERVER );
	ptXcomm->header.src_proc[ strlen(ptXcomm->header.src_proc) ] = '\0';
	/* header.dst_sys */
	ptXcomm->header.dst_sys = oam_get_omp_system_id();
	if ( 0 > ptXcomm->header.dst_sys )
	{
		MPGLOG_ERR( "%s:: oam_get_omp_system_id() fail", __func__ );
		return RC_FAIL;
	}
	/* header.dst_proc */
	snprintf( ptXcomm->header.dst_proc, OAM_PROCNAME_MAX, "OTRACE" );
	ptXcomm->header.dst_proc[ strlen(ptXcomm->header.dst_proc) ] = '\0';
	/* header.msg_len */
	ptXcomm->header.msg_len = sizeof(ptXcomm->proto) + ptXcomm->keylen + strlen(ptXcomm->body) + 1;
	MPGLOG_DBG( "%s:: ptXcomm->header.msg_len=%d", __func__, ptXcomm->header.msg_len );
	/* header.extra_id1 */
	ptXcomm->header.extra_id1 = 1;
	/* header.extra_id2 */
	ptXcomm->header.extra_id2 = 0;
	/* ptXcomm->header.dummy =  */
	
	nSizeMXCOMM = (int)sizeof(ptXcomm->msg_id) + (int)sizeof(ptXcomm->header) + ptXcomm->header.msg_len;
	MPGLOG_DBG( "%s:: nSizeMXCOMM=%d", __func__, nSizeMXCOMM );

	nRC = TAP_ipc_msgsndm( mpipc_tap_ipc(g_ptMpipc), tKeyMXCOMM, ptXcomm, nSizeMXCOMM, IPC_NONBLOCK );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_msgsnd() fail<%d>", __func__, nRC );
		return IPC_FAIL;
	}

	return RC_SUCCESS;
}
