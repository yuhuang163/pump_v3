/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "handler_post.h"
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
#define HOME_UI_KEY_CODE                            (KEY_CODE_MODE | KEY_CODE_BREASTFEEDING | KEY_CODE_START | KEY_CODE_POWER)
#define HOME_UI_KEY_TYPE                            KEY_EVENT_CLICK
#define HOME_IDLE_UI_START_TASK_NAME                "home_idle_ui_start_task"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(HOME_IDLE_UI_START_TASK_NAME); \
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
static void home_ui_switch_cb(hdr_post_args_t *args)
{
    ui_proc_home_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_HOME);
    if (args->arg1 == HOME_UI_TYPE_HOME_IDLE) {
        start_home_idle_ui_animation(proc);
    } else if (args->arg1 == HOME_UI_TYPE_HOME) {
        start_home_ui_animation(proc);
    }
}

static int on_home_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_home_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_HOME, 0);
    switch (keycode) {
        case KEY_CODE_POWER:
            if (proc->led && proc->led->hook && proc->led->hook->toggle_night_light) {
                proc->led->hook->toggle_night_light(proc->led);
            }
            break;
        case KEY_CODE_MODE:
        case KEY_CODE_BREASTFEEDING:
        case KEY_CODE_START:
            if (proc->wm) {
                multi_func_msg_data_t msg = {0};
                LOG_WM("goto pump window.");
                msg.union_body.extra.window_id = ID_WINDOW_PUMP;
                msg.union_body.extra.arg1 = keycode;
                proc->wm->dispatch_new_window_args(proc->wm, &msg);
            }
            break;
        default:
            break;
    }
    return 0;
}

static void on_home_ui_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
    broadcast_priv_data_t *pdata)
{
   ui_proc_home_t *proc = (ui_proc_home_t *)context;
   EXCEPTION_CHECK_VOID(proc);
   WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_HOME);
   switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE: 
            LOG_WM("PM_STATE_CHANGE: %d", arg1);
            if (arg1 == PM_BROADCAST_SYS_IDLE) {
                POST_TASK_HANDLER_NO_DELAY_ARG1(home_ui_switch_cb, proc, HOME_UI_TYPE_HOME_IDLE, HOME_IDLE_UI_START_TASK_NAME);
            } else if (arg1 == PM_BROADCAST_SYS_IDLE_RELEASE) {
                POST_TASK_HANDLER_DELAYED_ARG2(home_ui_switch_cb, 100, HOME_UI_TYPE_HOME,
                    proc, HOME_IDLE_UI_START_TASK_NAME);
            }
            break;
        default:
            break;
   }
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
                on_home_ui_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);
        } else {
            LOG_WM("key not present, ignore.");
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 100);
        }
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
            stop_home_common_ui(proc);
            REMOVE_ALL_HANDER_TASK();
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
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "home_screen";
    proc->receiver.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.receiver_cb = on_home_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    wm->dispatch_new_window(w_ctx);
    
    return ID_WINDOW_DISPLAY_OK;
}

