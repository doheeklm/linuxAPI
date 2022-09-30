/* FW_4-3/FW_Mmc.h */
#ifndef _FW_MMC_H_
#define _FW_MMC_H_

int MMC_Init( char *pszModule, oammmc_t *ptMmc, iipc_ds_t *ptIpc );
void MMC_Destroy( oammmc_t *ptMmc );

#endif /*_FW_MMC_H_*/
