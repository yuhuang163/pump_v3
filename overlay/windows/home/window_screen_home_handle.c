/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            home
#define PAGE_NAME_STR                               "home"
#define HOME_UI_KEY_CODE                            KEY_CODE_START
#define HOME_UI_KEY_TYPE                            KEY_EVENT_CLICK

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
static int on_home_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return 0;
    }
    ui_proc_home_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    switch (keycode) {
        case KEY_CODE_START:
            home_start_key_event_handler(proc);
            break;
        default:
            break;
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_home_t *proc = CONTAINER_OF(thiz, ui_proc_home_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        if (proc->key) {
            proc->key->add_keyevent_handler("home_wm", proc, HOME_UI_KEY_CODE, HOME_UI_KEY_TYPE,
                on_home_ui_keyevent_hdr);
        } else {
            LOG_WM("key not present, ignore.");
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        ui->animation_play_status_update(ui, false); /// 清除动画播放标志
        start_home_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_home_t *proc = CONTAINER_OF(thiz, ui_proc_home_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_home_ui_animation(proc);
            if (proc->key) {
                proc->key->remove_keyevent_handler(on_home_ui_keyevent_hdr);
            }
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            u_free(proc);
            proc = NULL;
            SET_WINDOW_CONTEXT(NULL);
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
    ui_proc_home_t *proc = u_malloz(sizeof(ui_proc_home_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = HOME_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    wm->dispatch_new_window(w_ctx);
    return ID_WINDOW_DISPLAY_OK;
}

