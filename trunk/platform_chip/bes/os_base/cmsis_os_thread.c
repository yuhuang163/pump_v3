/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint32_t s_thd_total_statck_size = 0;
static uint8_t s_thd_total_num = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
os_status_e os_kernel_start (void)
{
    return osKernelStart();
}

bool os_kernel_running(void)
{
    return osKernelGetState() == osKernelRunning ? 1 : 0;
}

os_thread_id os_thread_create_wrapper(const os_thread_def_t *thread_def, void *argument)
{
    return osThreadNew((osThreadFunc_t)thread_def->pthread, argument, &thread_def->attr);
}

os_thread_id os_thread_get_id_wrapper (void)
{
    return osThreadGetId();
}

os_status_e os_thread_yield_wrapper(void)
{
    return osThreadYield();
}

os_status_e os_thread_set_priority_wrapper(os_thread_id thread_id, os_priority_e priority)
{
    return osThreadSetPriority(thread_id, (osPriority_t)priority);
}

os_priority_e os_thread_get_priority_wrapper(os_thread_id thread_id)
{
    return (os_priority_e)osThreadGetPriority(thread_id);
}

os_status_e os_delay_wrapper(uint32_t millisec)
{
    return osDelay(millisec);
}

os_thread_state_e os_thread_get_state_wrapper(os_thread_id thread_id)
{
    return (os_thread_state_e)osThreadGetState(thread_id);
}

bool os_thread_is_suspended_wrapper(os_thread_id thread_id)
{
    return os_thread_get_state_wrapper(thread_id) == OS_THREAD_SUSPENDED ? true : false;
}

os_status_e os_thread_suspend_wrapper(os_thread_id thread_id)
{
    return osThreadSuspend(thread_id);
}

os_status_e os_thread_resume_wrapper(os_thread_id thread_id)
{
    return osThreadResume(thread_id);
}

int run_new_thread_wrapper(thread_context_t *thd, const char *name_s, void *cb, int prio, int stack_s)
{
    thd->p_thd = (os_thread_def_t *)u_malloz(sizeof(os_thread_def_t));
    if (thd->p_thd == NULL) {
        LOGE("Oops! u_malloz: fail %s", "thd->p_thd");
        return -1;
    }

    uint8_t *stack_mem = u_malloz_align(stack_s, 8);
    if (stack_mem == NULL) {
        LOGE("Oops! u_malloz: fail %s", "stack_mem");
        u_free(thd->p_thd);
        return -1;
    }

    thd->p_thd->pthread = (os_pthread)cb;
    thd->p_thd->attr = (osThreadAttr_t){
        .name = name_s,
        .attr_bits = osThreadDetached,
        .cb_mem = NULL,
        .cb_size = 0U,
        .stack_mem = stack_mem,
        .stack_size = stack_s,
        .priority = (osPriority_t)prio,
        .tz_module = 1U,                  /// indicate calls to secure mode
        .reserved = 0U,
    };

    thd->tid = osThreadNew((osThreadFunc_t)thd->p_thd->pthread, thd, &thd->p_thd->attr);
    if (thd->tid == NULL) {
        u_free(thd->p_thd);
        u_free_align(thd->p_thd->attr.stack_mem);
        LOGE("Oops! osThreadNew: fail %s", "thd->tid");
        return -1;
    }
    return 0;
}
 
int exit_thread_wrapper(thread_context_t *thd)
{
    os_thread_id id = thd->tid;
    LOGI("exit_thread_wrapper: %p, %p", thd, id);
    u_free(thd->p_thd);
    u_free_align(thd->p_thd->attr.stack_mem);
    os_status_e status = osThreadTerminate(id);
    if (status != osOK) {
        LOGE("Oops! osThreadTerminate: fail %s", "status");
    }
    return status;
}
