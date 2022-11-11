/* RAS_Err.c */
#include "RAS_Inc.h"

char *ERR_GetDesc( int nErrCode )
{
	switch( nErrCode )
	{
		STR_CASE( RAS_rErrInvalidParam, MMC_FAIL_REASON_INVALID_PARAM );
		STR_CASE( RAS_rErrOverflow, MMC_FAIL_REASON_SYSTEM_FAIL );
		
		STR_CASE( RAS_rErrDBSetValue, MMC_FAIL_REASON_DB_FAIL );
		STR_CASE( RAS_rErrDBGetValue, MMC_FAIL_REASON_DB_FAIL );
		STR_CASE( RAS_rErrDBExecute, MMC_FAIL_REASON_DB_FAIL );
		STR_CASE( RAS_rErrDBExecUpdate, MMC_FAIL_REASON_DB_FAIL );
		STR_CASE( RAS_rErrDBNotFound, MMC_FAIL_REASON_DB_NOT_FOUND );	

		STR_CASE( RAS_rErrRegiCreate, MMC_FAIL_REASON_REGI_FAIL );
		STR_CASE( RAS_rErrRegiSetValue, MMC_FAIL_REASON_REGI_FAIL );
		STR_CASE( RAS_rErrRegiGetValue, MMC_FAIL_REASON_REGI_FAIL );
		STR_CASE( RAS_rErrRegiGetKeyAndValue, MMC_FAIL_REASON_REGI_FAIL );
		STR_CASE( RAS_rErrRegiDelete, MMC_FAIL_REASON_REGI_FAIL );
		STR_CASE( RAS_rErrRegiKeyExist, MMC_FAIL_REASON_REGI_KEY_EXIST );
		STR_CASE( RAS_rErrRegiNotFound, MMC_FAIL_REASON_REGI_NOT_FOUND );	
		STR_CASE_DFLT_UKN;
	}
}
