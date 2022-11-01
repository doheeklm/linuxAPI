/* RAS_Stat.h */
/* created by stmake utility, 2022-11-01-16:00:12 */
#ifndef _RAS_STAT_H_
#define _RAS_STAT_H_

typedef enum {
    HTTP_TOTAL_REQUEST,
    HTTP_REQUEST_POST,
    HTTP_REQUEST_GET,
    HTTP_REQUEST_DELETE,
    HTTP_REQUEST_UNKNOWN,
    HTTP_TOTAL_RESPONSE,
    HTTP_RESPONSE_POST201,
    HTTP_RESPONSE_POST400,
    HTTP_RESPONSE_POST500,
    HTTP_RESPONSE_GET200,
    HTTP_RESPONSE_GET400,
    HTTP_RESPONSE_GET404,
    HTTP_RESPONSE_GET500,
    HTTP_RESPONSE_DEL200,
    HTTP_RESPONSE_DEL400,
    HTTP_RESPONSE_DEL404,
    HTTP_RESPONSE_DEL500,
    HTTP_RESPONSE_UNKNOWN405,
    __ITEM_END__
}en_item_index_t;

extern stctl_item_desc_t stctl_item_list[];
extern stctl_dtl_type_desc_t stctl_dtl_type_list[];

int STAT_Init();

#endif /* _RAS_STAT_H_ */
