/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_battery.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                        battery
#define PAGE_NAME_STR                           "battery"
#define BATTERY_LOW_POWER_TASK_NAME             "battery_low_power_task"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(BATTERY_LOW_POWER_TASK_NAME); \
} while (0)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
INIT_WINDOW_CONTEXT();

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

static void battery_low_power_exit_cb(ui_proc_battery_t *proc)
{
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_BATTERY);
    multi_func_msg_data_t msg = {0};
    if (proc->wm->is_window_factory_test_deployed() == 0) {
        LOG_WM("goto factory test screen");
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("vbus plug in, goto charge screen");
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->wm->is_window_guidance_deployed() == 0) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }    
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void battery_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_battery_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_BATTERY);
    LOG_WM("battery_ui_common_cb, margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    switch (action) {
        case BATTERY_HDR_ACTION_LOW_POWER_POWER_OFF: 
            LOG_WM("low power power off");
            if (proc->pm) {
                proc->pm->system_power_control(false);
            }
            break;
        case BATTERY_HDR_ACTION_LOW_POWER_EXIT: 
            LOG_WM("low power exit, goto exit screen.");
            battery_low_power_exit_cb(proc);
            break;
        default:
            LOG_WM("unknow action: %d, ignore.", action);
            break;
    }
}

static void on_battery_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_battery_t *proc = (ui_proc_battery_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_BATTERY);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    
    if (BATTERY_IS_CHARGING(info->charge_status)) {
        if (proc->current_ui_type == BATTERY_UI_TYPE_LOW_POWER) {
            POST_WM_TASK_HANDLER_NO_DELAY_ARG1(battery_ui_common_cb, proc,
                BATTERY_HDR_ACTION_LOW_POWER_EXIT, BATTERY_LOW_POWER_TASK_NAME);
        }
    }
}

static int entry_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_battery_t *proc = CONTAINER_OF(thiz, ui_proc_battery_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_battery_window_battery_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        show_battery_low_power_ui(proc);
        POST_WM_TASK_HANDLER_DELAYED_ARG2(battery_ui_common_cb, 3000, BATTERY_HDR_ACTION_LOW_POWER_POWER_OFF,
                proc, BATTERY_LOW_POWER_TASK_NAME);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_battery_t *proc = CONTAINER_OF(thiz, ui_proc_battery_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE: 
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_battery_low_power_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_BATTERY_STATE_CB(proc->battery, on_battery_window_battery_state_change_cb);
            u_free(proc);
            proc = NULL;
            LOG_WM("%s window on destory--end", PAGE_NAME_STR);
            break;
        default:
            break;
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
window_enter_result_e PAGE_HANDLE_NAME(PAGE_STATIC_NAME)(window_manager_context_t *wm, uint8_t window_id)
{
    LOG_WM("%s window enter, time: %ld.", PAGE_NAME_STR, get_timestamp());
    ui_proc_battery_t *proc = u_malloz(sizeof(ui_proc_battery_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = BATTERY_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = NULL;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}



