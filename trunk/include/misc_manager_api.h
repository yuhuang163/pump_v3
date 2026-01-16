#ifndef __MISC_MANAGER_API_H__
#define __MISC_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

typedef enum {
    MISC_TASK_ID_SYSINFO_PRINT = 0,
    MISC_TASK_ID_CHARGE_POLLING = 1,
    MISC_TASK_ID_BATTERY_POLLING = 2,
    MISC_TASK_ID_BATTERY_VOLTAGE_POLLING = 3,
    MISC_TASK_ID_ADAPTIVE_DIMMING = 4,
    MISC_TASK_ID_MAX
} misc_task_id_e;

/// misc工作线程消息ID类型（位掩码类型）
typedef enum {
    MISC_WORK_MSG_ID_TIMER = (1 << 0),        /// 定时器触发
    MISC_WORK_MSG_ID_CHARGE_INTERRUPT = (1 << 1), /// 充电中断触发
} misc_work_msg_id_e;

/// misc工作线程消息ID位掩码类型
typedef uint32_t misc_work_msg_id_mask_t;

/// 杂项任务回调函数类型
typedef void (*on_misc_task_cb_fptr)(void *user_data);

/// 描述misc杂项管理上下文对象
struct rt_device_node;
struct broadcast_obj;
typedef struct misc_manager_context {
/// public:
    void (*add_misc_task)(void *user_data, int task_id, uint32_t polling_period, misc_work_msg_id_mask_t msg_id_mask, on_misc_task_cb_fptr cb);
    void (*remove_misc_task)(on_misc_task_cb_fptr cb);
    void (*trigger_misc_work)(misc_work_msg_id_mask_t msg_id_mask);  /// 立即触发misc_work_polling_thread执行，带消息ID位掩码
/// private:
    struct __misc_private {
        thread_context_t misc_work_thd;
        os_timer_id misc_timer;
        struct broadcast_obj *misc_bcd;;
        os_mutex_id m_lock;          /// 保护任务链表的互斥锁
        list_head_t task_list;       /// 任务链表
    } priv;
} misc_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


