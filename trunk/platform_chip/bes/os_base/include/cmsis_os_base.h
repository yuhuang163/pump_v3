#ifndef __CMSIS_OS_BASE_H__
#define __CMSIS_OS_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "hwtimer_list.h"

#define OS_WAIT_FOREVER  osWaitForever     ///< wait forever timeout value

typedef enum {
  OS_TIMER_ONCE               = 0,        ///< One-shot timer.
  OS_TIMER_PERIODIC           = 1         ///< Repeating timer.
} os_timer_type_e;

#define IS_SUPPORT_CMSIS_HW_TIMER            (0)

typedef struct {
    HWTIMER_ID hw_timer_id;
    uint32_t period;
    int is_running;
    os_timer_type_e type;
    os_ptimer callback;
    void *user_data;
    char name[16];
} hw_timer_def_t;

typedef osThreadId_t os_thread_id;
typedef osMessageQId os_message_id;
#if IS_SUPPORT_CMSIS_HW_TIMER
typedef hw_timer_def_t *os_timer_id;
#else
typedef osTimerId_t os_timer_id;
#endif
typedef osMutexId_t os_mutex_id;
typedef osSemaphoreId_t os_semaphore_id;
typedef osThreadDef_t os_thread_def_t;
typedef osTimerDef_t os_timer_def_t;
typedef osMessageQDef_t os_message_def_t;
typedef osSemaphoreAttr_t os_semaphore_def_t;
typedef osMutexAttr_t os_mutex_def_t;
typedef osStatus os_status_e;

// Priority used for thread control.
typedef enum  {
  OS_PRIORITY_IDLE           = 0,          ///< priority: idle (lowest)
  OS_PRIORITY_LOW            = 1,          ///< priority: low
  OS_PRIORITY_BELOW_NORMAL   = 2,          ///< priority: below normal
  OS_PRIORITY_NORMAL         = 3,          ///< priority: normal (default)
  OS_PRIORITY_ABOVE_NORMAL   = 4,          ///< priority: above normal
  OS_PRIORITY_HIGH           = 5,          ///< priority: high
  OS_PRIORITY_REALTIME       = 6,          ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME1      = 7,          ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME2      = 8,          ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME3      = 9,          ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME4      = 10,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME5      = 11,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME6      = 12,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME7      = 13,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME8      = 14,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME9      = 15,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME10     = 16,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME11     = 17,         ///< priority: realtime (highest)
  OS_PRIORITY_REALTIME12     = 18,         ///< priority: realtime (highest)
  OS_PRIORITY_ERROR         =  0x84        ///< system cannot determine priority or thread has illegal priority
} os_priority_e;

typedef enum {
	OS_THREAD_RUNNING   = 0x0,	      /* A thread is querying the state of itself, so must be running. */
	OS_THREAD_READY     = 0x1 ,			        /* The thread being queried is in a read or pending ready list. */
	OS_THREAD_BLOCKED   = 0x2,		        /* The thread being queried is in the Blocked state. */
	OS_THREAD_SUSPENDED = 0x3,	      /* The thread being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
	OS_THREAD_DELETED   = 0x4,		          /* The thread being queried has been deleted, but its TCB has not yet been freed. */   
  OS_THREAD_ERROR     = 0x7FFFFFFF
} os_thread_state_e;

// 操作系统线程
typedef struct thread_context {
  os_thread_id tid;
  os_message_id msg_id;
  os_mutex_id lock_id;
  os_thread_def_t *p_thd;
  uint32_t is_running;
  void *user_data;
  char *task_name;
  uint8_t exclusive_task;
} thread_context_t;

bool is_interrupt_handler_wrapper(void);
os_status_e os_kernel_start_wrapper (void);
bool os_kernel_running_wrapper(void);

int run_new_thread_wrapper(thread_context_t *thd, const char *name_s, void *cb, int prio, int stack_s);
int exit_thread_wrapper(thread_context_t *thd);
os_thread_id os_thread_get_id_wrapper (void);
char *os_thread_get_name_wrapper(thread_context_t *thd);
os_status_e os_thread_set_priority_wrapper (os_thread_id thread_id, os_priority_e priority);
os_priority_e os_thread_get_priority_wrapper (os_thread_id thread_id);
os_status_e os_thread_yield_wrapper (void);
bool os_thread_is_suspended_wrapper(os_thread_id thread_id);
os_status_e os_thread_suspend_wrapper (os_thread_id thread_id);
os_status_e os_thread_resume_wrapper (os_thread_id thread_id);
os_thread_state_e os_thread_get_state_wrapper(os_thread_id thread_id);

int os_create_timer_wrapper(os_timer_id *timer_id, const char *name_s, void *user_data, int type, os_ptimer cb);
os_status_e os_timer_start_wrapper (os_timer_id timer_id, uint32_t millisec);
os_status_e os_timer_stop_wrapper (os_timer_id timer_id);
os_status_e os_timer_delete_wrapper (os_timer_id timer_id);
void *os_timer_get_name_wrapper (os_timer_id timer_id);
bool os_timer_is_running_wrapper (os_timer_id timer_id);

int os_create_msg_queue_wrapper(os_message_id *msg_ptr, int queue_num, int item_size);
os_status_e os_message_put_wrapper (os_message_id queue_id, void *msg, uint32_t millisec);
os_status_e os_message_get_wrapper (os_message_id queue_id, void *msg, uint32_t millisec);
os_status_e os_message_delete_wrapper (os_message_id queue_id);
os_status_e os_message_reset_wrapper (os_message_id queue_id);

os_semaphore_id os_semaphore_create_wrapper (const os_semaphore_def_t *semaphore_def, int32_t count);
os_status_e os_semaphore_wait_wrapper (os_semaphore_id semaphore_id, uint32_t millisec);
os_status_e os_semaphore_release_wrapper (os_semaphore_id semaphore_id);
os_status_e os_semaphore_delete_wrapper (os_semaphore_id semaphore_id);

int os_mutex_module_init(void);
int os_mutex_create_wrapper(os_mutex_id *mutex_id, const char *name);
os_status_e os_mutex_wait_wrapper (os_mutex_id mutex_id, uint32_t millisec);
os_status_e os_mutex_release_wrapper (os_mutex_id mutex_id);
os_status_e os_mutex_delete_wrapper (os_mutex_id mutex_id); 

void platform_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif 
