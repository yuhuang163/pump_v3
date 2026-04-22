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
#define FACTORY_RESET_UI_KEY_CODE                   (KEY_CODE_POWER)
#define FACTORY_RESET_UI_KEY_TYPE                   (KEY_EVENT_CLICK | KEY_EVENT_LONG_2S | KEY_EVENT_UP)
#define FACTORY_RESET_DELAY_TIME                    (1000 * 3)
#define FACTORY_RESET_TASK_NAME                     "factory_reset_task"
#define FACTORY_RESET_EXIT_TASK_NAME                "factory_reset_exit_task"

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
static void factory_reset_exit_task_cb(hdr_post_args_t *args)
{
    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_ID_GUARD_VOID(proc, ID_WINDOW_FACTORY_RESET);
    multi_func_msg_data_t msg = {0};
    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static int on_factory_reset_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_factory_reset_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_FACTORY_RESET, 0);
    if (proc->current_ui_type != FACTORY_RESET_UI_TYPE_FACTORY_RESET_CONFIRM) {
        return 0;
    }

    if (keycode != KEY_CODE_POWER) {
        return 0;
    }

    if (keytype == KEY_EVENT_LONG_2S) {
        proc->display_fac_rst_confirm_progress = true;
    } else if (keytype == KEY_EVENT_CLICK) {
        POST_TASK_HANDLER_NO_DELAY(factory_reset_exit_task_cb, proc,FACTORY_RESET_EXIT_TASK_NAME);
        proc->display_fac_rst_confirm_progress = false;
    } else if (keytype == KEY_EVENT_UP) {
        proc->display_fac_rst_confirm_progress = false;
    }

    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_factory_reset_t *proc = CONTAINER_OF(thiz, ui_proc_factory_reset_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {

    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->extra.arg1 == 0) {
            proc->current_ui_type = FACTORY_RESET_UI_TYPE_FACTORY_RESET;
            factory_reset_handle(proc);
        } else {
            show_factory_reset_confirm_ui(proc);
            if (proc->key) {
                proc->key->add_keyevent_handler("factory_reset_wm", proc, FACTORY_RESET_UI_KEY_CODE, FACTORY_RESET_UI_KEY_TYPE,
                    on_factory_reset_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);
            } else {
                LOG_WM("key not present, ignore.");
            }
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
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_factory_reset_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            if (proc->key) {
                proc->key->remove_keyevent_handler(on_factory_reset_keyevent_hdr);
            } else {
                LOG_WM("key not present, ignore.");
            }
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
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
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

