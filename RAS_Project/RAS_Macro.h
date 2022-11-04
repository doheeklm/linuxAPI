/* RAS_Macro.h */
#ifndef _RAS_MACRO_H_
#define _RAS_MACRO_H_

#define TRUE	1
#define FALSE	0

#define F_FORMAT( format )			"%s:: "format"\n", __func__
#define LOG_ERR_F( format, ... )	LOG_ERR( F_FORMAT(format), ##__VA_ARGS__ );
#define LOG_DBG_F( format, ... )	LOG_DBG( F_FORMAT(format), ##__VA_ARGS__ );
#define LOG_SVC_F( format, ... )	LOG_SVC( F_FORMAT(format), ##__VA_ARGS__ );

#define CHECK_PARAM( _expr_t, _error ) \
	do { \
		if ( !(_expr_t) ) \
		{ \
			LOG_ERR_F( "Invalid parameter <(%s)=false>", #_expr_t ); \
			_error; \
		} \
	} while (0)

#define CHECK_PARAM_RC( _expr_t ) CHECK_PARAM( (_expr_t), return RAS_rErrInvalidParam )

#define VALID_RANGE( _val, _min, _max ) \
	( ((int)(_val) >= (int)(_min)) && \
	  ((int)(_val) <= (int)(_max)) \
	  ? TRUE : FALSE )

#define MMC_PRT_CRLF( p_mmc ) \
	do { \
		oammmc_out( (p_mmc), "\n" );\
	} while (0)

#define MMC_PRT_LINE( p_mmc ) \
	do { \
		oammmc_out( (p_mmc), "====================\n" ); \
	} while (0)

#define MMC_PRT_CLI_IP_ONE( p_mmc, _attr_ip, _ip, _attr_desc, _desc ) \
	do { \
		oammmc_out( (p_mmc), "==============================\n" ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _attr_ip, _ip ); \
		oammmc_out( (p_mmc), "%-12s = %s\n", _attr_desc, _desc ); \
		oammmc_out( (p_mmc), "==============================\n" ); \
	} while (0)

#define MMC_PRT_NOT_FOUND( p_mmc ) \
	do { \
		oammmc_out( (p_mmc), "not found" ); \
	} while (0)

#endif /* _RAS_MACRO_H_ */
