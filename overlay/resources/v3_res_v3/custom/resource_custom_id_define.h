#ifndef __RESOURCE_CUSTOM_ID_DEFINE_H__
#define __RESOURCE_CUSTOM_ID_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/// custom 主题资源 ID 区间 [10000 .. 20000]（预留，当前无资源）
#define CUSTOM_RES_ID_RANGE_MIN                   10000
#define CUSTOM_RES_ID_RANGE_MAX                   20000

typedef enum {
    CUSTOM_RES_ID_NONE = 0, /// 无资源

    CUSTOM_RES_ID_TYPE_MAX
} custom_res_id_type_e;

#ifdef __cplusplus
}
#endif

#endif
