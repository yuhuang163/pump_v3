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
static void popup_exit_handler_cb(hdr_post_args_t *args)
{
    ui_proc_popup_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->wm);
    EXCEPTION_CHECK_VOID(proc->pm);
    proc->pm->release_wakelock("popup-windows");
    if (proc->extra.popup_exit_cb) {
        proc->extra.popup_exit_cb(proc->extra.user_data, 0, 0);
    } else {
        LOG_UI("GO_BACK_TO_HISTORY");
        window_entry_extra_t extra = {0};
        proc->wm->go_back_to_history(proc->wm, NULL);
    }
}

static int on_popup_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return 0;
    }
    ui_proc_popup_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc->wm);
    EXCEPTION_CHECK_INT(proc->pm);
    window_entry_extra_t extra = {0};
    proc->pm->release_wakelock("popup-windows");
    proc->wm->go_back_to_history(proc->wm, &extra);

    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_popup_t *proc = CONTAINER_OF(thiz, ui_proc_popup_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {

    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        ui->animation_play_status_update(ui, false); /// 清除动画播放标志
        start_logo_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_popup_t *proc = CONTAINER_OF(thiz, ui_proc_popup_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            // stop_pop_ui_animation(proc);
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
    ui_proc_popup_t *proc = u_malloz(sizeof(ui_proc_popup_t));
    SET_WINDOW_CONTEXT(proc);
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    memcpy(&proc->extra, extra, sizeof(window_entry_extra_t));
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = POPUP_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);

    if (extra->popup_bg_res_index < ID_POPUP_WINDOW_MAX) {
        // proc->bg_res_index = extra->popup_bg_res_index;
    }

    if (extra->popup_exit_time < 500) {
        extra->popup_exit_time = 2000; /// 限制min退出时间长度
    }
    
    if (proc->pm) {
        proc->pm->acquire_wakelock("popup-windows", extra->popup_exit_time / 1000 + 3); /// 锁住popup这段时间内不休眠.
    }

    // proc->which_type = extra->popup_bg_which ? 1 : 0;

    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

