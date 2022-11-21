/* RAS_Regi.h */
#ifndef _RAS_REGI_H
#define _RAS_REGI_H

#define REGI_DIR		"/RAS_CLI_IP_TRC"
#define REGI_SYS_ID		2
#define REGI_DELIM_KEY	":"REGI_DIR"/"
#define REGI_DELIM_VAL	"|"
#define REGI_KEY_MAX	63

int REGI_Init();
int REGI_GetAll();
int REGI_CheckKeyExist( const char *pszIp );

#endif /* _RAS_REGI_H_ */
