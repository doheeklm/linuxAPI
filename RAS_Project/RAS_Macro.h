/* RAS_Macro.h */
#ifndef _RAS_MACRO_H_
#define _RAS_MACRO_H_

#define RAS_TRUE					0
#define RAS_FALSE					1

#define F_FORMAT( format )			"%s:: "format"\n", __func__
#define LOG_ERR_F( format, ... )	LOG_ERR( F_FORMAT(format), ##__VA_ARGS__ );
#define LOG_DBG_F( format, ... )	LOG_DBG( F_FORMAT(format), ##__VA_ARGS__ );
#define LOG_SVC_F( format, ... )	LOG_SVC( F_FORMAT(format), ##__VA_ARGS__ );

#define STR_ARG(_v_)				#_v_
#define STR_ENUM_CASE(_v_)			case (_v_): return #_v_
#define STR_ENUM_DFLT				default: return #_v_
#define STR_CASE( _v_, _s_)			case (_v_): return _s_
#define STR_CASE_NONE( _v_)			case (_v_): return "NONE"
#define STR_CASE_DFLT_UKN			default: return "UNKNOWN"

#define CHECK_PARAM( _expr_t, _error ) \
	do { \
		if ( !(_expr_t) ) \
		{ \
			LOG_ERR_F( "Invalid parameter <(%s)=false>", #_expr_t ); \
			_error; \
		} \
	} while (0)

#define CHECK_PARAM_RC( _expr_t ) CHECK_PARAM( (_expr_t), return RAS_rErrInvalidParam )

#define STRLCAT_CHECK_OVERFLOW( _dst, _src, _dstsize, _rc ) \
	do { \
		if ( strlcat( _dst, _src, _dstsize ) >= _dstsize ) \
		{ \
			LOG_ERR_F( "strlcat overflow error" ); \
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

#define PRT_IP_ALL_HEADER( p_mmc, _arg_ip, _arg_optional ) \
	do { \
		PRT_LINE( p_mmc ); \
		oammmc_out( (p_mmc), "%-16s %-33s\n", _arg_ip, _arg_optional ); \
		PRT_LINE( p_mmc ); \
	} while (0)

#define PRT_IP_ALL_BODY( p_mmc, _ip, _optional ) \
	do { \
		oammmc_out( (p_mmc), "%-16s %-33s\n", _ip, _optional ); \
	} while (0)

//for _arg_optional,
//ip-ARG_DESC_DESC, trc-ARG_DESC_TIME
#define PRT_IP_ONE( p_mmc, _arg_ip, _ip, _arg_optional, _optional ) \
	do { \
		PRT_LINE( p_mmc ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_ip, _ip ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _arg_optional, _optional ); \
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

#define REGI_GET_KEY_AND_VALUE( _key, _keylen, _buf, _bufsize, _rc ); \
	do { \
		_rc = TAP_Registry_udp_enum_key_value( _key, _keylen, _buf, _bufsize, REGI_SYS_ID ); \
		if ( 0 > _rc ) \
		{ \
			LOG_ERR_F( "TAP_Registry_udp_enum_key_value (%s) <%d:%s>", \
					_key, _rc, TAP_REG_GET_ERROR_CONTENT(_rc) ); \
			_rc = RAS_rErrRegiGetKeyAndValue; \
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

#endif /* _RAS_MACRO_H_ */
