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

#endif /* _RAS_MACRO_H_ */
