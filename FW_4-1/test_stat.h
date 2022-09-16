/* created by stmake utility, 2022-09-16-10:42:39 */
#ifndef __test_stat_H__
#define __test_stat_H__

#include <sys/types.h>
#include "stctl/stctl.h"

/* for old style stat api */
#define STAT_STYPE_MIPCSVR "MIPCSVR"
#define STATMIPCSVR 0

/* enumerated type for item list */
typedef enum {
    TCP_RCV_TOTAL_INV,
    RET_CODE_SUCCESS,
    RET_CODE_INVALID_PARAMETER,
    RET_CODE_SYSTEM_FAIL,
    RET_CODE_UNKNOWN_USER,
    __ITEM_END__
}en_item_index_t;

/* item list */
extern stctl_item_desc_t stctl_item_list[];
/* detail item type list */
extern stctl_dtl_type_desc_t stctl_dtl_type_list[];

/*
 * when you call stgen_open() function, you should use item-list and
 * detail-item-type-list as a parameter
 *
 * for example
 * stgen_open("MODULE-NAME", stctl_item_list, stctl_dtl_type_list);
 *
 * if the module do not have detail items, second parameter is NULL
 * stgen_open("MODULE-NAME", stctl_item_list, NULL);
 */

#endif /* #define  __test_stat_H__ */
