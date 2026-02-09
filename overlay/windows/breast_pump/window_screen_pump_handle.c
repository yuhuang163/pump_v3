/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager.h"
#include "window_screen_pump.h"// IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                    pump
#define PAGE_NAME_STR                       "pump"
#define PUMP_UI_KEY_CODE                    (KEY_CODE_POWER |KEY_CODE_START | KEY_CODE_MODE | KEY_CODE_FREQUENCY | \
                                            KEY_CODE_BREASTFEEDING | KEY_CODE_LEFT_CONTROL | KEY_CODE_RIGHT_CONTROL | \
                                            KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT)
#define PUMP_UI_KEY_TYPE                    (KEY_EVENT_CLICK)
#define PUMP_START_TASK_NAME                "pump_start"
#define PUMP_STATE_CHANGE_TASK_NAME         "pump_state_change"
#define PUMP_COMPLETE_TASK_NAME             "pump_complete"

#define PUMP_BLINK_PERIOD                    (500)
#define PUMP_BLINK_TIMES                     (1)
#define PUMP_LEVEL_BLINK_TIMES               (3)

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_START_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_STATE_CHANGE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_TASK_NAME); \
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
static void on_breast_pump_result_cb(void *user_data, breast_pump_result_t *result)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
}

static void pump_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    LOG_WM("pump_ui_common_cb, margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    switch (action) {
        case PUMP_HDR_ACTION_START_PUMP: 
            LOG_WM("%s", args->name);
            break;
        case PUMP_HDR_ACTION_CHANGE_PUMP_STATUS: 
            LOG_WM("%s, new_state: %d", args->name, args->arg2);
            if (proc->ui_show_state != args->arg2) {
                proc->ui_show_state = args->arg2;
            }
            break;
        case PUMP_HDR_ACTION_LOOP_SHOW: 
            /// TODO: 轮播显示
            break;
        case PUMP_HDR_ACTION_SHOW_ONCE: 
            LOG_WM("%s, new_state: %d", args->name, args->arg2);
            if (proc->ui_show_state != args->arg2) {
                proc->ui_show_state = args->arg2;
            }
            break;
        case PUMP_HDR_ACTION_SHOW_THEMES_ANI: 
            LOG_WM("%s", args->name);
            /// TODO: 播放主题动画
            break;
        case PUMP_HDR_ACTION_RECOVER_DEFAULT_ANI: 
            LOG_WM("%s: recovery-play..", args->name);
            show_pump_home_ui(proc);
            break;
        case PUMP_HDR_ACTION_PROGRESS_STOP: 
            /// TODO: 停止进度条
            break;
        case PUMP_HDR_ACTION_PUMP_COMPLETE: 
            // REMOVE_ALL_HANDER_TASK();
            LOG_WM("pump complete, show progress time policy.");
            if (proc->pump) {
                show_pump_progress_time_policy(proc, proc->pump);
            }
            break;
        default:
            LOG_WM("unknow action: %d, ignore.", action);
            break;
    }
}

static int on_pump_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_pump_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    EXCEPTION_CHECK_INT(proc->key);
    EXCEPTION_CHECK_INT(proc->ui);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_PUMP, 0);
    breast_pump_info_t pump_info = {
        .status = proc->pump->get_breast_pump_status(),
        .mode = proc->pump->get_breast_pump_mode(),
        .do_work = proc->pump->is_on_working()
    };

    if (proc->goto_charge_ui_flag != 0) {
        LOG_WM("goto charge ui, ignore key event.");
        return 0;
    }

    EXCEPTION_CHECK_INT(proc->led);
#if IS_SUPPORT_AIR_LEAKAGE_WARNING
    if (proc->is_air_leakage_popup_show == true) {
        proc->hide_air_leakage_popup_wm = true;     /// 隐藏空气泄漏弹窗提示.
    }
