/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "windows_manager.h"
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_theme_guide.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            theme_guide
#define PAGE_NAME_STR                               "theme_guide"
#define THEME_GUIDE_UI_KEY_CODE                     (KEY_CODE_KNOB_RIGHT | KEY_CODE_KNOB_LEFT | KEY_CODE_START)
#define THEME_GUIDE_UI_KEY_TYPE                     KEY_EVENT_CLICK

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
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int theme_guide_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_theme_guide_t *proc = (ui_proc_theme_guide_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_THEME_GUIDE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}

static void theme_guide_knob_event_handler(ui_proc_theme_guide_t *proc, bool is_left)
{
    proc->is_operated = 1;

    if (is_left) {
        if (proc->selected_index > THEME_GUIDE_SELECT_SIMPLE) {
            proc->selected_index--;
        }
    } else if (proc->selected_index + 1 < THEME_GUIDE_SELECT_MAX) {
        proc->selected_index++;
    }
}

static int on_theme_guide_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    (void)keytype;
    ui_proc_theme_guide_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_THEME_GUIDE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    if (keycode == KEY_CODE_KNOB_LEFT) {
        theme_guide_knob_event_handler(proc, true);
    } else if (keycode == KEY_CODE_KNOB_RIGHT) {
        theme_guide_knob_event_handler(proc, false);
    } else if (keycode == KEY_CODE_START) {
        if (proc->popup_ui_type == ID_POPUP_UI_THEME_RESELECT) {
            return 0;
        }
        proc->popup_ui_type = ID_POPUP_UI_THEME_RESELECT;
    }
    return 0;
}

static void on_theme_guide_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_theme_guide_t *proc = (ui_proc_theme_guide_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_THEME_GUIDE);
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
    ui_proc_theme_guide_t *proc = CONTAINER_OF(thiz, ui_proc_theme_guide_t, w_ctx);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        ADD_KEYEVENT_CB(proc->key, "theme_guide_wm", proc, THEME_GUIDE_UI_KEY_CODE, THEME_GUIDE_UI_KEY_TYPE,
            on_theme_guide_ui_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_theme_guide_window_battery_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->led) {
            if (proc->led->hook && proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }
            proc->led->set_led_level(PUMP_START_BL, LED_LEVEL_MAX);
        }
        start_theme_guide_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_theme_guide_t *proc = CONTAINER_OF(thiz, ui_proc_theme_guide_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_theme_guide_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_KEYEVENT_CB(proc->key, on_theme_guide_ui_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_theme_guide_window_battery_state_change_cb);
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
    ui_proc_theme_guide_t *proc = u_malloz(sizeof(ui_proc_theme_guide_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = THEME_GUIDE_UI_TYPE_NONE;
    proc->selected_index = THEME_GUIDE_SELECT_SIMPLE;
    proc->is_operated = 0;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = theme_guide_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}
