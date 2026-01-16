#ifndef __ALGO_FUSION_OS_H__
#define __ALGO_FUSION_OS_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep
#include "stdbool.h" // IWYU pragma: keep
#include "stddef.h"

#define IS_SUPPORT_ROOT_PLATFORM       1

typedef enum {
    ALGO_OS_TIMER_ONCE = 0,       
    ALGO_OS_TIMER_PERIODIC  = 1  
} algo_os_timer_type_e;

#define ALGO_OS_WAIT_FOREVER         0xFFFFFFFFU     ///< wait forever timeout value

typedef void *algo_os_thread_id;
typedef void *algo_os_message_id;
typedef void *algo_os_timer_id;

#if IS_SUPPORT_ROOT_PLATFORM
#include "cmsis_os_api.h"  // IWYU pragma: keep

/// 内存分配
#define ALGO_MALLOC(size)                 u_malloc(size)
#define ALGO_MALLOZ(size)                 u_malloz(size)
/// 释放内存
#define ALGO_FREE(ptr)                    u_free(ptr)
/// 获取系统时间
#define ALGO_SYS_TIME()                   get_timestamp()
/// 最多创建的线程数
#define ALGO_THREAD_MAX_NUM               10
/// 线程上下文
static thread_context_t *thd[ALGO_THREAD_MAX_NUM] = {NULL};
/// 创建线程
#define ALGO_THREAD_CREATE(name_s, cb, prio, stack_s) \
({ \
    algo_os_thread_id __result = NULL; \
    for (int i = 0; i < ALGO_THREAD_MAX_NUM; i++) { \
        if (thd[i] == NULL) { \
            thd[i] = (thread_context_t *)u_malloz(sizeof(thread_context_t)); \
            if (thd[i] != NULL) { \
                if (run_new_thread_wrapper((thd[i]), (name_s), (cb), (prio), (stack_s)) == 0) { \
                    __result = (algo_os_thread_id)(thd[i])->tid; \
                    break; \
                } else { \
                    u_free(thd[i]); \
                } \
            } \
        } \
    } \
    __result; \
})
/// 删除线程
#define ALGO_THREAD_DELETE(tid) \
    ({ \
    int __result = -1; \
    for (int i = 0; i < ALGO_THREAD_MAX_NUM; i++) { \
        if (thd[i] != NULL && thd[i]->tid == tid) { \
            __result = exit_thread_wrapper((thd[i])); \
            u_free(thd[i]); \
            thd[i] = NULL; \
            break; \
        } \
    } \
    __result; \
})
/// 创建定时器
#define ALGO_TIMER_CREATE(timer, name_s, cb, prio, stack_s) \
    os_create_timer_wrapper((timer), (name_s), (cb), (prio), (stack_s))
/// 启动定时器
#define ALGO_TIMER_START(timer, timeout) \
    os_timer_start_wrapper((timer), (timeout))
/// 停止定时器
#define ALGO_TIMER_STOP(timer) \
    os_timer_stop_wrapper((timer))
/// 删除定时器
#define ALGO_TIMER_DELETE(timer) \
    os_timer_delete_wrapper((timer))
/// 创建队列
#define ALGO_QUEUE_CREATE(queue, name_s, queue_size, item_size) \
    os_create_msg_queue_wrapper((queue), (queue_size), (item_size))
/// 发送消息
#define ALGO_QUEUE_SEND(queue, msg, timeout) \
    os_message_put_wrapper((queue), (msg), (timeout))
/// 接收消息
#define ALGO_QUEUE_RECV(queue, msg, timeout) \
    os_message_get_wrapper((queue), (msg), (timeout))
/// 重置队列
#define ALGO_QUEUE_RESET(queue) \
    os_message_reset_wrapper((queue))
/// 删除队列
#define ALGO_QUEUE_DELETE(queue) \
    os_message_delete_wrapper((queue))

#define RT_PRIO_ALGO_DISPATCH_THREAD  OS_PRIORITY_REALTIME4

#else

#define ALGO_MALLOC(size)                 do { return NULL; } while (0)
#define ALGO_FREE(ptr)                    do { ; } while (0)
#define ALGO_SYS_TIME()                   do { return 0; } while (0)

#define ALGO_THREAD_CREATE(name_s, cb, prio, stack_s) \
    ((algo_os_thread_id)NULL)
#define ALGO_THREAD_DELETE(tid) \
    do { ; } while (0)
#define ALGO_TIMER_CREATE(timer, name_s, cb, prio, stack_s) \
    do { ; } while (0)
#define ALGO_TIMER_START(timer, timeout) \
    do { ; } while (0)
#define ALGO_TIMER_STOP(timer) \
    do { ; } while (0)
#define ALGO_TIMER_DELETE(timer) \
    do { ; } while (0)

#define ALGO_QUEUE_CREATE(queue, name_s, queue_size, item_size) \
    do { ; } while (0)
#define ALGO_QUEUE_SEND(queue, msg, timeout) \
    do { ; } while (0)
#define ALGO_QUEUE_RECV(queue, msg, timeout) \
    do { ; } while (0)
#define ALGO_QUEUE_RESET(queue) \
    do { ; } while (0)
#define ALGO_QUEUE_DELETE(queue) \
    do { ; } while (0)

#define RT_PRIO_ALGO_DISPATCH_THREAD  10
#endif

#define IS_SUPPORT_ALGO_LOG                      1

#define ALGO_LOG_SEVERITY_LVL                    4   
#define ALGO_LOG_TAG                             "ALGO_DETECTION"

#define ALGO_LOG_LVL_DEBUG                       4
#define ALGO_LOG_LVL_INFO                        3
#define ALGO_LOG_LVL_WARNING                     2
#define ALGO_LOG_LVL_ERROR                       1

#if (IS_SUPPORT_ROOT_PLATFORM == 1 && IS_SUPPORT_ALGO_LOG == 1)
#include "root_log_define.h"
#if ALGO_LOG_SEVERITY_LVL >= ALGO_LOG_LVL_ERROR
#define ALGO_LOG_ERROR(...)    LOGE(__VA_ARGS__)
#else
#define ALGO_LOG_ERROR(...)
#endif

#if ALGO_LOG_SEVERITY_LVL >= ALGO_LOG_LVL_WARNING
#define ALGO_LOG_WARNING(...)  LOGW(__VA_ARGS__)
#else
#define ALGO_LOG_WARNING(...)
#endif

#if ALGO_LOG_SEVERITY_LVL >= ALGO_LOG_LVL_INFO
#define ALGO_LOG_INFO(...)     LOG_ALGO(__VA_ARGS__)
#else
#define ALGO_LOG_INFO(tag, ...)
#endif

#if ALGO_LOG_SEVERITY_LVL >= ALGO_LOG_LVL_DEBUG
#define ALGO_LOG_DEBUG(...)   LOGD(__VA_ARGS__)
#else
#define ALGO_LOG_DEBUG(tag, ...)
#endif
#elif (IS_SUPPORT_ALGO_LOG == 1)
#define ALGO_LOG_ERROR(...)
#define ALGO_LOG_WARNING(...)
#define ALGO_LOG_INFO(...)
#define ALGO_LOG_DEBUG(...)
#else
#define ALGO_LOG_ERROR(...)
#define ALGO_LOG_WARNING(...)
#define ALGO_LOG_INFO(...)
#define ALGO_LOG_DEBUG(...)
#endif

#ifdef __cplusplus
}
#endif

#endif 
