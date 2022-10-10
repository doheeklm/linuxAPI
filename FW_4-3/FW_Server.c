/* FW_4-3/FW_Server.c */
#include "FW_Inc.h"

void SignalHandler( int nSig );
int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptIpcMsg, void *pvData );
void IPC_Destroy( mpipc_t *ptMpipc );

int MMC_Init( char *pszModule, oammmc_t *ptOammmc, mpipc_t *ptMpipc );
void MMC_Destroy( oammmc_t *ptOammmc );
int MMC_Handler_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );
int MMC_Handler_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );
int MMC_Handler_Chg( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );
int MMC_Handler_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg );

int PSTMT_Init();
void PSTMT_Destroy();

oammmc_cmd_t tCommandTable[] =
{
	{ 1, MMC_ADD, MMC_ADD_ID, MMC_Handler_Add, 4, 4, atArgsAdd, "Cmd_Desc: Add Employee Info" },
	{ 2, MMC_DIS, MMC_DIS_ID, MMC_Handler_Dis, 0, 5, atArgsDis, "Cmd_Desc: Display Employee Info" },
	{ 3, MMC_CHG, MMC_CHG_ID, MMC_Handler_Chg, 1, 5, atArgsChg, "Cmd_Desc: Change Employee Info" },
	{ 4, MMC_DEL, MMC_DEL_ID, MMC_Handler_Del, 1, 1, atArgsDel, "Cmd_Desc: Delete Employee Info" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int main( int argc, char *argv[] )
{
	argc = argc;
	argv = argv;

	tb_signal( SIGHUP, SignalHandler ); //1
	tb_signal( SIGINT, SignalHandler ); //2
	tb_signal( SIGQUIT, SignalHandler ); //3
	tb_signal( SIGTERM, SignalHandler ); //15

	int nRC = 0;
	
	oammmc_t *ptOammmc = NULL;
	mpipc_t *ptMpipc = NULL;

	/*
	 *	MPGLOG
	 */
	nRC = MPGLOG_INIT( MODULE, NULL, LOG_MODE_DAILY | LOG_MODE_NO_DATE | LOG_MODE_LEVEL_TAG, LOG_LEVEL_DBG );
	if ( 0 > nRC )
	{
		printf( "%s MPGLOG_INIT() fail\n", __func__ );
		exit( EXIT_FAILURE );
	}

	/*
	 *	MPIPC 초기화:
	 *	설정파일에서 필요한 정보를 읽어 MPIPC 초기화에 사용한다.
	 *	ipc_check_interval - mpipc 스레드가 ipc message를 확인하는 주기 (100)
	 *	grace_time - mpipc 종료시 대기시간 (300)
	 */
	ptMpipc = mpipc_init( MODULE, 100, 300 );
	if ( NULL == ptMpipc )
	{
		MPGLOG_ERR( "%s:: mpipc_init() fail", __func__ );
		exit( EXIT_FAILURE );
	}

	/*
	 *	IPC Handler 등록:
	 *	프로세스에 IPC Message가 수신되었을때 호출되는 콜백함수를 등록한다.
	 */
	nRC = mpipc_regi_hdlr( ptMpipc, IPC_Handler, NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpipc_regi_hdlr() fail=%d", __func__, nRC );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	MPIPC 서비스 시작:
	 *	API 호출 후부터 IPC Message를 수신하여 Handler를 호출한다.
	 */
	nRC = mpipc_start( ptMpipc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpipc_start() fail=%d", __func__, nRC );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	MMC
	 */
	nRC = MMC_Init( MODULE, ptOammmc, ptMpipc );
	if ( SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: MMC_Init() fail=%d", __func__, nRC );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	dal DB 연결 및 Prepared Statement 초기화
	 */
	g_ptDalConn = dalConnect( NULL );
	if ( NULL == g_ptDalConn )
	{
		MPGLOG_ERR( "%s:: dalConnect() fail=%d", __func__, nRC );
		MMC_Destroy( ptOammmc );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	nRC = PSTMT_Init();	
	if ( SUCCESS != nRC )
	{
		MPGLOG_ERR( "%s:: PSTMT_Init() fail=%d", __func__, nRC );
		nRC = dalDisconnect( g_ptDalConn );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
		}
		MMC_Destroy( ptOammmc );
		IPC_Destroy( ptMpipc );
		exit( EXIT_FAILURE );
	}

	/*
	 *	Run
	 */
	while ( FLAG_RUN == g_nFlag )
	{
		mpthr_sleep_msec(500);
	}

	PSTMT_Destroy();
	
	nRC = dalDisconnect( g_ptDalConn );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDisconnect() fail=%d", __func__, dalErrno() );
	}
	
	MMC_Destroy( ptOammmc );
	IPC_Destroy( ptMpipc );

	return 0;
}

void SignalHandler( int nSig )
{
	g_nFlag = FLAG_STOP;

	MPGLOG_SVC( "Signal: %d\n", nSig );

	exit( -1 );
}

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

int MMC_Init( char *pszModule, oammmc_t *ptOammmc, mpipc_t *ptMpipc )
{
	if ( NULL == pszModule )
	{
		MPGLOG_ERR( "%s:: pszModule NULL", __func__ );
		return NULL_FAIL;
	}

	if ( NULL != ptOammmc )
	{
		ptOammmc = NULL;
	}

	if ( NULL == ptMpipc )
	{
		MPGLOG_ERR( "%s:: ptMpipc NULL", __func__ );
		return NULL_FAIL;
	}

	int nRC = 0;
	int nMmtEnable = 0;
	int nMmlEnable = 0;

	/*
	 *	oammmc 초기화
	 */	
	ptOammmc = oammmc_init( pszModule );
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: oammmc_init fail", __func__ );
		return OAMMMC_FAIL;
	}

	/*
	 *	oammmc bind mmt channel:
	 *	설정파일에 mmt_enable=1 일경우 mmt를 사용하도록 구성한다.
	 */	
	nRC = mpconf_get_int( "", PROCESS_INI, pszModule, MMT_ENABLE, &nMmtEnable, 1, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
		return MPCONF_FAIL;
	}

	if ( 1 == nMmtEnable )
	{
		int nMmtPort = 0;
		int nMmtConnMax = 0;
		int nMmtLocalOnly = 0;
		int nMmtIsQuiet = 0;
		char szBuf[MPCONF_STR_MAX];
		memset( szBuf, 0x00, sizeof(szBuf) );

		nRC = mpconf_get_int( NULL, PROCESS_INI, pszModule, MMT_PORT, &nMmtPort, 1, 0 );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_get_int( NULL, PROCESS_INI, pszModule, MMT_CONN_MAX, &nMmtConnMax, 1, 0 );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
			return MPCONF_FAIL;
		}

		nRC = mpconf_get_str( NULL, PROCESS_INI, pszModule, MMT_LOCAL_ONLY, szBuf, sizeof(szBuf), "True" );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str fail", __func__ );
			return MPCONF_FAIL;
		}

		if ( strcmp(szBuf, "True") == 0 )
		{
			nMmtLocalOnly = 1;
		}
		else if ( strcmp(szBuf, "False") == 0 )
		{
			nMmtLocalOnly = 0;
		}

		memset( szBuf, 0x00, sizeof(szBuf) );
		nRC = mpconf_get_str( NULL, PROCESS_INI, pszModule, MMT_IS_QUIET, szBuf, sizeof(szBuf), "False" );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: mpconf_get_str fail", __func__ );
			return MPCONF_FAIL;
		}

		if ( strcmp(szBuf, "True") == 0 )
		{
			nMmtIsQuiet = 1;
		}
		else if ( strcmp(szBuf, "False") == 0 )
		{
			nMmtIsQuiet = 0;
		}

		nRC = oammmc_bind_mmt( ptOammmc, pszModule, nMmtPort, nMmtConnMax, nMmtLocalOnly, nMmtIsQuiet );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_bind_mmt=%d", __func__, nRC );
			return OAMMMC_FAIL;
		}

		MPGLOG_SVC( "mmt_enabled. name=%s port=%d conn_max=%d local_only=%d is_quiet=%d",
				pszModule, nMmtPort, nMmtConnMax, nMmtLocalOnly, nMmtIsQuiet );
	}
	
	/*
	 *	oammmc bind mml channel:
	 *	설정파일에 mml_enable=1 일경우 mml을 사용하도록 구성한다.
	 */
	nRC = mpconf_get_int( "", PROCESS_INI, pszModule, MML_ENABLE, &nMmlEnable, 1, 0 );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpconf_get_int fail", __func__ );
		return MPCONF_FAIL;
	}

	if ( 1 == nMmlEnable )
	{
		nRC = oammmc_bind_mml( ptOammmc, mpipc_tap_ipc(ptMpipc), oammmc_name(ptOammmc) );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_bind_mml fail=%d", __func__, nRC );
			return OAMMMC_FAIL;
		}

		MPGLOG_SVC( "mml_enabled. name=%s", pszModule );
	}
	else if ( 0 == nMmlEnable )
	{
		return SYSTEM_FAIL;
	}

	/*
	 *	oammmc command table 등록:
	 *	위에 정의한 command table을 oammmc에 등록한다.
	 */	
	nRC = oammmc_regi_cmd( ptOammmc, tCommandTable, NULL );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_regi_cmd fail=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	/*
	 *	oammmc handler 등록:
	 *	oammmc handler를 mpipc에 등록하여 mmc가 처리되도록 한다.
	 */
	nRC = mpipc_regi_hdlr( ptMpipc, oammmc_mpipc_hdlr, ptOammmc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: mpipc_regi_hdlr fail=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	/*
	 *	oammmc 시작:
	 *	oammmc를 시작하여 mmt 및 mml이 처리되도록 한다.
	 */
	nRC = oammmc_start( ptOammmc );
	if ( 0 > nRC )
	{
		MPGLOG_ERR( "%s:: oammmc_start=%d", __func__, nRC );
		return OAMMMC_FAIL;
	}

	return SUCCESS;
}

