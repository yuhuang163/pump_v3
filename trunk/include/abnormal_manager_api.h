#ifndef __ABNORMAL_MANAGER_API_H__
#define __ABNORMAL_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define SYS_ABNORMAL_BAT_TEMP_HIGH_THRESHOLD_C    60
#define SYS_ABNORMAL_BAT_TEMP_NORMAL_THRESHOLD_C    45

/// 系统异常类型
typedef enum {
    SYS_ABNORMAL_BATTERY_TEMP_HIGH = 0,   ///< E：电池温度 > 60℃
    SYS_ABNORMAL_STORAGE_FAILURE,         ///< F：存储芯片读写校验失败
    SYS_ABNORMAL_COMMUNICATION_ERROR,     ///< E：主控与显示屏/配件通信丢失
    SYS_ABNORMAL_TYPE_MAX,
} sys_abnormal_type_e;

typedef enum {
    SYS_ABNORMAL_SEVERITY_ERROR = 0, ///< E：停止相关功能，禁止新任务（优先级：低）
    SYS_ABNORMAL_SEVERITY_FATAL = 1, ///< F：立即停止，锁定核心功能（优先级：高）
} sys_abnormal_severity_e;

#define SYS_ABNORMAL_PRIORITY_LOW     SYS_ABNORMAL_SEVERITY_ERROR
#define SYS_ABNORMAL_PRIORITY_HIGH    SYS_ABNORMAL_SEVERITY_FATAL

typedef enum {
    SYS_ABNORMAL_STATE_CLEARED = 0x00, ///< 异常解除
    SYS_ABNORMAL_STATE_ACTIVE = 0x01,  ///< 异常产生
} sys_abnormal_state_e;

typedef struct {
    uint8_t type;  /// sys_abnormal_type_e
    uint8_t state; /// sys_abnormal_state_e
} sys_abnormal_state_info_t;

typedef void (*on_abnormal_state_cb_ptr)(void *user_data, sys_abnormal_state_info_t *info);

#define ADD_ABNORMAL_STATE_CB(ctx, user_data, cb) \
do { \
    if (ctx) { \
        ctx->add_abnormal_state_cb(user_data, cb); \
    } else { \
        LOG_MISC("param error, ignore."); \
    } \
} while (0)

#define REMOVE_ABNORMAL_STATE_CB(ctx, cb) \
do { \
    if (ctx) { \
        ctx->remove_abnormal_state_cb(cb); \
    } else { \
        LOG_MISC("param error, ignore."); \
    } \
} while (0)

typedef struct abnormal_manager_context {
    uint8_t (*set_abnormal_state)(uint8_t type, uint8_t state);
    uint8_t (*get_abnormal_state)(uint8_t type);
    uint8_t (*is_abnormal_active)(uint8_t type);
    uint8_t (*get_abnormal_severity)(uint8_t type);
    uint8_t (*is_new_task_forbidden)(void);
    uint8_t (*is_core_locked)(void);
    void (*add_abnormal_state_cb)(void *user_data, on_abnormal_state_cb_ptr cb);
    void (*remove_abnormal_state_cb)(on_abnormal_state_cb_ptr cb);
    void (*notify_abnormal_state_change)(sys_abnormal_state_info_t *info);

    struct __abnormal_manager_hook {
        int (*on_post_init_route)(struct abnormal_manager_context *ctx);
    } *hook;

    struct __abnormal_private {
        uint8_t state[SYS_ABNORMAL_TYPE_MAX];
        list_head_t client_list;
        os_mutex_id m_lock;
    } priv;
} abnormal_manager_context_t;

void sys_abnormal_report_storage_failure(uint8_t active);
void sys_abnormal_report_communication_error(uint8_t active);

#ifdef __cplusplus
}
#endif

#endif
