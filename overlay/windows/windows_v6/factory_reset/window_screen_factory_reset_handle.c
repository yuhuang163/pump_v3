/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            factory_reset
#define PAGE_NAME_STR                               "factory_reset"
/** 含 L+R 组合键，短按可跳转 home 对应功能页 */
#define FACTORY_RESET_UI_KEY_CODE                   (ALL_KEY_CODE_NO_COMBINE | KEY_CODE_BOTH_CONTROL)
#define FACTORY_RESET_UI_KEY_TYPE                   (KEY_EVENT_DOWN | KEY_EVENT_UP | KEY_EVENT_CLICK)
#define FACTORY_RESET_EXIT_TASK_NAME                "factory_reset_exit_task"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(FACTORY_RESET_EXIT_TASK_NAME); \
    factory_reset_confirm_ui_hdr_remove(); \
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
static int factory_reset_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_FACTORY_RESET, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}

static void factory_reset_exit_task_cb(hdr_post_args_t *args)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    multi_func_msg_data_t msg = {0};

#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (proc->res && proc->res->get_sys_theme_type() == RES_THEME_TYPE_NONE) {
        LOG_WM("goto none theme screen");
        msg.union_body.extra.window_id = ID_WINDOW_THEME_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#endif

    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

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

#if IS_SUPPORT_GUIDE_WINDOW
    if (proc->wm->is_window_guidance_deployed() == 0) {
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

static void factory_reset_goto_home_sub_ui(ui_proc_factory_reset_t *proc, int sub_ui, int arg2)
{
    multi_func_msg_data_t msg = {0};

    proc->popup_ui_type = ID_POPUP_UI_NONE;
    factory_reset_confirm_ui_hdr_remove();
    LOG_WM("factory reset confirm goto home sub_ui:%d arg2:%d", sub_ui, arg2);
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    msg.union_body.extra.arg1 = sub_ui;
    msg.union_body.extra.arg2 = arg2;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void factory_power_key_event_handler(ui_proc_factory_reset_t *proc, uint16_t keytype)
{
    if (keytype == KEY_EVENT_DOWN) {
        proc->popup_ui_type = ID_POPUP_UI_FACTORY_RESET_CONFIRM;
        proc->last_progress_ani_time = get_timestamp();
    } else if (keytype == KEY_EVENT_UP) {
        if (proc->popup_ui_type != ID_POPUP_UI_NONE) {
            proc->popup_ui_type = ID_POPUP_UI_NONE;
        }
        /// 松手未完成确认，重新计空闲超时
        proc->last_progress_ani_time = get_timestamp();
    }
}

static void factory_function_key_click_handler(ui_proc_factory_reset_t *proc, uint16_t keycode)
{
    switch (keycode) {
        case KEY_CODE_LEFT_CONTROL:
            /// arg2: 0=左, 1=右（与 home_level_adjust_side_e 一致）
            factory_reset_goto_home_sub_ui(proc, HOME_SUB_UI_TYPE_LEVEL_ADJUST, 0);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            factory_reset_goto_home_sub_ui(proc, HOME_SUB_UI_TYPE_LEVEL_ADJUST, 1);
            break;
        case KEY_CODE_BOTH_CONTROL:
            factory_reset_goto_home_sub_ui(proc, HOME_SUB_UI_TYPE_BOTH_LEVEL_ADJUST, 0);
            break;
        case KEY_CODE_STIMULATION_MODE:
            factory_reset_goto_home_sub_ui(proc, HOME_SUB_UI_TYPE_MODE_SET,
                BREAST_PUMP_STIMULATION_MODE);
            break;
        case KEY_CODE_LACTATION_MODE:
            factory_reset_goto_home_sub_ui(proc, HOME_SUB_UI_TYPE_MODE_SET,
                BREAST_PUMP_LACTATION_MODE);
            break;
        case KEY_CODE_START:
            factory_reset_request_exit(proc);
            break;
        case KEY_CODE_FREQUENCY:
            factory_reset_goto_home_sub_ui(proc, HOME_SUB_UI_TYPE_SPEED_ADJUST, 0);
            break;
        default:
            break;
    }
}

static int on_factory_reset_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_factory_reset_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_FACTORY_RESET, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    
    if (proc->current_ui_type != FACTORY_RESET_UI_TYPE_FACTORY_RESET_CONFIRM) {
        return 0;
    }

    if (keycode == KEY_CODE_POWER) {
        factory_power_key_event_handler(proc, keytype);
        return 0;
    }

    if (keytype != KEY_EVENT_CLICK) {
        return 0;
    }

    factory_function_key_click_handler(proc, keycode);
    return 0;
}

static void on_factory_reset_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }

    if (info->charge_status == CHARGE_STATE_ERROR) {
        LOG_WM("charge status error, goto charge screen.");
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
    }
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_factory_reset_t *proc = CONTAINER_OF(thiz, ui_proc_factory_reset_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
        if (proc->extra.arg1 != 0) {
            ADD_KEYEVENT_CB(proc->key, "factory_reset_wm", proc, FACTORY_RESET_UI_KEY_CODE, FACTORY_RESET_UI_KEY_TYPE,
                on_factory_reset_keyevent_hdr);
        } 
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_factory_reset_window_battery_state_change_cb);
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);

        if (proc->extra.arg1 == 0) {
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }
            factory_reset_handle(proc, 1000);
        } else {
            /// 确认页：所有按键灯常亮
            factory_reset_confirm_led_restore(proc);
            show_factory_reset_confirm_ui(proc);
        }
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_factory_reset_t *proc = CONTAINER_OF(thiz, ui_proc_factory_reset_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_factory_reset_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_KEYEVENT_CB(proc->key, on_factory_reset_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_factory_reset_window_battery_state_change_cb);
            u_free(proc);
            proc = NULL;
            LOG_WM("%s window exit--end", PAGE_NAME_STR);
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
void factory_reset_request_exit(ui_proc_factory_reset_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);
    proc->popup_ui_type = ID_POPUP_UI_NONE;
    factory_reset_confirm_ui_hdr_remove();
    POST_WM_TASK_HANDLER_NO_DELAY(factory_reset_exit_task_cb, proc, FACTORY_RESET_EXIT_TASK_NAME);
}

window_enter_result_e PAGE_HANDLE_NAME(PAGE_STATIC_NAME)(window_manager_context_t *wm, uint8_t window_id)
{
    LOG_WM("%s window enter, time: %ld.", PAGE_NAME_STR, get_timestamp());
    ui_proc_factory_reset_t *proc = u_malloz(sizeof(ui_proc_factory_reset_t));
    SET_WINDOW_CONTEXT(proc);
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    memcpy(&proc->extra, extra, sizeof(window_entry_extra_t));
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = FACTORY_RESET_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);

    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = factory_reset_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