void MMC_Destroy( oammmc_t *ptOammmc )
{
	int nRC = 0;
	
	if ( NULL != ptOammmc )
	{
		nRC = oammmc_stop( ptOammmc );
		if ( 0 > nRC )
		{
			MPGLOG_ERR( "%s:: oammmc_stop fail=%d", __func__, nRC );
		}
		oammmc_destroy( ptOammmc );
	}

	return;
}

int MMC_Handler_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptArgList )
	{
		MPGLOG_ERR( "%s:: ptArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;

	int i = 0;
	int nRC = 0;
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;
	oammmc_arg_t *ptArg = NULL;

	for ( i = 0; i < nArg; ++i )
	{
		ptArg = &ptArgList[i];

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				pszName = OAMMMC_VAL_STR( ptArg );
				
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_NAME, pszName );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
			case 2:
			{
				pszPosition = OAMMMC_VAL_STR( ptArg );
				
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_POSITION, pszPosition );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
			case 3:
			{
				pszTeam = OAMMMC_VAL_STR( ptArg );
			
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_TEAM, pszTeam );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
			case 4:
			{
				pszPhone = OAMMMC_VAL_STR( ptArg );
		
				nRC = dalSetStringByKey( g_ptPstmtInsert, TABLE_ATT_PHONE, pszPhone );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
			}
				break;
		}
	}

	nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtInsert, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	else if ( 0 == nRC )
	{
		oammmc_out( ptOammmc, "System Fail" );
		return SQL_EXEC_FAIL;
	}

	oammmc_out( ptOammmc, "[INSERT]\nName: %s\nPosition: %s\nTeam: %s\nPhone: %s\n",
				pszName, pszPosition, pszTeam, pszPhone );

	return MMC_SUCCESS;
};

