/* RAS_Errmsg.h */
#ifndef _RAS_ERRMSG_H_
#define _RAS_ERRMSG_H_

typedef enum
{
	RAS_rOK = 1000,
	RAS_rErrFail,
	RAS_rErrInvalidParam,
	RAS_rErrGetValue

#if 0 
	RAS_rErrSigTerm
	RAS_rErrDBConn
	RAS_rErrDBCommit
	RAS_rErrDBPstmt
	RAS_rErrDBFetch
	RAS_rErrDBGetValue
	RAS_rErrDBSetValue
	RAS_rErrDBNotFound
	RAS_rErrDBAttrNull
	RAS_rErrDBExecute
	RAS_rErrThreadCreate
	RAS_rErrStatOpen
	RAS_rErrJsonInvalidType
#endif
} ReturnCode_e;

#endif /* _RAS_ERRMSG_H_ */
