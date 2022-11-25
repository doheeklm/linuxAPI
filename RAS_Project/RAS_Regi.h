/* RAS_Regi.h */
#ifndef _RAS_REGI_H
#define _RAS_REGI_H

#define SIZE_IP			15
#define SIZE_TM			5	

#define REGI_SYS_ID		2
#define REGI_KEY_MAX	63

#define REGI_DIR		"/RAS_CLI_IP_TRC"
#define REGI_DELIM_KEY	":"REGI_DIR"/"
#define REGI_DELIM_VAL	"|"

#define MAX_REGI_CNT	1000

typedef struct REGI_s
{
	char szClientIp[SIZE_IP + 1];
	char szPeriodTm[SIZE_TM + 1];;

} REGI_t;

int REGI_Init();
int REGI_GetAll( int *pnCnt );
int REGI_CheckKeyExist( const char *pszIp );

#endif /* _RAS_REGI_H_ */