int MMC_Handler_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptArgList )
	{
		MPGLOG_ERR( "%s:: ptArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;

	int nRC = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;
	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;

	if ( 0 == nArg ) //옵션을 입력하지 않았을때 모든 직원의 정보를 출력함
	{
		nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtSelectAll, &ptResult );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
			oammmc_out( ptOammmc, "System Fail" );	
			return DAL_FAIL;
		}
		else if ( 0 == nRC )
		{
			nRC = dalResFree( ptResult );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
				oammmc_out( ptOammmc, "System Fail" );
				return DAL_FAIL;
			}
			oammmc_out( ptOammmc, "No Infos in DB" );
			return SQL_EXEC_FAIL;
		}

		for ( ptEntry = dalFetchFirst(ptResult); ptEntry != NULL; ptEntry = dalFetchNext(ptResult) )
		{
			nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;			
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
				goto _exit_failure;
			}

			oammmc_out( ptOammmc, "[SELECT ALL]\nId: %d\nName: %s\nPosition: %s\nTeam: %s\nPhone: %s\n",
						nId, pszName, pszPosition, pszTeam, pszPhone );
		}

		return MMC_SUCCESS;
	}
	else if ( 1 == nArg ) //옵션을 하나만 입력했을때만 검색 가능하도록 설정함
	{
		oammmc_arg_t *ptArg = NULL;
		
		ptArg = &ptArgList[0];

		switch( OAMMMC_ARG_ID( ptArg ) )
		{
			case 1:
			{
				nId = OAMMMC_VAL_INT( ptArg );

				snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s, %s from %s where %s = %d;",
						TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
						TABLE_ATT_ID, nId );
				szQuery[ strlen(szQuery) ] = '\0';

				nRC = dalExecute( g_ptDalConn, szQuery, &ptResult );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptResult );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "[SELECT by Id %d] Not Exist", nId );
					return SQL_EXEC_FAIL;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return MMC_FAIL;
				}

				ptEntry = dalFetchFirst( ptResult );
				if ( NULL != ptEntry )
				{
					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "[SELECT by Id %d]\nName: %s\nPosition: %s\nTeam: %s\nPhone: %s\n",
								nId, pszName, pszPosition, pszTeam, pszPhone );
				}
			}
				break;
			case 2: //해당 이름을 가진 모든 직원의 정보를 출력함
			{
				pszName = OAMMMC_VAL_STR( ptArg );

				snprintf( szQuery, sizeof(szQuery),
						"select %s, %s, %s, %s from %s where %s = %s;",
						TABLE_ATT_ID, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
						TABLE_ATT_NAME, pszName );

				nRC = dalExecute( g_ptDalConn, szQuery, &ptResult );
				if ( 0 == nRC )
				{
					nRC = dalResFree( ptResult );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
						oammmc_out( ptOammmc, "System Fail" );
						return DAL_FAIL;
					}
					oammmc_out( ptOammmc, "Name[%s] not exist", pszName );
					return SQL_EXEC_FAIL;
				}
				else if ( -1 == nRC )
				{
					oammmc_out( ptOammmc, "System Fail" );
					return MMC_FAIL;
				}
				
				for ( ptEntry = dalFetchFirst(ptResult); ptEntry != NULL; ptEntry = dalFetchNext(ptResult) )
				{
					nRC = dalGetIntByKey( ptEntry, TABLE_ATT_ID, &nId );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetIntByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}	

					nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
					if ( -1 == nRC )
					{
						MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
						goto _exit_failure;
					}

					oammmc_out( ptOammmc, "[SELECT by Name %s]\nId: %d\nPosition: %s\nTeam: %s\nPhone: %s\n",
								pszName, nId, pszPosition, pszTeam, pszPhone );
				}
			}
				break;
			/*
			case 3: //해당 직책을 가진 모든 직원의 정보를 출력함
			{
					snprintf( szQuery, sizeof(szQuery),
					"select %s, %s, %s, %s from %s where %s = ?%s;",
					TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_TEAM, TABLE_ATT_PHONE,
					TABLE_NAME, TABLE_ATT_POSITION, TABLE_ATT_POSITION );

					oammmc_out( ptOammmc, "[SELECT by Position %s]\nId: %d\nName: %s\nTeam: %s\nPhone: %s\n",
								pszPosition, nId, pszName, pszTeam, pszPhone );

			}
				break;
			case 4: //해당 팀에 속한 모든 직원의 정보를 출력함
			{
					snprintf( szQuery, sizeof(szQuery),
					"select %s, %s, %s, %s from %s where %s = ?%s;",
					TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_PHONE,
					TABLE_NAME, TABLE_ATT_TEAM, TABLE_ATT_TEAM );

					oammmc_out( ptOammmc, "[SELECT by Team %s]\nId: %d\nName: %s\nPosition: %s\nPhone: %s\n",
								pszTeam, nId, pszName, pszPosition, pszPhone );

			}
				break;
			case 5: //해당 전화번호를 가진 직원의 정보를 출력함
			{
					snprintf( szQuery, sizeof(szQuery),
					"select %s, %s, %s, %s from %s where %s = ?%s;",
					TABLE_ATT_ID, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM,
					TABLE_NAME, TABLE_ATT_PHONE, TABLE_ATT_PHONE );
					
					oammmc_out( ptOammmc, "[SELECT by Phone %s]\nId: %d\nName: %s\nPosition: %s\nTeam: %s\n",
								pszPhone, nId, pszName, pszPosition, pszTeam );

			}
				break;*/
			default:
				break;
		}//switch
	}//else
	else
	{
		oammmc_out( ptOammmc, "Need to fill only 1 option\n" );
		return MMC_FAIL;
	}

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	return MMC_SUCCESS;

