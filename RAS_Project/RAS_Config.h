/* RAS_Config.h */
#ifndef _RAS_CONFIG_H
#define _RAS_CONFIG_H

#define CONFIG_PATH			"./RAS_Config.ut"
#define CONN_SECTION		"CONNECTION"
#define REGI_SECTION		"REGI"
#define LOG_SECTION			"LOG"
#define MMC_SECTION			"MMC"

typedef struct
{
	char szIp[16];
	int nPort;
	char szRegiIp[16];
	int nRegiPort;
	
	int nLogMode;
	int nLogLevel;

	int nMmtEnable;
	int nMmtPort;
	int nMmtConnMax;
	int nMmtLocalOnly;
	int nMmtIsQuiet;
	int nMmlEnable;
} Env_t;

int CONFIG_Init( void );

#endif /* _RAS_CONFIG_H */
