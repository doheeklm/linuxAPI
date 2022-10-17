/* FW_Util.c */
#include "FW_Header.h"

int UTIL_GetConfig( char *pszIp, int *pnPort, int nSizeIp )
{
	int i = 0;

	mpconf_list_t *ptItemList = NULL;
	mpconf_list_t *ptValueList = NULL;

	ptValueList = mpconf_get_value_list( NULL, CONF_PATH, CONF_SECTION_NAME, &ptItemList );
	if ( NULL == ptValueList )
	{
		MPGLOG_ERR( "%s:: mpconf_get_value_list() fail", __func__ );
		return MPCONF_FAIL;
	}

	for ( i = 0; i < ptItemList->name_num; i++ )
	{
		if ( 0 == strcmp( ptItemList->name[i], CONF_ITEM_NAME_IP ) )
		{
			strlcpy( pszIp, ptValueList->name[i], nSizeIp );
		}
		else if ( 0 == strcmp( ptItemList->name[i], CONF_ITEM_NAME_PORT ) )
		{
			*pnPort = atoi( ptValueList->name[i] );
		}
	}

	MPGLOG_DBG( "%s:: [%s] %s = %s | %s = %d", __func__, CONF_SECTION_NAME, CONF_ITEM_NAME_IP, pszIp, CONF_ITEM_NAME_PORT, *pnPort );

	UTIL_FreeList( NULL, ptItemList, ptValueList );

	return RC_SUCCESS;
}

void UTIL_FreeList( mpconf_list_t *ptSectList, mpconf_list_t *ptItemList, mpconf_list_t *ptValueList )
{
	if ( NULL != ptSectList )
	{
		mpconf_list_free( ptSectList );
	}

	if ( NULL != ptItemList )
	{
		mpconf_list_free( ptItemList );
	}

	if ( NULL != ptValueList )
	{
		mpconf_list_free( ptValueList );
	}
}
