/* created by stmake utility, 2022-09-14-15:43:32 */
#include <stdio.h>
#include <stdlib.h>
#include "stctl/stctl.h"
#include "__st_sample.h"

/* item list */
stctl_item_desc_t stctl_item_list[] = {
    {"ITEM1", ITEM1, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, NULL},
    {"ITEM2", ITEM2, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, NULL},
    {"ITEM3", ITEM3, STCTL_VTYPE_UINT, STCTL_CTYPE_SUM, NULL},
    {"ITEM4", ITEM4, STCTL_VTYPE_INT, STCTL_CTYPE_MAX, NULL},
    {"ITEM5", ITEM5, STCTL_VTYPE_INT, STCTL_CTYPE_MIN, NULL},
    {"ITEM6", ITEM6, STCTL_VTYPE_FLOAT, STCTL_CTYPE_AVR, NULL},
    {"ITEM7", ITEM7, STCTL_VTYPE_FLOAT, STCTL_CTYPE_CUR, NULL},
    {"ITEM8", ITEM8, STCTL_VTYPE_FLOAT, STCTL_CTYPE_MAX, NULL},
    {"ITEM9", ITEM9, STCTL_VTYPE_FLOAT, STCTL_CTYPE_MIN, NULL},
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
    ret = stgen_open("PROCNAME", stctl_item_list, NULL);
    if(ret < 0) {
        LOG_ERR("stgen_open() failed, ret=%d(%s)", ret, stctl_strerror(ret));
        exit(1);
    }
    while(service_loop) {
        /*
         * set value functions of normal items 
         */
        STGEN_ITEM_SET_VALUE(ITEM1, 1);
        STGEN_ITEM_1COUNT(ITEM1);
        STGEN_ITEM_NCOUNT(ITEM1, 1);
    }
    /* stop operation and detach the shared memory */
    stgen_close();
}
#endif
