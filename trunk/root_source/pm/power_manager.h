#ifndef __POWER_MANAGER_H__
#define __POWER_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

// 调整休眠时间.
#ifndef PM_SUSPEND_TIME_S
#define PM_SUSPEND_TIME_S  (1000)
#endif

// 低电量休眠时间.
#ifndef PM_SUSPEND_LOW_BAT_TIME_S
#define PM_SUSPEND_LOW_BAT_TIME_S  (4)
#endif

#define IS_WAKEUP_BY_KEY_POWER(reason) ((reason == PM_WAKEUP_REASON_KEY_POWER) ? 1 : 0)
#define IS_WAKEUP_BY_KEY_MODE(reason) ((reason == PM_WAKEUP_REASON_KEY_MODE) ? 1 : 0)
#define IS_WAKEUP_BY_IMU(reason) ((reason == PM_WAKEUP_REASON_IMU_IRQ) ? 1 : 0)
#define IS_WAKEUP_BY_BLE(reason) ((reason == PM_WAKEUP_REASON_BLE_REQ) ? 1 : 0)
#define IS_WAKEUP_BY_OTA(reason) ((reason == PM_WAKEUP_REASON_OTA) ? 1 : 0)
#define IS_WAKEUP_BY_IOT(reason) ((reason == PM_WAKEUP_REASON_IOT) ? 1 : 0)
#define IS_WAKEUP_BY_BOOT(reason) ((reason == PM_WAKEUP_REASON_BOOT) ? 1 : 0)

typedef enum {
    PM_SUSPEND_GOING = 0,   // 挂起过程中
    PM_SUSPEND_FINISH,      // 挂起完成
    PM_RESUME_GOING = 0x10,   // 恢复过程中
    PM_RESUME_FINISH,       // 恢复完成
} pm_wakeup_state_e;

typedef struct {
    int timeout_s;
    char caller[32];
    list_head_t list;
} pm_wake_lock_node_t;


#ifdef __cplusplus
}
#endif

#endif 
