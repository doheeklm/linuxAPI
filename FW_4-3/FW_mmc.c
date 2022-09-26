/* FW_4-3/FW_mmc.c */
#include "FW_Inc.h"

int MMC_Init( char *pszModule, oammmc_t *ptMmc, iipc_ds_t *ptIpc )
{
	int nRC = 0;
	int nMmtEnable = 0;
	int nMmtPort = 0;

	ptMmc = oammmc_init( pszModule );
	if ( NULL == ptMmc )
	{
		MPGLOG_ERR( "%s:: oammmc_init fail", __func__ );
		//return
	}

	mpconf_get_int( "", MODULE_CONF, SERVER_PROCNAME, MMT_ENABLE, &nMmtEnable, 1, 0 );

	if ( nMmtEnable == 1 )
	{
		//mpconf get int mmt_port
	
	}
}
