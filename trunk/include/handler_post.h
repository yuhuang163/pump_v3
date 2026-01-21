#ifndef __HANDLER_POST_H__
#define __HANDLER_POST_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "stdint.h"
#include "root_list_api.h" // IWYU pragma: keep
#include "cmsis_os_api.h" // IWYU pragma: keep

/**
 * @brief 该任务名称必须统一.
*/
#define __OTA_UPGRADE_RES_TASK "__ota_upgrade_res_task"
#define __REBOOT_REASON_FACTORY_RESET "__reboot_reason_factory_reset"

#define __SYSTEM_REBOOT_TASK "__system_reboot_task"
#define __SETTING_FACTORY_RESET_TASK "__setting_factory_reset_task"
#define __FACTORY_WORK_TASK "factory_work_task"
#define __MOTOR_PLAY_TASK "__motor_play_task"
#define __MISC_POLLING_TASK "__misc_polling_task"
#define __PM_SUSPEND_CHECK_TASK "__pm_suspend_check_task"

#define HDR_SKIP_TASK_SET() \
    __SYSTEM_REBOOT_TASK, \
    __SETTING_FACTORY_RESET_TASK, \
    __FACTORY_WORK_TASK, \
    __MOTOR_PLAY_TASK, \
    __MISC_POLLING_TASK, \
    __PM_SUSPEND_CHECK_TASK

struct multi_func_msg_data;
struct thread_context;

typedef void (*hdr_post_callback_fptr)(void *hdr_args);
typedef int (*task_post_process_fptr)(struct multi_func_msg_data *msg, struct thread_context *thd);

typedef struct hdr_post_args {
    uint32_t margic;
    const char *name;
    int32_t arg1;
    int32_t arg2;
    void *user_data;
    uint32_t from_tid;
    hdr_post_callback_fptr cb;
    struct thread_context *thd;
    int8_t core_affinity;
    uint8_t exclusive_task;
    uint8_t run_on_ui_thd;
    uint32_t timestamp;
} hdr_post_args_t;

typedef struct {
    uint32_t create_time;
    uint32_t excpect_time;
    uint32_t arrival_time;
    hdr_post_args_t args;
    list_head_t list;
} hdr_post_node_t;

/**
 * @brief 任务（延迟）投递机制.
*/
void init_handler_post_context(void);
void post_task_handler(hdr_post_args_t *args, uint32_t delay_ms);
int remove_task_handler(const char *task_name);
void handler_post_sample_test(void);
int is_handler_post_queue_busy(void);
void handle_post_register_ui_thd_cb(os_message_id ui_msg_id, task_post_process_fptr **cb);

/**
 * @brief 直接投递.
*/
#define POST_TASK_HANDLER_NO_DELAY(cb, __user_data, __name) POST_TASK_HANDLER_DELAYED(cb, 0, 0, 0, __user_data, __name, 0, -1, 0)
#define POST_TASK_HANDLER_NO_DELAY_ARG1(cb, __user_data, arg1, __name) POST_TASK_HANDLER_DELAYED(cb, 0, arg1, 0, __user_data, __name, 0, -1, 0)
#define POST_TASK_HANDLER_NO_DELAY_ARG2(cb, __user_data, arg1, arg2, __name) POST_TASK_HANDLER_DELAYED(cb, 0, arg1, arg2, __user_data, __name, 0, -1, 0)

/**
 * @brief 带延迟参数的任务投递
*/
#define POST_TASK_HANDLER_DELAYED_ARG0(cb, delay_ms, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, 0, 0, NULL, __name, 0, -1, 0)
#define POST_TASK_HANDLER_DELAYED_ARG1(cb, delay_ms, __user_data, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, 0, 0, __user_data, __name, 0, -1, 0)
#define POST_TASK_HANDLER_DELAYED_ARG2(cb, delay_ms, arg1, __user_data, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, arg1, 0, __user_data, __name, 0, -1, 0)
#define POST_TASK_HANDLER_DELAYED_ARG3(cb, delay_ms, arg1, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, arg1, 0, NULL, __name, 0, -1, 0)
#define POST_TASK_HANDLER_DELAYED_ARG4(cb, delay_ms, arg1, arg2, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, arg1, arg2, NULL, __name, 0, -1, 0)
#define POST_TASK_HANDLER_DELAYED_ARG5(cb, delay_ms, arg1, arg2, __user_data, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, arg1, arg2, __user_data, __name, 0, -1, 0)

/**
 * @brief 带独占参数任务投递
*/
#define POST_TASK_HANDLER_DELAYED_EXCLUSIVE(cb, delay_ms, arg1, __user_data, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, arg1, 0, __user_data, __name, 1, -1, 0)

/**
 * @brief 投递到UI线程,不可执行长时间阻塞任务，否则导致UI卡顿.
*/
#define POST_TASK_HANDLER_DELAYED_RUN_UI_THD(cb, delay_ms, arg1, arg2, __user_data, __name) POST_TASK_HANDLER_DELAYED(cb, delay_ms, arg1, arg2, __user_data, __name, 0, -1, 1)

/**
 * @brief internal.
*/
#define POST_TASK_HANDLER_DELAYED(__cb, __delay_ms, __arg1, __arg2, __user_data, __name, __exclusive_task, __core, __run_on_ui_thd) \
do { \
    REMOVE_TASK_HANDLER_DIRECTLY(__name); \
    hdr_post_args_t _args = {0}; \
    _args.cb = (hdr_post_callback_fptr)(__cb); \
    _args.arg1 = (__arg1); \
    _args.arg2 = (__arg2); \
    _args.user_data = (__user_data); \
    _args.name = (const char *)__name; \
    _args.exclusive_task = (__exclusive_task); \
    _args.core_affinity = (__core); \
    _args.run_on_ui_thd = (__run_on_ui_thd); \
    post_task_handler(&(_args), (__delay_ms)); \
} while (0)

/**
 * @brief 移除.
*/
#define REMOVE_TASK_HANDLER_DIRECTLY(__name) \
    do { \
        remove_task_handler(__name); \
    } while (0)

/**
 * @brief 将UI线程消息句柄加入到投递线程池中.
*/
#define REGISTER_TO_HANDLER_POST_POOL(ui_msg_id, process_cb_p) \
    handle_post_register_ui_thd_cb(ui_msg_id, process_cb_p)

#ifdef __cplusplus
}
#endif

#endif 

