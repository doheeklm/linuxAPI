/* RAS_Stat.c */
/* created by stmake utility, 2022-11-01-16:00:12 */
#include "RAS_Inc.h"

stctl_dtl_type_desc_t stctl_dtl_type_list[] = {
    {"IPADDR", 64},
    {NULL, 0}
};

stctl_item_desc_t stctl_item_list[] = {
    {"HTTP_TOTAL_REQUEST", HTTP_TOTAL_REQUEST, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_REQUEST_POST", HTTP_REQUEST_POST, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_REQUEST_GET", HTTP_REQUEST_GET, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_REQUEST_DELETE", HTTP_REQUEST_DELETE, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_REQUEST_UNKNOWN", HTTP_REQUEST_UNKNOWN, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_TOTAL_RESPONSE", HTTP_TOTAL_RESPONSE, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_POST201", HTTP_RESPONSE_POST201, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_POST400", HTTP_RESPONSE_POST400, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_POST500", HTTP_RESPONSE_POST500, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_GET200", HTTP_RESPONSE_GET200, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_GET400", HTTP_RESPONSE_GET400, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_GET404", HTTP_RESPONSE_GET404, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_GET500", HTTP_RESPONSE_GET500, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_DEL200", HTTP_RESPONSE_DEL200, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_DEL400", HTTP_RESPONSE_DEL400, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_DEL404", HTTP_RESPONSE_DEL404, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_DEL500", HTTP_RESPONSE_DEL500, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {"HTTP_RESPONSE_UNKNOWN405", HTTP_RESPONSE_UNKNOWN405, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "IPADDR"},
    {NULL, 0, 0, 0, NULL}
};

int STAT_Init()
{
	int nRC = 0;

	nRC = stgen_open( PROCESS_NAME, stctl_item_list, stctl_dtl_type_list );
	if ( 0 > nRC )
	{
		LOG_ERR_F( "stgen_open fail <%d>", nRC );
		return RAS_rErrStatInit;
	}	

	return RAS_rOK;
}

void STAT_Count( int nHttpType, int nMethod, int nStatusCode, const char *pszIp )
{
	switch ( nHttpType )
	{
		case HTTP_TYPE_REQUEST:
		{
			STGEN_DTLITEM_1COUNT( HTTP_TOTAL_REQUEST, pszIp );

			switch ( nMethod )
			{
				case HTTP_METHOD_POST_NUM:
				{
					STGEN_DTLITEM_1COUNT( HTTP_REQUEST_POST, pszIp );
				}
					break;
				case HTTP_METHOD_GET_NUM:
				{
					STGEN_DTLITEM_1COUNT( HTTP_REQUEST_GET, pszIp );
				}
					break;
				case HTTP_METHOD_DEL_NUM:
				{
					STGEN_DTLITEM_1COUNT( HTTP_REQUEST_DELETE, pszIp );
				}
					break;
				default:
				{
					STGEN_DTLITEM_1COUNT( HTTP_REQUEST_UNKNOWN, pszIp );
				}
					break;
			}
		}
			break;
		case HTTP_TYPE_RESPONSE:
		{
			STGEN_DTLITEM_1COUNT( HTTP_TOTAL_RESPONSE, pszIp );
			
			switch ( nMethod )
			{
				case HTTP_METHOD_POST_NUM:
				{
					switch ( nStatusCode )
					{
						case STATUS_CODE_201:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_POST201, pszIp );
						}
							break;
						case STATUS_CODE_400:
						{						
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_POST400, pszIp );
						}
							break;
						case STATUS_CODE_500:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_POST500, pszIp );
						}
							break;
					}
				}
					break;
				case HTTP_METHOD_GET_NUM:
				{
					switch ( nStatusCode )
					{
						case STATUS_CODE_200:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_GET200, pszIp );
						}
							break;
						case STATUS_CODE_400:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_GET400, pszIp );
						}
							break;
						case STATUS_CODE_404:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_GET404, pszIp );
						}
							break;
						case STATUS_CODE_500:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_GET500, pszIp );
						}
							break;
					}
				}
					break;
				case HTTP_METHOD_DEL_NUM:
				{
					switch ( nStatusCode )
					{
						case STATUS_CODE_200:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_DEL200, pszIp );
						}
							break;
						case STATUS_CODE_400:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_DEL400, pszIp );
						}
							break;
						case STATUS_CODE_404:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_DEL404, pszIp );
						}
							break;
						case STATUS_CODE_500:
						{
							STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_DEL500, pszIp );
						}
							break;
					}
				}
					break;
				default:
				{
					STGEN_DTLITEM_1COUNT( HTTP_RESPONSE_UNKNOWN405, pszIp );
				}
					break;
			}//switch(nMethod)

		}//case(HTTP_TYPE_RESPONSE)
			break;
	}//switch(nHttpType)
}
