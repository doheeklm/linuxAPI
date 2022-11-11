/* RAS_Err.h */
#ifndef _RAS_ERR_H_
#define _RAS_ERR_H_

#define MMC_FAIL_REASON_SYSTEM_FAIL		"SYSTEM FAIL"
#define MMC_FAIL_REASON_INVALID_PARAM	"INVALID PARAMETER"
#define MMC_FAIL_REASON_DB_FAIL			"DB FAIL"
#define MMC_FAIL_REASON_DB_NOT_FOUND	"DB NOT FOUND"
#define MMC_FAIL_REASON_REGI_FAIL		"REGI FAIL"
#define MMC_FAIL_REASON_REGI_KEY_EXIST	"REGI KEY EXIST"
#define MMC_FAIL_REASON_REGI_NOT_FOUND	"REGI NOT FOUND"

typedef enum
{
	RAS_rOK = 1000,
	RAS_rErrFail,
	RAS_rErrGetValue,
	RAS_rErrGetConfValue,
	RAS_rErrSetValue, 
	RAS_rErrOverflow,//
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
	RAS_rErrInvalidParam,//

	RAS_rErrIpcRecv,
	RAS_rErrIpcSend,
	RAS_rErrInvalidData,
	RAS_rErrInvalidValue,

	RAS_rErrDBConn,
	RAS_rErrDBCommit,
	RAS_rErrDBExecute,//
	RAS_rErrDBExecUpdate,//
	RAS_rErrDBNotFound,//
	RAS_rErrDBPstmt,
	RAS_rErrDBSetValue,//

	RAS_rErrDBFail,
	RAS_rErrDBFetch,
	RAS_rErrDBGetValue,
	RAS_rErrDBAttrNull,
	RAS_rErrDBInsert,
	RAS_rErrDBSelect,
	RAS_rErrDBDelete,
	
	RAS_rErrThreadCreate,
	RAS_rErrStatOpen,

	RAS_rErrRegiCreate,//
	RAS_rErrRegiKeyExist,//
	RAS_rErrRegiSetValue,//
	RAS_rErrRegiGetKeyAndValue,//
	RAS_rErrRegiGetValue,//
	RAS_rErrRegiDelete,//
	RAS_rErrRegiNotFound,//

	RAS_rErrJsonCheckSize,
	RAS_rErrJsonGetArr,
	RAS_rErrJsonMakeArr,
	RAS_rErrJsonMakeStr,
	RAS_rErrJsonAppendVal,
	RAS_rErrJsonInvalidType,

	RAS_rSuccessMmchdl = 0
} ReturnCode_e;

char *ERR_GetDesc( int nErrCode );

#endif /* _RAS_ERR_H_ */