_exit_failure:
	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}
	oammmc_out( ptOammmc, "System Fail" );
	return DAL_FAIL;
};

int MMC_Handler_Chg( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptArgList )
	{
		MPGLOG_ERR( "%s:: ptArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;

	int i = 0;
	int nRC = 0;
	int nId = 0;

	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );
	
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;

	char *pszNewName = NULL;
	char *pszNewPosition = NULL;
	char *pszNewTeam = NULL;
	char *pszNewPhone = NULL;

	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;
	oammmc_arg_t *ptArg = NULL;

	//입력받은 것이 하나고, 그것이 1번칸이면 값이 하나도 안 들어온 상태임 -> 에러처리
	if ( 1 == nArg && 1 == OAMMMC_ARG_ID( &ptArgList[0] ) )
	{
		oammmc_out( ptOammmc, "Need input" );
		return MMC_FAIL;
	}

	nId = OAMMMC_VAL_INT( &ptArgList[0] );

	/*
	 *	기존 정보와 새로운 업데이트 정보 비교위해 확인
	 */
	snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s, %s from %s where %s = %d;",
			TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
			TABLE_ATT_ID, nId );
	szQuery[ strlen(szQuery) ] = '\0';

	nRC = dalExecute( g_ptDalConn, szQuery, &ptResult );
	if ( 0 == nRC )
	{
		nRC = dalResFree( ptResult );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
			oammmc_out( ptOammmc, "System Fail" );
			return DAL_FAIL;
		}
		oammmc_out( ptOammmc, "[UPDATE by Id %d] Not Exist", nId );
		return SQL_EXEC_FAIL;
	}
	else if ( -1 == nRC )
	{
		oammmc_out( ptOammmc, "System Fail" );
		return MMC_FAIL;
	}

	ptEntry = dalFetchFirst( ptResult );
	if ( NULL != ptEntry )
	{
		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}

		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}

		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}	

		nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
	}

	for ( i = 1; i < nArg; ++i )
	{
		ptArg = &ptArgList[i];

		switch ( OAMMMC_ARG_ID( ptArg ) )
		{
			case 2:
			{
				pszNewName = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 3:
			{
				pszNewPosition = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 4:
			{
				pszNewTeam = OAMMMC_VAL_STR( ptArg );
			}
				break;
			case 5:
			{
				pszNewPhone = OAMMMC_VAL_STR( ptArg );
			}
				break;
		}
	}

	if ( strcmp(pszName, pszNewName) == 0 && strcmp(pszPosition, pszNewPosition) == 0 &&
		 strcmp(pszTeam, pszNewTeam) == 0 && strcmp(pszPhone, pszNewPhone) == 0 )
	{
		oammmc_out( ptOammmc, "Old Data == New Data\n" );
		goto _exit_failure;
	}

	nRC = dalSetIntByKey( g_ptPstmtUpdate, TABLE_ATT_ID, nId );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );
		goto _exit_failure;
	}

	if ( NULL != pszNewName )
	{
		nRC = dalSetStringByKey( g_ptPstmtUpdate, TABLE_ATT_NAME, pszNewName );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
	}

	if ( NULL != pszNewPosition )
	{
		nRC = dalSetStringByKey( g_ptPstmtUpdate, TABLE_ATT_POSITION, pszNewPosition );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
	}

	if ( NULL != pszNewTeam )
	{
		nRC =  dalSetStringByKey( g_ptPstmtUpdate, TABLE_ATT_TEAM, pszNewTeam );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
	}

	if ( NULL != pszNewPhone )
	{
		nRC = dalSetStringByKey( g_ptPstmtUpdate, TABLE_ATT_PHONE, pszNewPhone );
		if ( -1 == nRC )
		{
			MPGLOG_ERR( "%s:: dalSetStringByKey() fail=%d", __func__, dalErrno() );
			goto _exit_failure;
		}
	}

	nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtUpdate, NULL );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
		goto _exit_failure;
	}
	else if ( 0 == nRC )
	{
		MPGLOG_DBG( "%s:: id not exist", __func__ );
		oammmc_out( ptOammmc, "[UPDATE by Id %d] Not Exist", nId );
		goto _exit_failure;
	}

	oammmc_out( ptOammmc, "[UPDATE by Id %d]\n"
						  "--Old Data--Name: %s\nPosition: %s\nTeam: %s\nPhone: %s\n"
						  "--New Data--Name: %s\nPosition: %s\nTeam: %s\nPhone: %s\n",
						  nId, pszName, pszPosition, pszTeam, pszPhone,
						  pszNewName, pszNewPosition, pszNewTeam, pszNewPhone );

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}
	return MMC_SUCCESS;

