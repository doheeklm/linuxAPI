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
