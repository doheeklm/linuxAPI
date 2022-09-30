/* FW_4-3/FW_Pstmt.h */
#ifndef _FW_PSTMT_H_
#define _FW_PSTMT_H_

int PSTMT_Init( DAL_CONN *ptDal, DAL_PSTMT *ptPstmtInsert, DAL_PSTMT *ptPstmtSelectAll, DAL_PSTMT *ptPstmtSelectOne, DAL_PSTMT *ptPstmtUpdate, DAL_PSTMT *ptPstmtDelete, DAL_PSTMT *ptPstmtNumtuples );

void PSTMT_Destroy( DAL_PSTMT *ptPstmtInsert, DAL_PSTMT *ptPstmtSelectAll, DAL_PSTMT *ptPstmtSelectOne, DAL_PSTMT *ptPstmtUpdate, DAL_PSTMT *ptPstmtDelete, DAL_PSTMT *ptPstmtNumTuples );

#endif /*_FW_PSTMT_H_*/
