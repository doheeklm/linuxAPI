/* RAS_Macro.h */
#ifndef _RAS_MACRO_H_
#define _RAS_MACRO_H_

#define RAS_TRUE					0
#define RAS_FALSE					1

#define F_FORMAT( format )			"%s:: "format"\n", __func__
#define LOG_ERR_F( format, ... )	LOG_ERR( F_FORMAT(format), ##__VA_ARGS__ );
#define LOG_DBG_F( format, ... )	LOG_DBG( F_FORMAT(format), ##__VA_ARGS__ );
#define LOG_SVC_F( format, ... )	LOG_SVC( F_FORMAT(format), ##__VA_ARGS__ );

#define CASE_RETURN( _c, _ret )		case ( _c ): return _ret
#define CASE_DEFAULT_UNKNOWN		default: return "UNKNOWN" 
#define CASE_DEFAULT_500			default: return STATUS_CODE_500

#define LINE						"=============================="

#define CHECK_PARAM( _expr_t, _error ) \
	do { \
		if ( !(_expr_t) ) \
		{ \
			LOG_ERR_F( "Invalid parameter <(%s)=false>", #_expr_t ); \
			_error; \
		} \
	} while (0)

#define CHECK_PARAM_RC( _expr_t ) \
	CHECK_PARAM( (_expr_t), return RAS_rErrInvalidParam )

#define STRLCAT_OVERFLOW_CHECK( _dst, _src, _dstsize, _rc ) \
	do { \
		if ( strlcat( _dst, _src, _dstsize ) >= _dstsize ) \
		{ \
			LOG_ERR_F( "strlcat overflow fail" ); \
			_rc = RAS_rErrOverflow; \
			goto end_of_function; \
		} \
	} while (0)

#define SNPRINTF_QUERY_INT( _buf, _bufsize, _attr, _val ) \
	do { \
		snprintf( _buf, _bufsize, "%s = %d", _attr, _val ); \
		_buf[ strlen(_buf) ] = '\0'; \
	} while (0)

#define SNPRINTF_QUERY_STR( _buf, _bufsize, _attr, _val ) \
	do { \
		snprintf( _buf, _bufsize, "%s = '%s'", _attr, _val ); \
		_buf[ strlen(_buf) ] = '\0'; \
	} while (0)

#define VALID_RANGE( _val, _min, _max ) \
	( ((int)(_val) >= (int)(_min)) && \
	  ((int)(_val) <= (int)(_max)) \
	  ? TRUE : FALSE )

#define PRT_LINE( p_mmc ) \
	do { \
		oammmc_out( (p_mmc), "---------------------------------\n" ); \
	} while (0)

#define PRT_LINEx3( p_mmc ) \
	do { \
		oammmc_out( (p_mmc), "---------------------------------" \
 							 "---------------------------------" \
							 "---------------------------------\n" ); \
	} while (0)

#define PRT_CNT( p_mmc, _cnt ) \
	do { \
		oammmc_out( (p_mmc), "Total Count = %d", _cnt ); \
	} while (0)

#define PRT_IP_ALL_HEADER( p_mmc, _arg_ip, _arg_opt ) \
	do { \
		PRT_LINE( p_mmc ); \
		oammmc_out( (p_mmc), "%-16s %-10s\n", _arg_ip, _arg_opt ); \
		PRT_LINE( p_mmc ); \
	} while (0)

/*
 *	For _opt,
 *	IP  - DESC char(32)
 *	TRC - PERIOD_TM char(5)
 */
#define PRT_IP_ALL_BODY( p_mmc, _ip, _opt ) \
	do { \
		oammmc_out( (p_mmc), "%-16s %-33s\n", _ip, _opt ); \
	} while (0)

#define PRT_IP_ONE( p_mmc, _arg_ip, _ip, _arg_opt, _opt ) \
	do { \
		PRT_LINE( p_mmc ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_ip, _ip ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_opt, _opt ); \
		PRT_LINE( p_mmc ); \
	} while (0)

#define PRT_INFO_ALL_HEADER( p_mmc, _arg_id, _arg_name, _arg_gender, _arg_birth, _arg_address ) \
	do { \
		PRT_LINEx3( p_mmc ); \
		oammmc_out( (p_mmc), "%-11s %-33s %-7s %-7s %-20s\n", \
			   _arg_id, _arg_name, _arg_gender, _arg_birth, _arg_address ); \
		PRT_LINEx3( p_mmc ); \
	} while (0)

#define PRT_INFO_ALL_BODY( p_mmc, _id, _name, _gender, _birth, _address ) \
	do { \
		oammmc_out( (p_mmc), "%-11d %-33s %-7s %-7s %-20s\n", \
			   _id, _name, _gender, _birth, _address ); \
	} while (0)

#define PRT_INFO_ONE( p_mmc, _arg_id, _id, _arg_name, _name, _arg_gender, _gender, \
		_arg_birth, _birth, _arg_address, _address ) \
	do { \
		PRT_LINE( p_mmc ); \
		oammmc_out( (p_mmc), "%-12s = %d\n", _arg_id, _id ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_name, _name ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_gender, _gender ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_birth, _birth ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_address, _address ); \
		PRT_LINE( p_mmc ); \
	} while (0)

