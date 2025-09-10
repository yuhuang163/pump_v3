#ifndef __CMSIS_OS_API_H__
#define __CMSIS_OS_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "cmsis_os_base.h" // IWYU pragma: keep
#include "hal_trace.h"
#include "cmsis.h"

/// log define
#ifndef PRINT_LOG
#define PRINT_LOG(...)  platform_printf(__VA_ARGS__)
#endif

#define IS_SUPPORT_QUEUE           1
#define IS_SUPPORT_TIME            1
#define IS_SUPPORT_MUTEX           1
#define IS_SUPPORT_THREAD          1
#define IS_SUPPORT_SEM_T           1

#define TO_STR(str)                #str

#if IS_SUPPORT_THREAD
/// 创建新线程
#define RUN_NEW_THREAD(thd, name_s, cb, prio, stack_s) \
    run_new_thread_wrapper((thd), (name_s), (cb), (prio), (stack_s))

/// 退出线程
#define EXIT_THREAD(thd) exit_thread_wrapper(thd)
/// 获取线程名称
#define THREAD_NAME_GET(thd) os_thread_get_name_wrapper(thd)
#else
#define RUN_NEW_THREAD(thd, name_s, cb, prio, stack_s) do { ; } while (0)
#define EXIT_THREAD(thd) do { ; } while (0)
#define THREAD_NAME_GET(thd) do { ; } while (0)
#endif

// 消息队列宏定义.
#if IS_SUPPORT_QUEUE
#define CREATE_MSG_TYPE_QUEUE(msg_id_ptr, queue_num, _size) \
    os_create_msg_queue_wrapper(msg_id_ptr, queue_num, _size)
#define MSG_QUEUE_SEND(msg_id_ptr, msg_data, timeout) \
    os_message_put_wrapper(msg_id_ptr, msg_data, timeout)
#define MSG_QUEUE_RECV(msg_id_ptr, msg_data, timeout) \
    os_message_get_wrapper(msg_id_ptr, msg_data, timeout)
#define MSG_QUEUE_RESETRESET(msg_id_ptr) \
    os_message_reset_wrapper(msg_id_ptr)
#define DELETE_MSG_QUEUE(msg_id_ptr) \
    os_message_delete_wrapper(msg_id_ptr)    
#else
#define CREATE_MSG_QUEUE(msg_id_ptr, queue_num) do { ; } while (0)
#define CREATE_MSG_TYPE_QUEUE(msg_id_ptr, queue_num, _size) do { ; } while (0)
#define MSG_QUEUE_SEND(msg_id_ptr, msg_data, timeout) do { ; } while (0)
#define MSG_QUEUE_RECV(msg_id_ptr, msg_data, timeout) do { ; } while (0)
#define MSG_QUEUE_RESETRESET(msg_id_ptr)              do { ; } while (0)
#define DELETE_MSG_QUEUE(msg_id_ptr)                  do { ; } while (0)
#endif

/// 定时器宏定义.
#if IS_SUPPORT_TIME
#define CREATE_TIMER(timer_id_ptr, name_s, user_data, type, cb) \
    os_create_timer_wrapper(timer_id_ptr, name_s, user_data, type, (os_ptimer)cb)
#define START_TIMER(timer_id, period)   os_timer_start_wrapper(timer_id, period)
#define STOP_TIMER(timer_id)            os_timer_stop_wrapper(timer_id)
#define DELETE_TIMER(timer_id)          os_timer_delete_wrapper(timer_id)
#else
#define CREATE_TIMER(timer_id_ptr, name_s, user_data, type, cb)   do { ; } while (0)
#define START_TIMER(timer_id, period)   do { ; } while (0)
#define STOP_TIMER(timer_id)            do { ; } while (0)
#define DELETE_TIMER(timer_id)          do { ; } while (0)
#endif

