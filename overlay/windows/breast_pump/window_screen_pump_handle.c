/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager_api.h"
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
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }
}

static void pump_ui_common_cb(hdr_post_args_t *args)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }
    ui_proc_pump_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    LOG_WM("pump_ui_common_cb, margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    switch (action) {
        case HDR_ACTION_START_PUMP: {
            LOG_WM("%s", args->name);
        }
        break;
        
        case HDR_ACTION_CHANGE_PUMP_STATUS: {
            int new_state = args->arg2;
            LOG_WM("%s, new_state: %d", args->name, new_state);
            if (proc->ui_show_state != new_state) {
                proc->ui_show_state = new_state;
            }
        }
        break;
        
        case HDR_ACTION_LOOP_SHOW: {
            /// TODO: 轮播显示
        }
        break;
        
        case HDR_ACTION_SHOW_ONCE: {
            int new_state = args->arg2;
            LOG_WM("%s, new_state: %d", args->name, new_state);
            if (proc->ui_show_state != new_state) {
                proc->ui_show_state = new_state;
            }
        }
        break;

        case HDR_ACTION_SHOW_THEMES_ANI: {
            LOG_WM("%s", args->name);
            /// TODO: 播放主题动画
        }
        break;

        case HDR_ACTION_RECOVER_DEFAULT_ANI: {
            LOG_WM("%s: recovery-play..", args->name);
            show_pump_home_ui(proc);
        }
        break;

        case HDR_ACTION_PROGRESS_STOP: {
            /// TODO: 停止进度条
        }
        break;

        case HDR_ACTION_PUMP_COMPLETE: {
            // REMOVE_ALL_HANDER_TASK();
            LOG_WM("pump complete, show progress time policy.");
            show_pump_progress_time_policy(proc, proc->wm->priv.pump);
        }
        break;

        default:
        break;
    }
}

static int on_pump_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return 0;
    }
    ui_proc_pump_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc);
    breast_pump_manager_context_t *pump = proc->wm->priv.pump;
    EXCEPTION_CHECK_INT(pump);
    key_input_context_t *key_ctx = proc->wm->priv.key;
    EXCEPTION_CHECK_INT(key_ctx);
    breast_pump_info_t pump_info = {
        .status = pump->get_breast_pump_status(),
        .mode = pump->get_breast_pump_mode(),
        .do_work = pump->is_on_working()
    };
    const char *key_name = NULL;
    if (key_ctx && key_ctx->hook && key_ctx->hook->get_key_code_name) {
        key_name = key_ctx->hook->get_key_code_name(keycode);
    } else {
        key_name = "@unknow";
    }
    LOG_WM("%s, keyevent:%d/%d, pump working: %d.", key_name, keycode, keytype, pump_info.do_work);

    if (proc->goto_charge_ui_flag != 0) {
        LOG_WM("goto charge ui, ignore key event.");
        return 0;
    }

    switch (keycode) {
        case KEY_CODE_POWER:
            switch_pump_power_ui(proc, pump);
            break;
        case KEY_CODE_START:
            switch_pump_status_ui(proc, pump, pump_info.status);
            break;
        case KEY_CODE_MODE:
            switch_pump_common_mode_ui(proc, pump, &pump_info);
            break;
        case KEY_CODE_FREQUENCY:
            switch_pump_frequency_ui(proc, pump, &pump_info);
            break;
        case KEY_CODE_BREASTFEEDING:
            switch_pump_breastfeeding_mode_ui(proc, pump, &pump_info);
            break;
        case KEY_CODE_LEFT_CONTROL:
            switch_pump_control_type_ui(proc, pump, 0);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            switch_pump_control_type_ui(proc, pump, 1);
            break;
        case KEY_CODE_KNOB_LEFT:
            switch_pump_level_ui(proc, pump, &pump_info, 1);
            break;
        case KEY_CODE_KNOB_RIGHT:
            switch_pump_level_ui(proc, pump, &pump_info, 0);
            break;

        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }
    LOG_WM("keyevent#EXIT: %d: %d", keycode, keytype);

    return 0;
}

