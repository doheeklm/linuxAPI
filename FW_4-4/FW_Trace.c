/* FW_Trace.c */
#include "FW_Header.h" 

int TRACE_MakeTrace( int nId, int nMsgType )
{
	int nRC = 0;
	int nXcommSize = 0;

	char szKey[256];
	char szTrcResult[2048];

	memset( szKey, 0x00, sizeof(szKey) );
	memset( szTrcResult, 0x00, sizeof(szTrcResult) );
	
	iipc_key_t tKey;
	iipc_msg_t tMsg;

	xcomm_ipc_t *ptXcomm = NULL;
	ptXcomm = (xcomm_ipc_t *)tMsg.buf.msgq_buf;

	tKey = TAP_ipc_getkey( mpipc_tap_ipc(ptMpipc), "MXCOMM" );
	if ( IPC_NOPROC == tKey )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_getkey() fail", __func__ );
		return IPC_FAIL;
	}

	//1. unsigned short keylen;
	snprintf( szKey, sizeof(szKey), "%d", nId );
	szKey[ strlen(szKey) ] = '\0';
	if ( 128 < strlen(szKey) )
	{
		MPGLOG_ERR( "%s:: key length over 128", __func__ );
		return DATA_OVERFLOW_FAIL;
	}
	ptXcomm->keylen = htons( strlen(szKey) );

	//2. int msg_id;
	ptXcomm->msg_id = MI_KEY_TRACE;
	
	//3. char proto[8];
	//프로토콜 변수를 입력할 수 있는 char형 변수
	//SIP,DIAM,MAP 등의 정보를 입력하면 OMP에서 로그 조회시 출력됨
	snprintf( ptXcomm->proto, sizeof(ptXcomm->proto), "EDU" );
	ptXcomm->proto[ strlen(ptXcomm->proto) ] = '\0';
	
	//4. char body[4000];
	snprintf( szTrcResult, sizeof(szTrcResult), "Message Type: %d", nMsgType );
	szTrcResult[ strlen(szTrcResult) ] = '\0';
	snprintf( ptXcomm->body, sizeof(ptXcomm->body), "%s%s", szKey, szTrcResult );
	ptXcomm->body[ strlen(ptXcomm->body) ] = '\0';

	//5. XCOMM_HEAD header;

	//5-1. short src_sys;
	//자신의 시스템 아이디를 가져온다.
	ptXcomm->header.src_sys = oam_get_system_id(NULL);
	if ( 0 > ptXcomm->header.src_sys )
	{
		MPGLOG_ERR( "%s:: oam_get_system_id() fail", __func__ );
		return RC_FAIL;
	}

	//5-2. char src_proc[OAM_PROCNAME_MAX];
	snprintf( ptXcomm->header.src_proc, OAM_PROCNAME_MAX, "%s", PROCNAME_SERVER );

	//5-3. short dst_sys;
	//현재 시스템에 대한 OMP 시스템 아이디를 가져온다.
	ptXcomm->header.dst_sys = oam_get_omp_system_id();
	if ( 0 > ptXcomm->header.dst_sys )
	{
		MPGLOG_ERR( "%s:: oam_get_omp_system_id() fail", __func__ );
		return RC_FAIL;
	}

	//5-4. char dst_proc[OAM_PROCNAME_MAX];
	//OTRACE에서 시스템별 TRACE 메시지를 저장함
	snprintf( ptXcomm->header.dst_proc, OAM_PROCNAME_MAX, "OTRACE" );
	
	//5-5. short msg_len;
	//proto 크기 + keylen 크기 + (NULL포함) body 문자열 길이
	//(예제 SLF Trace) ptXcomm->header.msg_len = sizeof(ptXcomm->proto) + nKeyLen + nBodyLen + sizeof(unsigned short);
	ptXcomm->header.msg_len = sizeof(ptXcomm->proto) + ptXcomm->keylen + strlen(ptXcomm->body) + sizeof(unsigned short);

	//5-6. short extra_id1;
	ptXcomm->header.extra_id1 = 1;

	//5-7. short extra_id2;
	ptXcomm->header.extra_id2 = 0;

	//5-8. short dummy;
	//ptXcomm->header.dummy = 

	/*
	 *	Send Message
	 */	
	nXcommSize = sizeof(ptXcomm->msg_id) + sizeof(ptXcomm->header) + sizeof(ptXcomm->header.msg_len);
	
	MPGLOG_DBG( "%s:: nXcommSize=%d", __func__, nXcommSize );

	nRC = TAP_ipc_msgsndm( mpipc_tap_ipc(ptMpipc), tKey, ptXcomm, nXcommSize, IPC_NONBLOCK );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: TAP_ipc_msgsnd() fail=%d", __func__, nRC );
		return IPC_FAIL;
	}

	return RC_SUCCESS;


	
	// period time 분단위 (시간은 registry value로)
}