#define PRT_NOT_FOUND( p_mmc ) \
	do { \
		oammmc_out( (p_mmc), "NOT FOUND" ); \
	} while (0)

#define PRT_FAIL( p_mmc, format, ... ) \
	do { \
		if ( p_mmc ) \
		{ \
			oammmc_clear( (p_mmc) ); \
			oammmc_out( (p_mmc), format, ##__VA_ARGS__ ); \
		} \
	} while (0)

/*
 *	DB
 */
#define DB_INIT_PSTMT( p_db, _index, _sql ) \
	do { \
		(p_db)->patPstmt[(_index)] = dalPreparedStatement( (p_db)->ptDBConn, (_sql) ); \
		if ( NULL == (p_db)->patPstmt[(_index)] ) \
		{ \
			LOG_ERR_F( "dalPreparedStatement fail <%d:%s> (%d:%s)", \
					dalErrno(), dalErrmsg(dalErrno()), (_index), (_sql) ); \
			return RAS_rErrDBPstmt; \
		} \
	} while (0)

#define DB_DESTROY_PSTMT( p_db, _index ) \
	do { \
		if ( NULL != (p_db)->patPstmt[(_index)] ) \
		{ \
			if ( 0 > dalDestroyPreparedStmt( (p_db)->patPstmt[(_index)] ) ) \
			{ \
				LOG_ERR_F( "dalDestroyPreparedStatement fail <%d:%s>", \
						dalErrno(),dalErrmsg(dalErrno()) ); \
				return; \
			} \
			(p_db)->patPstmt[(_index)] = NULL; \
		} \
	} while (0)

#define DB_FREE( p_res ) \
	do { \
		if ( p_res ) \
		{ \
			dalResFree( p_res ); \
			p_res = NULL; \
		} \
	} while (0)

#define DB_SET_INT_BY_KEY( p_pstmt, _key, _value, _rc ) \
	do { \
		if ( -1 == dalSetIntByKey( p_pstmt, _key, _value ) ) \
		{ \
			LOG_ERR_F( "dalSetIntByKey (%s, %d) fail <%d:%s>", \
					_key, _value, dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBSetValue; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_SET_STRING_BY_KEY( p_pstmt, _key, _value, _rc ) \
	do { \
		if ( -1 == dalSetStringByKey( p_pstmt, _key, _value ) ) \
		{ \
			LOG_ERR_F( "dalSetStringByKey (%s, %s) fail <%d:%s>", \
					_key, _value, dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBSetValue; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_GET_INT_BY_KEY( p_entry, _key, p_value, _rc ) \
	do { \
		if ( -1 == dalGetIntByKey( p_entry, _key, p_value ) ) \
		{ \
			LOG_ERR_F( "dalGetIntByKey (%s) fail <%d:%s>", \
					_key, dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBGetValue; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_GET_STRING_BY_KEY( p_entry, _key, p_value, _rc ) \
	do { \
		if ( -1 == dalGetStringByKey( p_entry, _key, p_value ) ) \
		{ \
			LOG_ERR_F( "dalGetStringByKey (%s) fail <%d:%s>", \
					_key, dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBGetValue; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_EXECUTE( _db, _query, pp_res, _rc ) \
	do { \
		_rc = dalExecute( _db.ptDBConn, _query, pp_res ); \
		if ( -1 == _rc ) \
		{ \
			LOG_ERR_F( "dalExecute (%s) fail <%d:%s>", \
					_query, dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBExecute; \
			goto end_of_function; \
		} \
		else if ( 0 == _rc ) \
		{ \
			LOG_ERR_F( "dalExecute zero" ); \
			_rc = RAS_rErrDBNotFound; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_PREPARED_EXEC( _db, p_pstmt, pp_res, _rc ) \
	do { \
		_rc = dalPreparedExec( _db.ptDBConn, p_pstmt, pp_res ); \
		if ( -1 == _rc ) \
		{ \
			LOG_ERR_F( "dalPreparedExec fail <%d:%s>", \
					dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBExecute; \
			goto end_of_function; \
		} \
		else if ( 0 == _rc ) \
		{ \
			LOG_ERR_F( "dalPreparedExec zero" ); \
			_rc = RAS_rErrDBNotFound; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_PREPARED_EXEC_UPDATE( _db, p_pstmt, _rc ) \
	do { \
		_rc = dalPreparedExecUpdate( _db.ptDBConn, p_pstmt ); \
		if ( -1 == _rc ) \
		{ \
			LOG_ERR_F( "dalPreparedExecUpdate fail <%d:%s>", \
					dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBExecUpdate; \
			goto end_of_function; \
		} \
		else if ( 0 == _rc ) \
		{ \
			LOG_ERR_F( "dalPreparedExecUpdate zero" ); \
			_rc = RAS_rErrDBNotFound; \
			goto end_of_function; \
		} \
	} while (0)

#define DB_ROLLBACK( _db, _rc ) \
	do { \
		_rc = dalRollback( _db.ptDBConn ); \
		if ( -1 == _rc ) \
		{ \
			LOG_ERR_F( "dalRollback fail <%d:%s>", \
					dalErrno(), dalErrmsg(dalErrno()) ); \
			_rc = RAS_rErrDBRollback; \
			goto end_of_function; \
		} \
	} while (0)

/*
 *	REGI
 */
#define REGI_STR_KEY( _buf, _bufsize, _key ); \
	do { \
		snprintf( _buf, _bufsize, "%s/%s", REGI_DIR, _key ); \
		_buf[ strlen(_buf) ] = '\0'; \
	} while (0)

#define REGI_STR_VALUE( _buf, _bufsize, _val ); \
	do { \
		snprintf( _buf, _bufsize, "%d", _val ); \
		_buf[ strlen(_buf) ] = '\0'; \
	} while (0)

#define REGI_CREATE( _key, _keylen, _rc ); \
	do { \
		_rc = TAP_Registry_udp_key_create( _key, _keylen, TAP_REGISTRY_FILE, REGI_SYS_ID ); \
		if ( TAP_REGI_ALREADY_EXIST == _rc ) \
		{ \
			LOG_SVC_F( "TAP_REGI_ALREADY_EXIST (%s)", _key ); \
			_rc = RAS_rErrRegiKeyExist; \
			goto end_of_function; \
		} \
		else if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "TAP_Registry_udp_key_create (%s) fail <%d:%s>", \
					_key, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
			_rc = RAS_rErrRegiCreate; \
			goto end_of_function; \
		} \
	} while (0)

#define REGI_SET_VALUE( _key, _keylen, _val, _valsize, _rc ); \
	do { \
		_rc = TAP_Registry_udp_set_value( _key, _keylen, _val, _valsize, REGI_SYS_ID );\
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "TAP_Registry_udp_set_value (%s: %s) fail <%d:%s>", \
					_key, _val, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
			_rc = RAS_rErrRegiSetValue; \
			goto end_of_function; \
		} \
	} while (0)

#define REGI_GET_VALUE( _key, _keylen, _val, _valsize, _rc ); \
	do { \
		_rc = TAP_Registry_udp_query_value( _key, _keylen, _val, _valsize, REGI_SYS_ID );\
		if ( 0 > _rc ) \
		{ \
			if ( TAP_REGI_DATA_NOT_FOUND == _rc ) \
			{ \
				_rc = RAS_rErrRegiKeyExist; \
			} \
			else if ( TAP_REGI_NOT_FOUND == _rc ) \
			{ \
				LOG_ERR_F( "TAP_Registry_udp_query_value (%s: %s) fail <%d:%s>", \
						_key, _val, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
				_rc = RAS_rErrRegiNotFound; \
				goto end_of_function; \
			} \
			else \
			{ \
				LOG_ERR_F( "TAP_Registry_udp_query_value (%s: %s) fail <%d:%s>", \
						_key, _val, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
				_rc = RAS_rErrRegiGetValue; \
				goto end_of_function; \
			} \
		} \
	} while (0)

#define REGI_GET_ENUM_KEY_VALUE( _key, _keylen, _buf, _bufsize, _rc ); \
	do { \
		_rc = TAP_Registry_udp_enum_key_value( _key, _keylen, _buf, _bufsize, REGI_SYS_ID ); \
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "TAP_Registry_udp_enum_key_value (%s) <%d:%s>", \
					_key, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
			_rc = RAS_rErrRegiGetEnumKeyValue; \
			goto end_of_function; \
		} \
	} while (0)

#define REGI_DELETE( _key, _keylen, _rc ); \
	do { \
		_rc = TAP_Registry_udp_key_delete( _key, _keylen, TAP_REGISTRY_UDP_REMOVEALL, REGI_SYS_ID ); \
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "TAP_Registry_udp_key_delete (%s) fail <%d:%s>", \
					_key, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
			_rc = RAS_rErrRegiDelete; \
			goto end_of_function; \
		} \
	} while (0)

/*
 *	FD
 */
#define FD_CLOSE( _fd ); \
	do { \
		if ( -1 == close( _fd ) ) \
		{ \
			LOG_ERR_F( "close (fd %d) fail <%d:%s>", _fd, errno, strerror(errno) ); \
		} \
	} while (0)

#define FD_DELETE_AND_CLOSE( _fd, _epoll ); \
	do { \
		if ( -1 == epoll_ctl( _epoll, EPOLL_CTL_DEL, _fd, NULL ) ) \
		{ \
			LOG_ERR_F( "epoll_ctl (delete fd %d from epoll %d) fail <%d:%s>", _fd, _epoll, errno, strerror(errno) ); \
		} \
		FD_CLOSE( _fd ); \
	} while (0)

/*
 *	HTTP
 */
#define HTTP_SET_RESPONSE( _response ) \
	do { \
		snprintf( _response.szMsg, sizeof(_response.szMsg), \
				"HTTP/1.1 %d %s\r\n" \
				"Content-Type: application/json\r\n" \
				"Content-Length: %d\r\n\r\n%s", \
				_response.nStatusCode, HTTP_GetStatusMsg(_response.nStatusCode ), \
				_response.nContentLength, _response.szBody ); \
		_response.szMsg[ strlen(_response.szMsg) ] = '\0'; \
	} while (0)

#define HTTP_JSON_INFO_BEGIN	"[\n"
#define HTTP_JSON_INFO_AND		",\n"
#define HTTP_JSON_INFO_END		"\n]"

#define HTTP_JSON_INFO( _buf, _bufsize, _attr_id, _id, _attr_name, _name, \
		_attr_gender, _gender, _attr_birth, _birth, _attr_address, _address ) \
	do { \
		snprintf( _buf, _bufsize, \
				"{\n    " \
				"\"%s\": %d,\n    " \
				"\"%s\": \"%s\",\n    " \
				"\"%s\": \"%s\",\n    " \
				"\"%s\": \"%s\",\n    " \
				"\"%s\": \"%s\"\n}", \
				_attr_id, _id, _attr_name, _name, _attr_gender, _gender, \
				_attr_birth, _birth, _attr_address, _address ); \
		_buf[ strlen(_buf) ] = '\0'; \
	} while (0)

#define HTTP_STR_TO_NUM_METHOD( _method, _num ) \
	do { \
		if ( 0 == strcmp( HTTP_METHOD_POST_STR, _method ) ) \
		{ \
			_num = HTTP_METHOD_POST_NUM; \
		} \
		else if ( 0 == strcmp( HTTP_METHOD_GET_STR, _method ) ) \
		{ \
			_num = HTTP_METHOD_GET_NUM; \
		} \
		else if ( 0 == strcmp( HTTP_METHOD_DEL_STR, _method ) ) \
		{ \
			_num = HTTP_METHOD_DEL_NUM; \
		} \
	} while (0)

/*
 *	WORKER
 */
#define THREAD_CANCEL( _thread_id ) \
	do { \
		if ( 0 != pthread_cancel( _thread_id ) ) \
		{ \
			LOG_ERR_F( "pthread_cancel fail" ); \
		} \
	} while (0)

/*
 *	Alarm
 */
#define ALARM_CREATE( p_ipc, _upp, _low, _item, _status, _buf, _rc ) \
	do { \
		_rc = oam_uda_crte_alarm( p_ipc, _upp, _low, _item, _status, OAM_SFM_UDA_NOTI_OFF, _buf ); \
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "oam_uda_crte_alarm fail <%d>", _rc ); \
			_rc = RAS_rErrFail; \
			goto end_of_function; \
		} \
	} while (0)

#define ALARM_CREATE_NOTI( p_ipc, _upp, _low, _rc ) \
	do { \
		_rc = oam_uda_crte_alarm_noti( p_ipc, _upp, _low ); \
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "oam_uda_crte_alarm_noti fail <%d>", _rc ); \
			_rc = RAS_rErrFail; \
			goto end_of_function; \
		} \
	} while (0)

#define ALARM_SET_MODULE_INFO( _buf, _bufsize, _upp, _low, _item, _status, _cnt ) \
	do { \
		snprintf( _buf, _bufsize, \
				"UNIT NAME	: %s/%s\n" \
				"ITEM NAME	: %s\n" \
				"STATUS		: %d\n" \
				"CNT USER	: %d", \
				_upp, _low, _item, _status, _cnt ); \
		_buf[ strlen(_buf) ] = '\0'; \
	} while (0)

#define ALARM_REPORT_STATUS( p_ipc, _upp, _low, _item, _status, _buf, _rc ) \
	do { \
		_rc = oam_uda_rpt_alarm_sts( p_ipc, _upp, _low, _item, _status, _buf ); \
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "oam_uda_rpt_alarm_sts fail <%d>", _rc ); \
			_rc = RAS_rErrFail; \
			goto end_of_function; \
		} \
	} while (0)

#endif /* _RAS_MACRO_H_ */
