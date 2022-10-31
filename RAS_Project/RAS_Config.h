/* RAS_Config.h */
#ifndef _RAS_CONFIG_H
#define _RAS_CONFIG_H

#define CONFIG_PATH			"./RAS_Config.ut"
#define CONN_SECTION		"CONNECTION"
#define LOG_SECTION			"LOG"
#define MMC_SECTION			"MMC"

typedef struct
{
	char szIp[40];
	int nPort;

	int nLogMode;
	int nLogLevel;

	int nMmtEnable;
	int nMmtPort;
	int nMmtConnMax;
	int nMmtLocalOnly;
	int nMmtIsQuiet;
	int nMmlEnable;
} Env_t;

int CONF_Init( void );

#endif /* _RAS_CONFIG_H */
