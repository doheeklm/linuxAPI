/* RAS_Ipc.h */
#ifndef _RAS_IPC_H
#define _RAS_IPC_H

#define CHECK_INTERVAL_MSEC		100			//mpipc thread가 ipc message를 확인하는 주기
#define STOP_GRACE_TIME_MSEC	300			//mpipc 종료시 대기시간

#define MSG_ID_ADD_CLI_IP		7748
#define MSG_ID_DIS_CLI_IP		7749
#define MSG_ID_DEL_CLI_IP		7750
#define MSG_ID_ADD_CLI_IP_TRC	7751
#define MSG_ID_DIS_CLI_IP_TRC	7752
#define MSG_ID_DEL_CLI_IP_TRC	7753
#define MSG_ID_DIS_USR_INFO		7754
#define MSG_ID_DEL_USR_INFO		7755

int IPC_Init();
int IPC_Handler( mpipc_t *ptMpipc, iipc_msg_t *ptRecvMsg, void *pvData );
void IPC_Destroy( mpipc_t *ptMpipc );

#endif /* _RAS_IPC_H_ */