static void on_pump_ui_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }

    ui_proc_pump_t *proc = (ui_proc_pump_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    breast_pump_manager_context_t *pump = proc->wm->priv.pump;
    EXCEPTION_CHECK_VOID(pump);
    /// 获取泵奶状态.
    int pump_status = pump->get_breast_pump_status();
    int pump_mode = pump->get_breast_pump_mode();
    int pump_working = pump->is_on_working();
    /// 如果用户暂停，忽略.
    if (proc->user_pause != 0) {
        LOG_WM("user_pause ignore.");
        return;
    }
    LOG_WM("receive broadcast, code: %lx, code_v2: %lx, flag: %d, status: %d, mode: %ld,\
            working: %d, arg1: %d, arg2: %d.", id.code, id.code_v2, proc->is_mode_changed_by_device,
            pump_status, pump_mode, pump_working, arg1, arg2);
    switch (id.code) {
        case ID_BROADCAST_BREAST_PUMP_MODE_CHANGE: {
            /// 监测APP端切换模式动作，同步手柄端
            if (!proc->is_mode_changed_by_device) {
                /// 泵奶中切换模式，4秒后进入计时界面
                if (pump_working && PUMP_IS_GOING(pump_status)) {
                    REMOVE_ALL_HANDER_TASK();
                    LOG_WM("start pump state change task after %ds", PUMP_MODE_CHANGE_TIMEOUT);
                    POST_TASK_HANDLER_DELAYED_ARG5(pump_ui_common_cb, PUMP_MODE_CHANGE_TIMEOUT,
                            HDR_ACTION_CHANGE_PUMP_STATUS, LOOP_SHOW_PUMPING, proc, PUMP_STATE_CHANGE_TASK_NAME);
                }
                proc->ui_show_state = LOOP_SHOW_SWITCH_MODE;
                proc->is_mode_changed_by_app = true;
                /// 接收到APP事件之后仅刷新进入休眠时间
                proc->wm->priv.pm->pm_wakeup(PM_WAKEUP_REASON_BLE_REQ);
            } else {
                proc->is_mode_changed_by_device = false;
                proc->is_mode_changed_by_app = false;
            }
            break;
        }
        case ID_BROADCAST_BREAST_PUMP_FINISH: {
            POST_TASK_HANDLER_DELAYED_ARG2(pump_ui_common_cb, 100, HDR_ACTION_PUMP_COMPLETE, 
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
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_INT(ui);
    struct storage_manager_context *st = wm->priv.st;
    EXCEPTION_CHECK_INT(st);
    /// 获取泵奶控制类型
    if (st) {
        st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &proc->info.control_type);
    }
    /// 处理窗口进入事件.
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        if (wm->priv.key) {
            wm->priv.key->add_keyevent_handler("pump_wm", proc, PUMP_UI_KEY_CODE, PUMP_UI_KEY_TYPE,
                 on_pump_ui_keyevent_hdr);
        } else {
            LOG_WM("key not present, ignore.");
        }
        if (proc->need_start_pump) {
            proc->need_start_pump = 0;
            POST_TASK_HANDLER_DELAYED_ARG2(pump_ui_common_cb, 100, HDR_ACTION_START_PUMP, proc,
                 PUMP_START_TASK_NAME);
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        ui->animation_play_status_update(ui, false); /// 清除动画播放标志
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
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE: {
            proc->user_pause = 1;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_pump_ui_animation(proc); 
            unregister_broadcast_receiver(&proc->receiver);  // 注销广播接收器，防止use-after-free
            REMOVE_ALL_HANDER_TASK();
            remove_breast_pump_result_cb(wm->priv.pump);
            EXCEPTION_CHECK_INT(wm->priv.key);
            wm->priv.key->remove_keyevent_handler(on_pump_ui_keyevent_hdr);
        }
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
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "pump_screen";
    proc->receiver.id.code |= ID_BROADCAST_BREAST_PUMP_MODE_CHANGE;
    proc->receiver.id.code |= ID_BROADCAST_SYS_PERSIST_INFO_SAVE;
    proc->receiver.id.code |= ID_BROADCAST_BREAST_PUMP_FINISH;
    proc->receiver.receiver_cb = on_pump_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    /// 配置泵奶管理器.
    if (wm->priv.pump) {
        wm->priv.pump->set_breast_pump_status(BREAST_PUMP_IDLE);
        wm->priv.pump->add_breast_pump_result_listener(proc, on_breast_pump_result_cb);
    }
    /// 设置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    wm->dispatch_new_window(w_ctx);
    return ID_WINDOW_DISPLAY_OK;
}

