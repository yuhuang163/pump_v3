/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            guide
#define PAGE_NAME_STR                               "guide"
#define GUIDE_UI_KEY_CODE                           ALL_KEY_CODE_NO_COMBINE
#define GUIDE_UI_KEY_TYPE                           (KEY_EVENT_CLICK)

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
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
static int on_guide_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_guide_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_GUIDE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    if (keycode == KEY_CODE_KNOB_LEFT) {
        guide_knob_event_handler(proc, true);
    } else if (keycode == KEY_CODE_KNOB_RIGHT) {
        guide_knob_event_handler(proc, false);
    } else {
        guide_key_event_handler(proc, keycode);
    }
    return 0;
}

static void on_guide_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_guide_t *proc = (ui_proc_guide_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_GUIDE);
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
    ui_proc_guide_t *proc = CONTAINER_OF(thiz, ui_proc_guide_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        ADD_KEYEVENT_CB(proc->key, "guide_wm", proc, GUIDE_UI_KEY_CODE, GUIDE_UI_KEY_TYPE,
            on_guide_ui_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_guide_window_battery_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 0);
            proc->led->set_led_level(PUMP_FREQ_SET_BL, LED_LEVEL_MAX);
        }
        proc->page_index = GUIDE_PAGE_PUMP_LEVEL_DOUBLE_ADJUST;
        proc->mode_selected = 0;
        proc->last_adjust_time = get_timestamp();
        proc->setting.level[0] = BREAST_PUMP_LEVEL_1;
        proc->setting.level[1] = BREAST_PUMP_LEVEL_1;
        proc->setting.speed = BREAST_PUMP_SPEED_MID;
        if (proc->pump) {
            WRITE_PUMP_SETTING_INFO(proc->pump, 0, true, proc->setting);
        }
        start_guide_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_guide_t *proc = CONTAINER_OF(thiz, ui_proc_guide_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_guide_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_KEYEVENT_CB(proc->key, on_guide_ui_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_guide_window_battery_state_change_cb);
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
window_enter_result_e PAGE_HANDLE_NAME(PAGE_STATIC_NAME)(window_manager_context_t *wm, uint8_t window_id)
{
    LOG_WM("%s window enter, time: %ld.", PAGE_NAME_STR, get_timestamp());
    ui_proc_guide_t *proc = u_malloz(sizeof(ui_proc_guide_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    wm->dispatch_new_window(w_ctx);
    
    return ID_WINDOW_DISPLAY_OK;
}

