/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_popup.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            popup_window
#define PAGE_NAME_STR                               "popup_window"
#define POPUP_UI_KEY_CODE                           (0xFFFF)         /// 任意按键
#define POPUP_UI_KEY_TYPE                           (KEY_EVENT_CLICK) /// 仅响应 click（不影响其它长按/特殊键）
#define IS_SUPPORT_POPUP_WAKE_LOCK                  (0)
#define POPUP_WAKE_LOCK_NAME                        "popup-windows"

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
static int on_popup_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_popup_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc && proc->pm);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_POPUP_WINDOW, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    window_entry_extra_t extra = {0};
#if IS_SUPPORT_POPUP_WAKE_LOCK
    proc->pm->release_wakelock(POPUP_WAKE_LOCK_NAME);
#endif
    if (proc->extra.forbid_short_press_exit_popup) {
        return 0;
    }

    if (proc->wm->close_popup_overlay) {
        proc->wm->close_popup_overlay(proc->wm);
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_popup_t *proc = CONTAINER_OF(thiz, ui_proc_popup_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        ADD_KEYEVENT_CB(proc->key, "popup_wm", proc, POPUP_UI_KEY_CODE, POPUP_UI_KEY_TYPE,
            on_popup_keyevent_hdr);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        start_popup_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_popup_t *proc = CONTAINER_OF(thiz, ui_proc_popup_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_popup_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            REMOVE_KEYEVENT_CB(proc->key, on_popup_keyevent_hdr);
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
    ui_proc_popup_t *proc = u_malloz(sizeof(ui_proc_popup_t));
    SET_WINDOW_CONTEXT(proc);
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    memcpy(&proc->extra, extra, sizeof(window_entry_extra_t));
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->popup_window_id = extra->arg1;
    proc->current_ui_type = POPUP_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    if (proc->extra.popup_exit_time < 500 || proc->extra.popup_exit_time > 10000) {
        proc->extra.popup_exit_time = 2000; /// 限制最小退出时间长度
    }
#if IS_SUPPORT_POPUP_WAKE_LOCK
    if (proc->pm) {
        proc->pm->acquire_wakelock(POPUP_WAKE_LOCK_NAME, proc->extra.popup_exit_time / 1000 + 3); /// 锁住popup这段时间内不休眠.
    }
#endif
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1; /// 不推入历史栈.
    w_ctx->is_popup_window = 1; /// 标记为弹出框窗口.
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