_exit_failure:
	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}
	oammmc_out( ptOammmc, "System Fail" );
	return DAL_FAIL;
};

int MMC_Handler_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd, oammmc_arg_t *ptArgList, int nArg, void *ptUarg )
{
	if ( NULL == ptOammmc )
	{
		MPGLOG_ERR( "%s:: ptOammmc NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptCmd )
	{
		MPGLOG_ERR( "%s:: ptCmd NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	if ( NULL == ptArgList )
	{
		MPGLOG_ERR( "%s:: ptArgList NULL", __func__ );
		oammmc_out( ptOammmc, "System Fail" );
		return NULL_FAIL;
	}

	ptUarg = ptUarg;

	int nRC = 0;
	int nId = 0;
	char *pszName = NULL;
	char *pszPosition = NULL;
	char *pszTeam = NULL;
	char *pszPhone = NULL;
	char szQuery[256];
	memset( szQuery, 0x00, sizeof(szQuery) );
	DAL_RESULT_SET *ptResult = NULL;
	DAL_ENTRY *ptEntry = NULL;
	oammmc_arg_t *ptArg = NULL;

	if ( 1 == nArg )
	{
		ptArg = &ptArgList[0];

		if ( 1 == OAMMMC_ARG_ID( ptArg ) )
		{
			nId = OAMMMC_VAL_INT( ptArg );

			snprintf( szQuery, sizeof(szQuery), "select %s, %s, %s, %s from %s where %s = %d;",
					TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE, TABLE_NAME,
					TABLE_ATT_ID, nId );
			szQuery[ strlen(szQuery) ] = '\0';

			nRC = dalExecute( g_ptDalConn, szQuery, &ptResult );
			if ( 0 == nRC )
			{
				nRC = dalResFree( ptResult );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
					oammmc_out( ptOammmc, "System Fail" );
					return DAL_FAIL;
				}
				oammmc_out( ptOammmc, "Id[%d] not exist", nId );
				return SQL_EXEC_FAIL;
			}
			else if ( -1 == nRC )
			{
				oammmc_out( ptOammmc, "System Fail" );
				return MMC_FAIL;
			}

			ptEntry = dalFetchFirst( ptResult );
			if ( NULL != ptEntry )
			{
				nRC = dalGetStringByKey( ptEntry, TABLE_ATT_NAME, &pszName );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
					goto _exit_failure;
				}

				nRC = dalGetStringByKey( ptEntry, TABLE_ATT_POSITION, &pszPosition );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
					goto _exit_failure;
				}

				nRC = dalGetStringByKey( ptEntry, TABLE_ATT_TEAM, &pszTeam );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
					goto _exit_failure;
				}	

				nRC = dalGetStringByKey( ptEntry, TABLE_ATT_PHONE, &pszPhone );
				if ( -1 == nRC )
				{
					MPGLOG_ERR( "%s:: dalGetStringByKey() fail=%d", __func__, dalErrno() );
					goto _exit_failure;
				}
			}	

			nRC = dalSetIntByKey( g_ptPstmtDelete, TABLE_ATT_ID, nId );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalSetIntByKey() fail=%d", __func__, dalErrno() );		
				return DAL_FAIL;
			}

			nRC = dalPreparedExec( g_ptDalConn, g_ptPstmtDelete, NULL );
			if ( -1 == nRC )
			{
				MPGLOG_ERR( "%s:: dalPreparedExec() fail=%d", __func__, dalErrno() );
				return DAL_FAIL;
			}
			else if ( 0 == nRC )
			{
				MPGLOG_DBG( "%s:: id not exist", __func__ );
				oammmc_out( ptOammmc, "Id[%d] not exist", nId );
				return SQL_EXEC_FAIL;
			}

			oammmc_out( ptOammmc, "[DELETE by Id %d]\nName: %s\nPosition: %s\nTeam: %s\nPhone: %s\n",
					nId, pszName, pszPosition, pszTeam, pszPhone );
		}
	}

	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
		oammmc_out( ptOammmc, "System Fail" );
		return DAL_FAIL;
	}

	return MMC_SUCCESS;

