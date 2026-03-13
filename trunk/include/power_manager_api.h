#ifndef __POWER_MANAGER_API_H__
#define __POWER_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"

typedef enum {
    PM_WAKEUP_REASON_BOOT = 80,
    PM_WAKEUP_REASON_KEY_POWER,
    PM_WAKEUP_REASON_KEY_MODE,
    PM_WAKEUP_REASON_ENTER_NEW_SCREEN,
    PM_WAKEUP_REASON_IMU_IRQ,
    PM_WAKEUP_REASON_BLE_REQ,
    PM_WAKEUP_REASON_OTA,
    PM_WAKEUP_REASON_CHARGE_PLUG_IN,
    PM_WAKEUP_REASON_CHARGE_PLUG_OUI,
    PM_WAKEUP_REASON_BACKGROUND_TASK,
    PM_WAKEUP_REASON_IOT,               /// IOT任务周期更新.
    PM_WAKEUP_REASON_FACTORY_RESUME,
    PM_WAKEUP_REASON_MAX,
} pm_wakeup_reason_e;

typedef enum {
    ID_SYS_PM_RESUME = 100, /// 系统恢复状态.
    ID_SYS_PM_IDLE, /// 系统空闲状态.
    ID_SYS_PM_WILL_SUSPEND, /// 系统即将进入休眠状态.
    ID_SYS_PM_SUSPEND, /// 系统休眠状态.
    ID_SYS_PM_MAX
} system_pm_state_e;

typedef enum {
    PM_BROADCAST_SYS_RESUME = 100, /// 系统空闲进入.
    PM_BROADCAST_SYS_IDLE, /// 系统空闲状态.
    PM_BROADCAST_SYS_IDLE_RELEASE, /// 系统空闲释放.
    PM_BROADCAST_SYS_WILL_SUSPEND, /// 系统即将进入休眠状态.
    PM_BROADCAST_SYS_SUSPEND, /// 系统休眠状态.
    PM_BROADCAST_USER_REQUEST_SUSPEND, /// 用户请求进入休眠状态.
    PM_BROADCAST_BLE_REQUEST_SUSPEND, /// 蓝牙请求进入休眠状态.
    PM_BROADCAST_FACTORY_REQUEST_SUSPEND, /// 工厂请求进入休眠状态.
    PM_BROADCAST_ID_MAX,
} pm_broadcast_id_e;

struct broadcast_obj;
typedef struct power_manager_context {
/// private:
    struct __pm_private {
        struct peripheral_bus_context *pbus;
        int pm_state;
        int pm_state_change_progress;
        uint32_t suspend_counter;
        uint32_t pm_check_time_ms; /// 上次检查时间戳(毫秒级).
        pm_wakeup_reason_e wakeup_reason;
        struct broadcast_obj *pm_bcd;
        os_mutex_id m_wake_lock;
        os_mutex_id m_suspend_lock;
        os_timer_id suspend_timer;
        list_head_t wake_lock_list;
        thread_context_t pm_suspend_thd;
        int is_enter_suspend;
        bool pmu_sleep_enable;
        bool user_set_to_sleep;
        bool is_back_ground_wakeup;
        bool is_system_power_on;
        bool is_power_on_timeout_detect;
        bool allow_shutdown_or_standby;
        bool is_goto_sys_standby;
    } priv;

/// public:
    void (*init)(void);
    int (*is_pm_suspend)(void);
    int (*acquire_wakelock)(const char *caller, int timeout_s);
    int (*release_wakelock)(const char *caller);
    void (*pm_wakeup)(pm_wakeup_reason_e reason);
    int (*enable_pmu_sleep)(const char *caller, bool enable);
    int (*is_pmu_sleep)(void);
    int (*system_power_control)(bool state);
    void (*set_period_wake_up)(int ms);
    bool (*is_background_wakeup)(void);
    bool (*is_on_suspend_gotoing)(void);
    bool (*is_system_power_on)(void);
    pm_wakeup_reason_e (*pm_get_wakeup_reason)(void);
    const char *(*print_wakeup_state)(int state);
    /// 外部hook点.
    struct __power_manager_hook {
        int (*on_post_init_route)(struct power_manager_context *ctx);
        int (*on_key_event_route)(struct power_manager_context *ctx, uint16_t keycode, uint16_t keytype);
        int (*on_broadcast_receive_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata);
        int (*pm_suspend_route)(struct power_manager_context *ctx, int state);
        int (*pm_resume_route)(struct power_manager_context *ctx, int state);
        void (*pm_suspend_timer_cb_route)(struct power_manager_context *ctx);
        void (*set_suspend_counter_route)(struct power_manager_context *ctx);
        /// 禁止进入待机状态的钩子.
        bool (*is_prohibit_pm_suspend)(struct power_manager_context *ctx);
    } *hook;
} power_manager_context_t;


#ifdef __cplusplus
}
#endif

#endif 


