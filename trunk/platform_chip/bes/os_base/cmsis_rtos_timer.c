/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "cmsis_os_api.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

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
os_status_e os_timer_start_wrapper(os_timer_id timer_id, uint32_t millisec)
{
    return osTimerStart(timer_id, millisec);
}

os_status_e os_timer_stop_wrapper(os_timer_id timer_id)
{
    return osTimerStop(timer_id);
}

os_status_e os_timer_delete_wrapper(os_timer_id timer_id)
{
    return osTimerDelete(timer_id);
}

void *os_timer_get_name_wrapper(os_timer_id timer_id)
{
    return (void *)osTimerGetName(timer_id);
}

bool os_timer_is_running_wrapper(os_timer_id timer_id)
{
    return osTimerIsRunning(timer_id);
}

int os_create_timer_wrapper(os_timer_id *timer_id, const char *name_s, void *user_data, int type, os_ptimer cb)
{
    const os_timer_def_t def = {
        .ptimer = cb,
        .attr = {
            .name = (char *)name_s,
            .attr_bits = 0,
            .cb_mem = NULL,
            .cb_size = 0,
        },
    };

    *(timer_id) = osTimerNew((osTimerFunc_t)def.ptimer, (os_timer_type )type, user_data, &def.attr);
    if (*(timer_id) == NULL) {
        LOGE("Oops! osTimerNew(%s): fail", name_s);
        return -1;
    }
    
    return 0;
}