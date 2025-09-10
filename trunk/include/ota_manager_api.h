#ifndef __OTA_MANAGER_API_H__
#define __OTA_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h"

typedef enum{
    OTA_TIMER_PROGRESS = 0,
    OTA_TIMER_MAX,
} ota_timer_type_e;

// typedef enum {
//     OTA_UPGRADE_STATE_NONE = 0,
//     OTA_UPGRADE_STATE_START,
//     OTA_UPGRADE_STATE_DOWNLOAD,
//     OTA_UPGRADE_STATE_END,
// } ota_upgrade_state_e;

// 描述ota管理上下文对象
typedef struct ota_manager_context {
// private
    thread_context_t ota_work_thd;
    os_timer_id ota_timer[OTA_TIMER_MAX];
    broadcast_obj_t bcreceiver;

    bool is_working;
    uint8_t ota_progress;
    // ota_upgrade_state_e ota_state;

// public
    void (*init)(void);

    void (*ota_thread_suspend)(void);
    void (*ota_thread_resume)(void);

    bool (*is_ota_upgrading)(void);
    void (*set_ota_upgrading)(bool yes);

    uint8_t (*get_ota_upgrade_progress)(void);
    void (*set_ota_upgrade_progress)(uint8_t progress);

    // ota_upgrade_state_e (*get_ota_upgrade_state)(void);
    // void (*set_ota_upgrade_state)(ota_upgrade_state_e state);
} ota_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


