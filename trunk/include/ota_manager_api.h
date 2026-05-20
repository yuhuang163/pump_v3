#ifndef __OTA_MANAGER_API_H__
#define __OTA_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h" // IWYU pragma: keep

typedef enum {
    OTA_BROADCAST_UI_ENTER = 100,          /// ui进入.
    OTA_BROADCAST_UI_EXIT,                 /// ui退出.
    OTA_BROADCAST_STATE_CHANGE,            /// 状态改变.
    OTA_BROADCAST_ID_MAX,
} ota_broadcast_id_e;

typedef enum{
    OTA_TIMER_PROGRESS = 0,
    OTA_TIMER_MAX,
} ota_timer_type_e;

typedef enum {
    OTA_UPGRADE_STATE_NONE = 0,
    OTA_UPGRADE_STATE_START,
    OTA_UPGRADE_STATE_DOWNLOAD,
    OTA_UPGRADE_STATE_SUCCESS,
    OTA_UPGRADE_STATE_FAIL,
    OTA_UPGRADE_STATE_END,
} ota_upgrade_state_e;

typedef enum {
    OTA_OBJECT_TYPE_NONE = 0,
    OTA_OBJECT_MAIN_FIRMWARE = 100, /// 主固件
    OTA_OBJECT_IMAGE_RESOURCE, /// 图片资源
    OTA_OBJECT_TOUCH_KEY_FIRMWARE, /// 触摸按键固件
} ota_object_e;

typedef struct {
    bool is_working;
    uint8_t progress;
    uint32_t total_size;
    uint32_t recv_size;
    ota_upgrade_state_e state;
    ota_object_e object;
} ota_info_t;

/// 描述ota上下文对象
struct broadcast_obj;
struct rt_device_node;
typedef struct ota_manager_context {
/// private
    struct __ota_private {
        thread_context_t ota_work_thd;
        struct broadcast_obj *ota_bcd;
        ota_info_t ota_info;
    } priv;
    
/// public
    bool (*is_ota_upgrading)(void);
    void (*get_ota_info)(ota_info_t *info);
    int (*dispatch_ota_event_wrapper)(int event, ota_object_e object, int arg1, int arg2);
    /// 外部钩子函数.
    struct __ota_manager_hook {
        int (*on_post_init_route)(struct ota_manager_context *ctx);
        int (*on_ota_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
    } *hook;
} ota_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


