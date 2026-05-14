/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_dummy.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                        dummy
#define PAGE_NAME_STR                           "dummy"
#define EXIT_DUMMY_POLICY_TASK_NAME             "exit_dummy_policy"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(EXIT_DUMMY_POLICY_TASK_NAME); \
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
static int wakup_enter_new_window_policy(ui_proc_dummy_t *proc, int wakeup_reason)
{
    tests_manager_context_t *tests = proc->tests;
    EXCEPTION_CHECK_INT(tests);
    EXCEPTION_CHECK_INT(proc->pm);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_DUMMY, 0);
    multi_func_msg_data_t msg = {0};
    if (tests->tests_manager_get_current_mode() != TST_NONE_TYPE) {
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        LOG_WM("goto product test mode window.");
    } else {
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        LOG_WM("goto home window.");
    }
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
    return 0;
}

static void on_dummy_window_receive(broadcast_mask_id_t id, void *context,
                                 int32_t pm_state, int32_t wakeup_reason, broadcast_priv_data_t *pdata)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            LOG_WM("received PM_STATE_CHANGE.");
            if (pm_state != PM_BROADCAST_SYS_RESUME) {
                LOG_WM("ignore: %d\n", wakeup_reason);
                break;
            }
            LOG_WM("PM_RESUME, wakeup_reason: %d.", wakeup_reason);
            wakup_enter_new_window_policy(proc, wakeup_reason);
            break;
        default:
            break;
    }
}

static int on_dummy_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_dummy_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_DUMMY, 0);
    LOG_WM("dummy key event: %d, %d", keycode, keytype);
    if (keytype == KEY_EVENT_LONG_10S) {
        msleep(1000);
        platform_system_reboot();
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_dummy_t *proc = CONTAINER_OF(thiz, ui_proc_dummy_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_INT(ui);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "dummy_wm", proc, (KEY_CODE_POWER), KEY_EVENT_LONG_10S,
            on_dummy_keyevent_hdr);
        proc->enter_dummy_time = platform_get_utc_time();
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state, 
    const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_dummy_t *proc = CONTAINER_OF(thiz, ui_proc_dummy_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            REMOVE_KEYEVENT_CB(proc->key, on_dummy_keyevent_hdr);
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
    ui_proc_dummy_t *proc = u_malloz(sizeof(ui_proc_dummy_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = DUMMY_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->tests = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    /// 监听PM状态改变.
    proc->receiver.id.code = ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.context = proc;
    proc->receiver.name = "dummy_screen";
    proc->receiver.receiver_cb = on_dummy_window_receive;
    register_broadcast_receiver(&proc->receiver);
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



