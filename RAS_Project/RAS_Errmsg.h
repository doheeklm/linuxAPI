/* RAS_Errmsg.h */
#ifndef _RAS_ERRMSG_H_
#define _RAS_ERRMSG_H_

typedef enum
{
	RAS_rOK = 1000,
	RAS_rErrFail,
	RAS_rErrGetValue,
	RAS_rErrGetConfValue,
	RAS_rErrSetValue, 
	RAS_rErrOutOfRange,
	RAS_rErrConfInit,
	RAS_rErrLogInit,
	RAS_rErrMmcInit,
	RAS_rErrRegiInit,
	RAS_rErrIpcInit,
	RAS_rErrAlarmInit,
	RAS_rErrStatInit,
	RAS_rErrThreadInit,
	RAS_rErrSocketInit,
	RAS_rErrEpollInit,
	RAS_rErrEventRecreateNo,
	RAS_rErrEventRecreateYes,	
	RAS_rErrIpcGetKey,
	RAS_rErrInvalidParam,
	
	RAS_rErrIpcRecv,
	RAS_rErrIpcSend,
	RAS_rErrInvalidData,
	RAS_rErrInvalidValue,

	RAS_rErrDBConn,
	RAS_rErrDBCommit,
	RAS_rErrDBPstmt,
	RAS_rErrDBFetch,
	RAS_rErrDBGetValue,
	RAS_rErrDBSetValue,
	RAS_rErrDBNotFound,
	RAS_rErrDBAttrNull,
	RAS_rErrDBExecute,
	RAS_rErrDBInsert,
	RAS_rErrDBSelect,
	RAS_rErrDBDelete,
	RAS_rErrDBFail,
	
	RAS_rErrThreadCreate,
	RAS_rErrStatOpen,

	RAS_rErrRegiUdpOpen,
	RAS_rErrRegiGetKeyList,
	RAS_rErrInvalidRegiKeyList,
	RAS_rErrRegiSetValue,
	RAS_rErrRegiGetValue,
	RAS_rErrRegiCreate,

	RAS_rErrJsonCheckSize,
	RAS_rErrJsonGetArr,
	RAS_rErrJsonMakeArr,
	RAS_rErrJsonMakeStr,
	RAS_rErrJsonAppendVal,
	RAS_rErrJsonInvalidType,

	RAS_rSuccessMmchdl = 0
} ReturnCode_e;

#endif /* _RAS_ERRMSG_H_ */
