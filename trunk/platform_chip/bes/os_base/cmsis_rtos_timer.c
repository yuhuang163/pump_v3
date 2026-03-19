/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "hal_timer.h"

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
static enum E_HWTIMER_T hw_timer_start_internal(hw_timer_def_t *hw_timer_def)
{
    if (hw_timer_def == NULL || hw_timer_def->hw_timer_id == NULL) {
        return E_HWTIMER_INVAL_ID;
    }

    return hwtimer_start(hw_timer_def->hw_timer_id, MS_TO_HWTICKS(hw_timer_def->period));
}

static void hw_timer_timeout_handler(void *param)
{
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)param;
    if (hw_timer_def == NULL) {
        return;
    }

    if (hw_timer_def->callback) {
        hw_timer_def->callback(hw_timer_def->user_data);
    }

    if (hw_timer_def->type == OS_TIMER_PERIODIC) {
        if (hw_timer_start_internal(hw_timer_def) != E_HWTIMER_OK) {
            hw_timer_def->is_running = 0;
        }
    } else {
        hw_timer_def->is_running = 0;
    }
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_CMSIS_HW_TIMER 
os_status_e os_timer_start_wrapper(os_timer_id timer_id, uint32_t millisec)
{
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)timer_id;
    if (hw_timer_def == NULL || hw_timer_def->hw_timer_id == NULL) {
        return osError;
    }

    hw_timer_def->period = millisec;
    if (hw_timer_start_internal(hw_timer_def) != E_HWTIMER_OK) {
        return osError;
    }

    hw_timer_def->is_running = 1;
    return osOK;
}

os_status_e os_timer_stop_wrapper(os_timer_id timer_id)
{
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)timer_id;
    if (hw_timer_def == NULL || hw_timer_def->hw_timer_id == NULL) {
        return osError;
    }

    if (hwtimer_stop(hw_timer_def->hw_timer_id) != E_HWTIMER_OK) {
        return osError;
    }
    hw_timer_def->is_running = 0;
    return osOK;
}

os_status_e os_timer_delete_wrapper(os_timer_id timer_id)
{
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)timer_id;
    enum E_HWTIMER_T ret;

    if (hw_timer_def == NULL || hw_timer_def->hw_timer_id == NULL) {
        return osError;
    }

    if (hw_timer_def->is_running > 0) {
        ret = hwtimer_stop(hw_timer_def->hw_timer_id);
        if (ret != E_HWTIMER_OK) {
            return osError;
        }

        hw_timer_def->is_running = 0;
    }

    ret = hwtimer_free(hw_timer_def->hw_timer_id);
    if (ret != E_HWTIMER_OK) {
        return osError;
    }

    hw_timer_def->hw_timer_id = NULL;
    u_free(hw_timer_def);
    return osOK;
}

void *os_timer_get_name_wrapper(os_timer_id timer_id)
{
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)timer_id;
    if (hw_timer_def == NULL || hw_timer_def->hw_timer_id == NULL) {
        return NULL;
    }
    return (void *)hw_timer_def->name;
}

bool os_timer_is_running_wrapper(os_timer_id timer_id)
{
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)timer_id;
    if (hw_timer_def == NULL || hw_timer_def->hw_timer_id == NULL) {
        return false;
    }
    return hw_timer_def->is_running > 0 ? true : false;
}

int os_create_timer_wrapper(os_timer_id *timer_id, const char *name_s, void *user_data, int type, os_ptimer cb)
{
    if (timer_id == NULL) {
        return -1;
    }

    *timer_id = NULL;
    hw_timer_def_t *hw_timer_def = (hw_timer_def_t *)u_malloz(sizeof(hw_timer_def_t));
    if (hw_timer_def == NULL) {
        return -1;
    }

    hw_timer_def->type = type;
    hw_timer_def->callback = cb;
    hw_timer_def->user_data = user_data;
    if (name_s != NULL) {
        strncpy(hw_timer_def->name, name_s, sizeof(hw_timer_def->name) - 1);
    }
    hw_timer_def->name[sizeof(hw_timer_def->name) - 1] = '\0';

    hw_timer_def->hw_timer_id = hwtimer_alloc(hw_timer_timeout_handler, hw_timer_def);
    if (hw_timer_def->hw_timer_id == NULL) {
        LOGE("Oops! hwtimer_alloc(%s): fail", name_s);
        u_free(hw_timer_def);
        return -1;
    }

    *(timer_id) = hw_timer_def;
    return 0;
}
#else
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
#endif