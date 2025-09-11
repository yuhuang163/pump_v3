#ifndef __POWER_MANAGER_API_H__
#define __POWER_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

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
    PM_WAKEUP_REASON_MAX,
} pm_wakeup_reason_e;

typedef enum {
    ID_SYS_PM_RESUME = 110,
    ID_SYS_PM_SUSPEND,
    ID_SYS_PM_MAX
} system_pm_state_e;

struct broadcast_obj;
typedef struct power_manager_context {
/// private:
    struct __pm_private {
        int pm_state;
        int pm_state_change_progress;
        int suspend_counter;
        pm_wakeup_reason_e wakeup_reason;
        struct broadcast_obj *pm_bcd;
        os_mutex_id m_wake_lock;
        os_mutex_id m_suspend_lock;
        os_timer_id suspend_timer;
        list_head_t wake_lock_list;
        thread_context_t pm_suspend_thd;
        thread_context_t pmu_freeze_thd;
        int is_enter_suspend;
        bool pmu_sleep_enable;
        bool user_set_to_sleep;
        bool is_back_ground_wakeup;
        bool is_shutdown;
    } priv;

/// public:
    void (*init)(void);
    int (*is_pm_suspend)(void);
    int (*acquire_wakelock)(const char *caller, int timeout_s);
    int (*release_wakelock)(const char *caller);
    void (*pm_wakeup)(pm_wakeup_reason_e reason);
    int (*enable_pmu_sleep)(const char *caller, bool enable);
    int (*is_pmu_sleep)(void);
    
    void (*set_period_wake_up)(int ms);
    bool (*is_background_wakeup)(void);
    bool (*is_on_suspend_gotoing)(void);
    bool (*is_on_is_shutdown)(void);
    pm_wakeup_reason_e (*pm_get_wakeup_reason)(void);
} power_manager_context_t;


#ifdef __cplusplus
}
#endif

#endif 


