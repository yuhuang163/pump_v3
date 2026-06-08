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
#define DUMMY_BROADCAST_TASK_NAME               "dummy_broadcast_task"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(DUMMY_BROADCAST_TASK_NAME); \
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
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_DUMMY, 0);   
    tests_manager_context_t *tests = proc->tests;
    multi_func_msg_data_t msg = {0};
    if (tests && tests->tests_manager_get_current_mode() != TST_NONE_TYPE) {
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        LOG_WM("goto product test mode window.");
    } else {
        msg.union_body.extra.window_id = ID_WINDOW_HOME;
        LOG_WM("goto home window.");
    }
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
    return 0;
}

static void dummy_broadcast_receive_ui_cb(hdr_post_args_t *args)
{
    window_broadcast_args_t *bcd_args = (window_broadcast_args_t *)args->user_data;
    EXCEPTION_CHECK_VOID(bcd_args);
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)bcd_args->context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    switch (bcd_args->code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            LOG_WM("received PM_STATE_CHANGE.");
            if (bcd_args->arg1 != PM_BROADCAST_SYS_RESUME) {
                LOG_WM("ignore: %d\n", bcd_args->arg2);
                break;
            }
            LOG_WM("PM_RESUME, wakeup_reason: %d.", bcd_args->arg2);
            wakup_enter_new_window_policy(proc, bcd_args->arg2);
            break;
        default:
            break;
   }
}

static void on_dummy_window_receive(broadcast_mask_id_t id, void *context,
                                 int32_t arg1, int32_t arg2, broadcast_priv_data_t *pdata)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    proc->broadcast_args.context = proc;
    proc->broadcast_args.code = id.code;
    proc->broadcast_args.arg1 = arg1;
    proc->broadcast_args.arg2 = arg2;
    if (pdata != NULL && pdata->size > 0) {
        memcpy(&proc->broadcast_args.priv_data, pdata, sizeof(broadcast_priv_data_t));
    } else {
        memset(&proc->broadcast_args.priv_data, 0, sizeof(proc->broadcast_args.priv_data));
    }
    POST_WM_TASK_HANDLER_NO_DELAY(dummy_broadcast_receive_ui_cb, &proc->broadcast_args, DUMMY_BROADCAST_TASK_NAME);
}

static int on_dummy_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_dummy_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_DUMMY, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

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
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
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
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
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