#endif

    switch (keycode) {
        case KEY_CODE_POWER:
            switch_pump_night_light_ui(proc, proc->pump);
            break;
        case KEY_CODE_START:
            proc->led->set_led_blink(PUMP_START_BL, PUMP_BLINK_PERIOD, 
                PUMP_BLINK_TIMES, LED_STATUS_ON);
            switch_pump_status_ui(proc, proc->pump, pump_info.status);
            break;
        case KEY_CODE_MODE:
            proc->led->set_led_blink(PUMP_MODE_BL, PUMP_BLINK_PERIOD,
                PUMP_BLINK_TIMES, LED_STATUS_ON);
            if (BREAST_PUMP_IS_COMMON_MODE(pump_info.mode) == 0) {
                proc->led->set_led_onoff(PUMP_FREQ_SET_BL, LED_STATUS_ON);
            }
            switch_pump_common_mode_ui(proc, proc->pump, &pump_info);
            break;
        case KEY_CODE_FREQUENCY:
            if (BREAST_PUMP_IS_COMMON_MODE(pump_info.mode)) {
                proc->led->set_led_blink(PUMP_FREQ_SET_BL, PUMP_BLINK_PERIOD, 
                    PUMP_BLINK_TIMES, LED_STATUS_ON);
            }
            switch_pump_frequency_ui(proc, proc->pump, &pump_info);
            break;
        case KEY_CODE_BREASTFEEDING:
            if (BREAST_PUMP_IS_COMMON_MODE(pump_info.mode)) {
                proc->led->set_led_onoff(PUMP_FREQ_SET_BL, LED_STATUS_OFF);
            }
            proc->led->set_led_blink(PUMP_BREASTFEEDING_BL, PUMP_BLINK_PERIOD, 
                PUMP_BLINK_TIMES, LED_STATUS_ON);
            switch_breastfeeding_mode_ui(proc, proc->pump, &pump_info);
            break;
        case KEY_CODE_LEFT_CONTROL:
            proc->led->set_led_blink(PUMP_LEFT_CONTROL_BL, PUMP_BLINK_PERIOD, 
                PUMP_LEVEL_BLINK_TIMES, LED_STATUS_ON);
            proc->led->set_led_level(PUMP_RIGHT_CONTROL_BL, LED_LEVEL_MAX / 2);
            switch_pump_control_type_ui(proc, proc->pump, 0);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            proc->led->set_led_blink(PUMP_RIGHT_CONTROL_BL, PUMP_BLINK_PERIOD, 
                PUMP_LEVEL_BLINK_TIMES, LED_STATUS_ON);
            proc->led->set_led_level(PUMP_LEFT_CONTROL_BL, LED_LEVEL_MAX / 2);
            switch_pump_control_type_ui(proc, proc->pump, 1);
            break;
        case KEY_CODE_KNOB_LEFT:
            switch_pump_level_ui(proc, proc->pump, &pump_info, 1);
            break;
        case KEY_CODE_KNOB_RIGHT:
            switch_pump_level_ui(proc, proc->pump, &pump_info, 0);
            break;
        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }

    return 0;
}