/// 互斥锁宏定义.
#if IS_SUPPORT_MUTEX
#define MUTEX_INIT_HG(x, name)          os_mutex_create_wrapper((x), TO_STR(name))
#define MUTEX_LOCK_HG(x, timeout)       os_mutex_wait_wrapper((x), timeout)
#define MUTEX_UNLOCK_HG(x)              os_mutex_release_wrapper((x))
#define MUTEX_DELETE_HG(x)              os_mutex_delete_wrapper((x))

#define MUTEX_INIT_HOLD(x, name, max_hold_time_s) \
    do { \
        MUTEX_INIT_HG(x, name); \
        add_to_hang_detect_list(*(x), TO_STR(name), (max_hold_time_s)); \
    } while (0)

#define MUTEX_INIT(x, name)             MUTEX_INIT_HOLD(x, name, 0)
#define MUTEX_LOCK(x)                   MUTEX_LOCK_HG(x, OS_WAIT_FOREVER)
#define MUTEX_UNLOCK(x)                 MUTEX_UNLOCK_HG(x)

#define MUTEX_DELETE(x) \
    do { \
        remove_from_hang_detect_list(x); \
        MUTEX_DELETE_HG(x); \
    } while (0)

#else
#define MUTEX_INIT_HG(x, name)  do { ; } while (0)
#define MUTEX_INIT(x, name)     do { ; } while (0)
#define MUTEX_LOCK(x)           do { ; } while (0)
#define MUTEX_UNLOCK(x)         do { ; } while (0)
#endif

/// 信号量宏定义.
#if IS_SUPPORT_SEM_T
#define U_SEM_INIT(sem_id_p, count) \
    do { \
        const os_semaphore_def_t __def = {0}; \
        *(sem_id_p) = os_semaphore_create_wrapper(&__def, count); \
    } while (0)

#define U_SEM_WAIT(sem_id, timeout)     os_semaphore_wait_wrapper(sem_id, timeout)
#define U_SEM_POST(sem_id)              os_semaphore_release_wrapper(sem_id)
#define U_SEM_DELETE(sem_id)            os_semaphore_delete_wrapper(sem_id)
#else
#define U_SEM_INIT(sem_id_p, count)     do { ; } while (0)
#define U_SEM_WAIT(sem_id, timeout)     do { ; } while (0)
#define U_SEM_POST(sem_id)              do { ; } while (0)
#define U_SEM_DELETE(sem_id)            do { ; } while (0)
#endif

typedef struct RTC_CALENDAR_FORMAT_T utc_time_t;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */

/// 初始化u_heap.
void u_heap_init(void);
/// 获取u_heap的空闲空间大小.
uint32_t u_heap_free_size_get(void);
/// 分配内存.
void *u_malloc(uint32_t size);
/// 分配内存并初始化.
void *u_malloz(uint32_t size);
/// 重新分配内存.
void *u_realloc(void *p1, uint32_t size);
/// 释放内存.
int u_free(void *p);
/// 分配内存并按指定对齐.
void *u_malloc_align(uint32_t size, uint32_t align);
/// 分配内存并按指定对齐并初始化.
void *u_malloz_align(uint32_t size, uint32_t align);
/// 释放对齐的内存.
int u_free_align(void *p);
/// 输出堆信息.
void u_heap_info_output(void);
/// 休眠毫秒.
void msleep(uint32_t time_ms);
/// 早期初始化接口.
void platform_prepare_init(void);
/// 后期初始化接口.
void platform_post_init(void);
/// 平台相关轮询信息打印接口.
void platform_sysinfo_print(void *user_data);
/// 平台提供的获取UTC时间戳接口.
uint32_t platform_get_utc_time(void);
uint64_t platform_get_utc_time_ms(void);
/// 平台提供的设置UTC时间戳接口.
void platform_set_utc_time(uint32_t timestemp, int timezone);
/// 平台提供的获取构建时间接口.
void platform_get_build_time(char *build_time);
/// 平台提供的获取当前时间戳接口.
uint32_t get_timestamp(void);
/// 系统重启.
void platform_system_reboot(void);
/// 平台提供的获取线程ID接口.
int platform_get_tid(void);

#ifdef __cplusplus
}
#endif

#endif 
