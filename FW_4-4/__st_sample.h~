/* created by stmake utility, 2022-09-14-15:43:32 */
#ifndef ____st_sample_H__
#define ____st_sample_H__

#include <sys/types.h>
#include "stctl/stctl.h"

/* enumerated type for item list */
typedef enum {
    ITEM1,
    ITEM2,
    ITEM3,
    ITEM4,
    ITEM5,
    ITEM6,
    ITEM7,
    ITEM8,
    ITEM9,
    __ITEM_END__
}en_item_index_t;

/* item list */
extern stctl_item_desc_t stctl_item_list[];

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

#endif /* #define  ____st_sample_H__ */