static void on_pump_ui_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    EXCEPTION_CHECK_VOID(proc->pump);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    /// 获取泵奶状态.
    int pump_status = proc->pump->get_breast_pump_status();
    int pump_mode = proc->pump->get_breast_pump_mode();
    int pump_working = proc->pump->is_on_working();
    /// 如果用户暂停，忽略.
    if (proc->user_pause != 0) {
        LOG_WM("user_pause ignore.");
        return;
    }
    LOG_WM("code: %lx, code_v2: %lx, flag: %d, status: %d, mode: %ld, working: %d, arg1: %d, arg2: %d.",
         id.code, id.code_v2, proc->is_mode_changed_by_device, pump_status, pump_mode, pump_working, arg1, arg2);
    switch (id.code) {
        case ID_BROADCAST_BREAST_PUMP_STATE_CHANGE: 
            if (arg1 == BREAST_PUMP_BROADCAST_MODE_CHANGE) {
                if (!proc->is_mode_changed_by_device) {
                    if (pump_working && PUMP_IS_GOING(pump_status)) {
                        REMOVE_ALL_HANDER_TASK();
                        LOG_WM("start pump state change task after %ds", PUMP_MODE_CHANGE_TIMEOUT);
                        POST_TASK_HANDLER_DELAYED_ARG5(pump_ui_common_cb, PUMP_MODE_CHANGE_TIMEOUT,
                                PUMP_HDR_ACTION_CHANGE_PUMP_STATUS, LOOP_SHOW_PUMPING, proc, PUMP_STATE_CHANGE_TASK_NAME);
                    }
                    proc->ui_show_state = LOOP_SHOW_SWITCH_MODE;
                    proc->is_mode_changed_by_app = true;  /// 监测APP端切换模式动作，同步手柄端
                    if (proc->pm) {
                        proc->pm->pm_wakeup(PM_WAKEUP_REASON_BLE_REQ); /// 接收到APP事件之后仅刷新进入休眠时间
                    }
                } else {
                    proc->is_mode_changed_by_device = false;
                    proc->is_mode_changed_by_app = false;
                }
            } else if (arg1 == BREAST_PUMP_BROADCAST_COMPLETED) {
                POST_TASK_HANDLER_DELAYED_ARG2(pump_ui_common_cb, 100, PUMP_HDR_ACTION_PUMP_COMPLETE, 
                    proc, PUMP_COMPLETE_TASK_NAME);
                LOG_WM("pump finish, post pump complete task.");
            }
            break;
        default:
            break;
    }
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_pump_t *proc = CONTAINER_OF(thiz, ui_proc_pump_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    /// 获取泵奶控制类型
    proc->info.control_type = proc->pump->get_breast_pump_control_type();
    /// 处理窗口进入事件.
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        if (proc->key) {
            proc->key->add_keyevent_handler("pump_wm", proc, PUMP_UI_KEY_CODE, PUMP_UI_KEY_TYPE,
                 on_pump_ui_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);
        } else {
            LOG_WM("key not present, ignore.");
        }
        if (proc->need_start_pump) {
            proc->need_start_pump = 0;
            POST_TASK_HANDLER_DELAYED_ARG2(pump_ui_common_cb, 100, PUMP_HDR_ACTION_START_PUMP, proc,
                 PUMP_START_TASK_NAME);
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        show_pump_home_ui(proc);
    }
    return 0;
}

static void remove_breast_pump_result_cb(breast_pump_manager_context_t *pump)
{
    EXCEPTION_CHECK_VOID(pump);
    int status = pump->get_breast_pump_status();
    if (status > BREAST_PUMP_IDLE && status < BREAST_PUMP_FINISH) {
        LOG_WM("pump status: %d => set_breast_pump_status:FINISH", status);
        pump->set_breast_pump_status(BREAST_PUMP_FINISH);
    }
    LOG_WM("==>remove breast_pump_result_listener");
    pump->remove_breast_pump_result_listener(on_breast_pump_result_cb);
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_pump_t *proc = CONTAINER_OF(thiz, ui_proc_pump_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->user_pause = 1;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_pump_ui_animation(proc); 
            unregister_broadcast_receiver(&proc->receiver);  // 注销广播接收器，防止use-after-free
            REMOVE_ALL_HANDER_TASK();
            remove_breast_pump_result_cb(proc->pump);
            EXCEPTION_CHECK_INT(proc->key);
            proc->key->remove_keyevent_handler(on_pump_ui_keyevent_hdr);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
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
    ui_proc_pump_t *proc = u_malloz(sizeof(ui_proc_pump_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    /// 设置窗口标识.
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = PUMP_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "pump_screen";
    proc->receiver.id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_PERSIST_INFO_UPDATE;
    proc->receiver.receiver_cb = on_pump_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    /// 配置泵奶管理器.
    if (proc->pump) {
        proc->pump->set_breast_pump_status(BREAST_PUMP_IDLE);
        proc->pump->add_breast_pump_result_listener(proc, on_breast_pump_result_cb);
    }
    /// 设置窗口上下文.
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

