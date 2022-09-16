/* created by stmake utility, 2022-09-16-10:42:39 */
#include <stdio.h>
#include <stdlib.h>
#include "stctl/stctl.h"
#include "test_stat.h"

/* detail item type list */
stctl_dtl_type_desc_t stctl_dtl_type_list[] = {
    {"HOSTID", 128},
    {"PROCESS", 128},
    {NULL, 0}
};

/* item list */
stctl_item_desc_t stctl_item_list[] = {
    {"TCP_RCV_TOTAL_INV", TCP_RCV_TOTAL_INV, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "PROCESS"},
    {"RET_CODE_SUCCESS", RET_CODE_SUCCESS, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "PROCESS"},
    {"RET_CODE_INVALID_PARAMETER", RET_CODE_INVALID_PARAMETER, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "PROCESS"},
    {"RET_CODE_SYSTEM_FAIL", RET_CODE_SYSTEM_FAIL, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "PROCESS"},
    {"RET_CODE_UNKNOWN_USER", RET_CODE_UNKNOWN_USER, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, "PROCESS"},
    {NULL, 0, 0, 0, NULL}
};

#if 0
/* example source code */

int main(int argc, char **argv)
{
    int ret;
    /*
     * stgen_open()
     * initialize statistics data structure, and open the shared memory
     * param 1 : module name
     * param 2 : item description data
     * param 3 : detail type description data
     */
    ret = stgen_open("PROCNAME", stctl_item_list, stctl_dtl_type_list);
    if(ret < 0) {
        LOG_ERR("stgen_open() failed, ret=%d(%s)", ret, stctl_strerror(ret));
        exit(1);
    }
    while(service_loop) {
        /*
         * set value functions of normal items 
         */
        STGEN_ITEM_SET_VALUE(TCP_RCV_TOTAL_INV, 1);
        STGEN_ITEM_1COUNT(TCP_RCV_TOTAL_INV);
        STGEN_ITEM_NCOUNT(TCP_RCV_TOTAL_INV, 1);


        /*
         * set value functions of detail items
         * STCTL_DTLSET_O_NORMAL set the value of normal item togegher
         * STCTL_DTLSET_O_KEYONLY set the value of detail item only
         */
        STGEN_DTLITEM_SET_VALUE(TCP_RCV_TOTAL_INV, "key_1", 1, STCTL_DTLSET_O_NORMAL);
        /* is same to */
        STGEN_DTLITEM_SET_VALUE(TCP_RCV_TOTAL_INV, "key_1", 1, 0);
        STGEN_DTLITEM_1COUNT(TCP_RCV_TOTAL_INV, "key_1");
        STGEN_DTLITEM_NCOUNT(TCP_RCV_TOTAL_INV, "key_1", 1);

        STGEN_DTLITEM_SET_VALUE(TCP_RCV_TOTAL_INV, "key_2", 1, STCTL_DTLSET_O_KEYONLY);
        /* is same to */
        STGEN_DTLITEM_SET_VALUE(TCP_RCV_TOTAL_INV, "key_2", 1, 1);
        STGEN_DTLITEM_1COUNT2(TCP_RCV_TOTAL_INV, "key_2");
        STGEN_DTLITEM_NCOUNT2(TCP_RCV_TOTAL_INV, "key_2", 1);
    }
    /* stop operation and detach the shared memory */
    stgen_close();
}
#endif