_exit_failure:
	nRC = dalResFree( ptResult );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalResFree() fail=%d", __func__, dalErrno() );
	}
	oammmc_out( ptOammmc, "System Fail" );
	
	return DAL_FAIL;
};

int PSTMT_Init()
{
	char szQuery[256];

	//INSERT
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"insert into %s (%s, %s, %s, %s) values (?%s, ?%s, ?%s, ?%s);",
			TABLE_NAME, TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE,
			TABLE_ATT_NAME, TABLE_ATT_POSITION, TABLE_ATT_TEAM, TABLE_ATT_PHONE );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtInsert = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtInsert )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//SELECT ALL
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery), "select * from %s", TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtSelectAll = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtSelectAll )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//UPDATE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"update %s set %s = ?%s, %s = ?%s, %s = ?%s where %s = ?%s;",
			TABLE_NAME,
			TABLE_ATT_POSITION, TABLE_ATT_POSITION,
			TABLE_ATT_TEAM, TABLE_ATT_TEAM,
			TABLE_ATT_PHONE, TABLE_ATT_PHONE,
			TABLE_ATT_ID, TABLE_ATT_ID );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtUpdate = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtUpdate )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//DELETE
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"delete from %s where %s = ?%s;",
			TABLE_NAME, TABLE_ATT_ID, TABLE_ATT_ID );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtDelete = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtDelete )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	//NUMTUPLES
	memset( szQuery, 0x00, sizeof(szQuery) );
	snprintf( szQuery, sizeof(szQuery),
			"select %s from __SYS_TABLES__ where TABLE_NAME='%s'",
			NUMTUPLES, TABLE_NAME );
	szQuery[ strlen(szQuery) ] = '\0';
	g_ptPstmtNumTuples = dalPreparedStatement( g_ptDalConn, szQuery );
	if ( NULL == g_ptPstmtNumTuples )
	{
		MPGLOG_ERR( "%s:: dalPreparedStatement() fail=%d", __func__, dalErrno() );
		return DAL_FAIL;
	}

	return SUCCESS;
}

void PSTMT_Destroy()
{
	int nRC = 0;

	nRC = dalDestroyPreparedStmt( g_ptPstmtInsert );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtSelectAll );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}
	
	nRC = dalDestroyPreparedStmt( g_ptPstmtUpdate );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtDelete );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	nRC = dalDestroyPreparedStmt( g_ptPstmtNumTuples );
	if ( -1 == nRC )
	{
		MPGLOG_ERR( "%s:: dalDestroyPreparedStmt() fail=%d", __func__, dalErrno() );
	}

	return;
}
