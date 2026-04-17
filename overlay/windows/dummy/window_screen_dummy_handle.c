/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "input_key_code.h"
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
#define DUMMY_TIME_INTERVAL                     (60000 * 1) /// 1分钟
#define DUMMY_IDLE_POWER_OFF_TIME               (60000 * 5) /// 5分钟
#define DUMMY_NIGHT_LIGHT_ONOFF_TASK_NAME       "dummy_night_light_onoff"
#define DUMMY_NIGHT_LIGHT_LEVEL_TASK_NAME       "dummy_night_light_level"
#define DUMMY_SYSTEM_POWER_OFF_TASK_NAME        "dummy_system_power_off"
#define EXIT_DUMMY_POLICY_TASK_NAME             "exit_dummy_policy"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(DUMMY_NIGHT_LIGHT_ONOFF_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(DUMMY_NIGHT_LIGHT_LEVEL_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(DUMMY_SYSTEM_POWER_OFF_TASK_NAME); \
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
static void system_power_off_cb(hdr_post_args_t *args)
{
    ui_proc_dummy_t *proc = (ui_proc_dummy_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pm);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_DUMMY);
    uint32_t current_time = platform_get_utc_time();
    if (current_time - proc->enter_dummy_time > DUMMY_IDLE_POWER_OFF_TIME) {
        LOG_WM("system power off");
        SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_STATE_CHANGE, SYSTEM_BROADCAST_POWER_OFF);
        return;
    }
    POST_TASK_HANDLER_DELAYED_ARG1(system_power_off_cb, DUMMY_TIME_INTERVAL, proc, 
        DUMMY_SYSTEM_POWER_OFF_TASK_NAME);
    LOG_WM("dummy time: %d", current_time - proc->enter_dummy_time);
}

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
        case ID_BROADCAST_BATTERY_STATE_CHANGE:
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
        if (proc->key) {
            proc->key->add_keyevent_handler("dummy_wm", proc, KEY_CODE_1, KEY_EVENT_CLICK,
                 on_dummy_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);
        } else {
            LOG_WM("key not present, ignore.");
        }
        proc->enter_dummy_time = platform_get_utc_time();
        POST_TASK_HANDLER_DELAYED_ARG1(system_power_off_cb, DUMMY_TIME_INTERVAL, proc, DUMMY_SYSTEM_POWER_OFF_TASK_NAME);
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
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            if (proc->key) {
                proc->key->remove_keyevent_handler(on_dummy_keyevent_hdr);
            } else {
                LOG_WM("key not present, ignore.");
            }
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
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
    proc->receiver.id.code |= ID_BROADCAST_BATTERY_STATE_CHANGE;
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



