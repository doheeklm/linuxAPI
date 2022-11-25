/* RAS_Err.h */
#ifndef _RAS_ERR_H_
#define _RAS_ERR_H_

#define MMC_FAIL_REASON_OVERFLOW		"OVERFLOW"
#define MMC_FAIL_REASON_INVALID_PARAM	"INVALID PARAM"
#define MMC_FAIL_REASON_DB_FAIL			"DB FAIL"
#define MMC_FAIL_REASON_DB_NOT_FOUND	"DB NOT FOUND"
#define MMC_FAIL_REASON_DB_DUPLICATE	"DB DUPLICATE"
#define MMC_FAIL_REASON_REGI_FAIL		"REGI FAIL"
#define MMC_FAIL_REASON_REGI_KEY_EXIST	"REGI KEY EXIST"
#define MMC_FAIL_REASON_REGI_NOT_FOUND	"REGI NOT FOUND"

typedef enum
{
	RAS_rOK = 1000,
	RAS_rSig,
	RAS_rErrFail,
	RAS_rErrDuplicate,
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
	RAS_rErrWorkerInit,
	RAS_rErrSocketInit,
	RAS_rErrEventInit,
	RAS_rErrIpcGetKey,
	RAS_rErrInvalidParam,//

	RAS_rErrEpollFdListenFd,
	RAS_rErrEpollFd,
	RAS_rErrListenFd,

	RAS_rErrIpcRecv,
	RAS_rErrIpcSend,//
	RAS_rErrInvalidData,
	RAS_rErrInvalidValue,

	RAS_rErrDBConn,
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
	RAS_rErrDBDuplicate,
	
	RAS_rErrStatOpen,

	RAS_rErrRegiCreate,//
	RAS_rErrRegiKeyExist,//
	RAS_rErrRegiSetValue,//
	RAS_rErrRegiGetEnumKeyValue,//
	RAS_rErrRegiGetValue,//
	RAS_rErrRegiDelete,//
	RAS_rErrRegiNotFound,//

	RAS_rErrHttpRead,
	RAS_rErrHttpWrite,
	RAS_rErrHttpBadRequest,//400
	RAS_rErrHttpMethodNotAllowed,//405
	RAS_rHttpOK,//200
	RAS_rHttpCreated,//201
	RAS_rHttpNotFound,//404
	RAS_rHttpInternalFail,//500

	RAS_rErrAlarmCreate,
	RAS_rErrAlarmReport,

	RAS_rSuccessMmchdl = 0
} ReturnCode_e;

char* ERR_GetDesc( int nErrCode );

#endif /* _RAS_ERR_H_ */
