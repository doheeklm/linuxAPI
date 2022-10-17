/* FW_Trace.c */
#include "FW_Header.h" 

int TRACE_AddTrace( char *pszKey )
{
	int nRC = 0;

	iipc_key_t tKey;
	iipc_msg_t tMsg;

	char szTrcResult[2048];
	memset( szTrcResult, 0x00, sizeof(szTrcResult) );

	xcomm_ipc_t *ptXcomm = (xcomm_ipc_t *)&tMsg.buf;

	tKey = TAP_ipc_getkey( mpipc_tap_ipc(ptMpipc), "MXCOMM" );
	if ( IPC_NOPROC == tKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail", __func__ );
		return IPC_FAIL;
	}

	//1. unsigned short keylen; (최대 파일이름 길이인 128을 넘지 않아야함)
	ptXcomm->keylen = htons( strlen(pszKey) );

	//2. int msg_id;
	ptXcomm->msg_id = MI_KEY_TRACE;
	
	//3. char proto[8];
	//snprintf( ptXcomm->proto, sizeof(ptXcomm->proto), "TEST" );
	//ptXcomm->proto[ strlen(ptXcomm->proto) ] = '\0';
	
	//4. char body[4000];
	snprintf( szTrcResult, sizeof(szTrcResult), "Add Trace Success" );
	szTrcResult[ strlen(szTrcResult) ] = '\0';
	
	snprintf( ptXcomm->body, sizeof(ptXcomm->body), "%s%s", pszKey, szTrcResult );
	ptXcomm->body[ strlen(ptXcomm->body) ] = '\0';
	
	//5. XCOMM_HEAD header;
	//5-1. short src_sys; 자신의 시스템 아이디를 가져온다.
	ptXcomm->header.src_sys = oam_get_system_id(NULL);
	if ( 0 > ptXcomm->header.src_sys )
	{
		MPGLOG_ERR( "%s:: oam_get_system_id() fail", __func__ );
		return RC_FAIL;
	}
	//5-2. char src_proc[OAM_PROCNAME_MAX];
	snprintf( ptXcomm->header.src_proc, OAM_PROCNAME_MAX, "%s", PROCNAME_SERVER );
	//5-3. short dst_sys; 현재 시스템에 대한 OMP 시스템 아이디를 가져온다.
	ptXcomm->header.dst_sys = oam_get_omp_system_id();
	if ( 0 > ptXcomm->header.dst_sys )
	{
		MPGLOG_ERR( "%s:: oam_get_omp_system_id() fail", __func__ );
		return RC_FAIL;
	}
	//5-4. char dst_proc[OAM_PROCNAME_MAX];
	snprintf( ptXcomm->header.dst_proc, OAM_PROCNAME_MAX, "OTRACE" );
	//5-5. short msg_len; proto 크기 + keylen 크기 + (NULL포함)body 문자열 길이
	ptXcomm->header.msg_len = sizeof(ptXcomm->proto) + sizeof(ptXcomm->keylen) + strlen(ptXcomm->body);
	//5-6. short extra_id1;
	ptXcomm->header.extra_id1 = 1;
	//5-7. short extra_id2;
	ptXcomm->header.extra_id2 = 0;
	//5-8. short dummy;
	//ptXcomm->header.dummy = 

	//Send Message
	int nIpcMsgLen = 0;
	nIpcMsgLen = sizeof(ptXcomm->msg_id) + sizeof(ptXcomm->header) + sizeof(ptXcomm->header.msg_len);

	MPGLOG_DBG( "%s:: nIpcMsgLen=%d", __func__, nIpcMsgLen );

	nRC = TAP_ipc_msgsndm( mpipc_tap_ipc(ptMpipc), tKey, ptXcomm, nIpcMsgLen, IPC_NONBLOCK );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_msgsnd() fail=%d", __func__, nRC );
		return IPC_FAIL;
	}
	
	return RC_SUCCESS;
	
	//TODO period time 분단위 나중에 Add 하기(시간은 registry value?)
}

int TRACE_DisTrace( char *pszKey )
{
	pszKey = pszKey;
	

	return RC_SUCCESS;
}

int TRACE_DelTrace( char *pszKey )
{
	int nRC = 0;

	char szKey[TAP_REGI_KEY_SIZE];
	memset( szKey, 0x00, sizeof(szKey) );

	/*
	 *	Delete Key(Phone Num) in Registry
	 */
	snprintf( szKey, sizeof(szKey), "%s%s", REGI_KEY_DIR, pszKey );
	szKey[ strlen(szKey) ] = '\0';

	nRC = TAP_Registry_udp_key_delete( szKey, strlen(szKey), TAP_REGISTRY_UDP_REMOVEALL, REGI_MAN_SYSTEM_ID );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_Regsitry_udp_key_delete() fail=%d", __func__, nRC );
		return RC_FAIL;
	}
		
	return RC_SUCCESS;
}
