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
#define BATTERY_UI_KEY_CODE                     (KEY_CODE_POWER | KEY_CODE_START)
#define BATTERY_UI_KEY_TYPE                     (KEY_EVENT_CLICK | KEY_EVENT_UP | KEY_EVENT_LONG_2S)
#define BATTERY_POWER_OFF_HOLD_COMPLETE_TASK_NAME "battery_power_off_hold_complete"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(BATTERY_LOW_POWER_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(BATTERY_POWER_OFF_HOLD_COMPLETE_TASK_NAME); \
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
static int battery_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_battery_t *proc = (ui_proc_battery_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_BATTERY, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}


static int battery_goto_charge_adapter_if_mismatch(ui_proc_battery_t *proc)
{
    EXCEPTION_CHECK_INT(proc);
    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("vbus plug in, goto charge screen");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return 1;
    }
    return 0;
}

static void battery_low_power_exit_cb(ui_proc_battery_t *proc)
{
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_BATTERY);
    multi_func_msg_data_t msg = {0};

    if (battery_goto_charge_adapter_if_mismatch(proc)) {
        return;
    }

    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

#if IS_SUPPORT_GUIDE_WINDOW
    if (is_need_show_guide_window(proc->wm)) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#else
    if (proc->wm->is_window_guidance_deployed() == 0) {
        proc->wm->set_window_guidance_deployed_flag(true);
    }
#endif

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
            if (proc->current_ui_type == BATTERY_UI_TYPE_POWER_OFF_HOLD) {
                LOG_WM("power off hold showing, ignore auto low power off.");
                break;
            }
            LOG_WM("low power power off");
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
            msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_LOW_POWER;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
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

static void battery_low_power_schedule_auto_off(ui_proc_battery_t *proc)
{
    uint8_t battery_state = BATTERY_STATE_NONE;

    EXCEPTION_CHECK_VOID(proc);
    if (proc->battery) {
        battery_state = proc->battery->get_battery_state();
    }
    if (battery_state == BATTERY_STATE_EMPTY) {
        POST_WM_TASK_HANDLER_NO_DELAY_ARG1(battery_ui_common_cb, proc,
            BATTERY_HDR_ACTION_LOW_POWER_POWER_OFF, BATTERY_LOW_POWER_TASK_NAME);
        return;
    }
    POST_WM_TASK_HANDLER_DELAYED_ARG2(battery_ui_common_cb, BATTERY_LOW_POWER_TIMEOUT_MS,
        BATTERY_HDR_ACTION_LOW_POWER_POWER_OFF, proc, BATTERY_LOW_POWER_TASK_NAME);
}

static void on_battery_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_battery_t *proc = (ui_proc_battery_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_BATTERY);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    EXCEPTION_CHECK_VOID(info);

    if (info->charge_status == CHARGE_STATE_ERROR &&
        battery_goto_charge_adapter_if_mismatch(proc)) {
        return;
    }

    if (BATTERY_IS_CHARGING(info->charge_status)) {
        if (proc->current_ui_type == BATTERY_UI_TYPE_LOW_POWER) {
            POST_WM_TASK_HANDLER_NO_DELAY_ARG1(battery_ui_common_cb, proc,
                BATTERY_HDR_ACTION_LOW_POWER_EXIT, BATTERY_LOW_POWER_TASK_NAME);
        }
        return;
    }

    if (info->battery_state == BATTERY_STATE_EMPTY &&
        proc->current_ui_type != BATTERY_UI_TYPE_POWER_OFF_HOLD) {
        LOG_WM("battery empty, goto low power off.");
        POST_WM_TASK_HANDLER_NO_DELAY_ARG1(battery_ui_common_cb, proc,
            BATTERY_HDR_ACTION_LOW_POWER_POWER_OFF, BATTERY_LOW_POWER_TASK_NAME);
    }
}

static int on_battery_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_battery_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_BATTERY, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    if (keytype == KEY_EVENT_LONG_2S &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        battery_power_off_hold_key_long_2s_handler(proc);
        return 0;
    }
    if (keytype == KEY_EVENT_UP &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        battery_power_off_hold_key_up_handler(proc);
        return 0;
    }
    if (keytype == KEY_EVENT_CLICK &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        if (proc->current_ui_type == BATTERY_UI_TYPE_POWER_OFF_HOLD) {
            return 0;
        }
        LOG_WM("click power/start, goto manual power off.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_POWER_OFF;
        msg.union_body.extra.arg1 = PM_POWER_OFF_TYPE_MANUAL;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return 0;
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_battery_t *proc = CONTAINER_OF(thiz, ui_proc_battery_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        ADD_KEYEVENT_CB(proc->key, "battery_wm", proc, BATTERY_UI_KEY_CODE, BATTERY_UI_KEY_TYPE,
            on_battery_ui_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_battery_window_battery_state_change_cb);
        wm_ensure_ble_adv_started();
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (battery_goto_charge_adapter_if_mismatch(proc)) {
            return 0;
        }
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, LED_LEVEL_MAX);
        }
        show_battery_low_power_ui(proc);
        battery_low_power_schedule_auto_off(proc);
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
            battery_power_off_hold_ui_exit(proc);
            stop_battery_low_power_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_BATTERY_STATE_CB(proc->battery, on_battery_window_battery_state_change_cb);
            REMOVE_KEYEVENT_CB(proc->key, on_battery_ui_keyevent_hdr);
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
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    /// 配置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = battery_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

void battery_low_power_auto_off_rearm(ui_proc_battery_t *proc)
{
    battery_low_power_schedule_auto_off(proc);
}



