/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager.h" // IWYU pragma: keep
#include "input_key_code.h"
#include "window_screen_pump.h"// IWYU pragma: keep
#include "windows_manager.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                    pump
#define PAGE_NAME_STR                       "pump"
#define PUMP_UI_KEY_CODE                    (ALL_KEY_CODE_NO_COMBINE)
#define PUMP_UI_KEY_TYPE                    (KEY_EVENT_CLICK | KEY_EVENT_LONG_2S | KEY_EVENT_UP)
#define PUMP_COMPLETE_TASK_NAME             "pump_complete"
#define PUMP_BROADCAST_TASK_NAME            "pump_broadcast"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BROADCAST_TASK_NAME); \
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
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
}

static void pump_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    int action = args->arg1;
    switch (action) {
        case PUMP_HDR_ACTION_PUMP_COMPLETE:        
            LOG_WM("pump complete, goto home window.");
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_HOME;
            msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
            break;
        default:
            LOG_WM("unknow action: %d, ignore.", action);
            break;
    }
}

static void pump_knob_event_cb(ui_proc_pump_t *proc, uint8_t is_left)
{
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER) {
        LOG_WM("pump finish timeout, ignore knob event.");
        return;
    }

#if (IS_ALLLOW_PUMP_COMPLETE_BREAK == 0)
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        LOG_WM("pump will finish, ignore knob event.");
        return;
    }
#endif
    
    switch (proc->current_ui_type) {
        case PUMP_UI_TYPE_LEVEL:
            switch_pump_level_ui(proc, is_left ? false : true);
            break;
        case PUMP_UI_TYPE_BOTH_LEVEL:
            switch_pump_both_level_ui(proc, is_left ? false : true);
            break;
        case PUMP_UI_TYPE_HOME:
            switch_pump_both_control_type_ui(proc, is_left ? false : true);
            break;
        default:
            LOG_WM("unknow ui type: %d, ignore.", proc->current_ui_type);
            break;
    }
}

static void pump_long_press_event_cb(ui_proc_pump_t *proc, uint16_t keycode)
{
    switch (keycode) {
        case KEY_CODE_START:
            if (proc->current_ui_type == PUMP_UI_TYPE_HOME) {
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_MANUAL);
            }
            break;
        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }
}

static void pump_click_event_cb(ui_proc_pump_t *proc, uint16_t keycode)
{
    if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_TIMEOUT ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR ||
        proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_LOW_POWER) {
        LOG_WM("pump finish timeout, ignore key event.");
        return;
    }

#if (IS_ALLLOW_PUMP_COMPLETE_BREAK == 0)
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        LOG_WM("pump result ui, ignore key event.");
        return;
    }
#endif

    switch (keycode) {
        case KEY_CODE_MODE:
            pump_mode_key_click_handler(proc);
            break;
        case KEY_CODE_BREASTFEEDING:
            #if IS_SUPPORT_PUMP_RHYTHM_MODE_SELECT
            breastfeeding_mode_key_click_handler(proc);
            #endif
            break;
        case KEY_CODE_CANCEL:
            pump_cancel_key_handler(proc);
            break;
        case KEY_CODE_POWER:
            pump_power_key_handler(proc);
            break;
        case KEY_CODE_START:
            switch_pump_status_ui(proc);
            break;
        case KEY_CODE_RIGHT_CONTROL:
            switch_pump_control_type_ui(proc, BREAST_PUMP_CONTROL_TYPE_RIGHT);
            break;
        case KEY_CODE_LEFT_CONTROL:
            switch_pump_control_type_ui(proc, BREAST_PUMP_CONTROL_TYPE_LEFT);
            break;
        case KEY_CODE_KNOB_LEFT:
        case KEY_CODE_KNOB_RIGHT:
            pump_knob_event_cb(proc, keycode == KEY_CODE_KNOB_LEFT);
            break;
        default:
            LOG_WM("unknow keycode: %d, ignore.", keycode);
            break;
    }
}

static int on_pump_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_pump_t *proc = user_data;
    WINDOW_CURRENT_ID_GUARD_INT(proc, ID_WINDOW_PUMP, 0);
    EXCEPTION_CHECK_INT(proc->pump);

    #if IS_SUPPORT_AIR_LEAKAGE_WARNING
    if (proc->is_air_leakage_popup_show == true) {
        proc->hide_air_leakage_popup_wm = true;     /// 隐藏空气泄漏弹窗提示.
    }
    #endif

    if (keytype == KEY_EVENT_LONG_2S) {
        pump_long_press_event_cb(proc, keycode);
    } else {
        if (proc->popup_ui_type == ID_POPUP_UI_PUMP_FINISH_MANUAL) {
            int pump_status = 0;
            READ_PUMP_WORK_STATUS(proc->pump, pump_status);
            if (pump_status == BREAST_PUMP_PAUSE) {
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_PAUSE_MANUAL);
            } else {
                pump_popup_ui_type_update(proc, ID_POPUP_UI_NONE);
            }
        }

        if (keytype == KEY_EVENT_CLICK) {
            pump_click_event_cb(proc, keycode);
        }
    }
    return 0;
}

static void pump_broadcast_receive_ui_cb(hdr_post_args_t *args)
{
    window_broadcast_args_t *bcd_args = (window_broadcast_args_t *)args->user_data;
    EXCEPTION_CHECK_VOID(bcd_args);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)bcd_args->context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    EXCEPTION_CHECK_VOID(proc->pump);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    int pump_mode = 0;
    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    LOG_WM("code:%lx, arg1:%d, arg2:%d, status:%d, mode:%d",
            bcd_args->code, bcd_args->arg1, bcd_args->arg2, pump_status, pump_mode);
    switch (bcd_args->code) {
        case ID_BROADCAST_BREAST_PUMP_STATE_CHANGE: {
            int state = bcd_args->arg1;
            if (state == BREAST_PUMP_BROADCAST_COMPLETED) {
                if (proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
                    proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
                    proc->pump_will_finish && PUMP_NO_FINISH(pump_status)) {
                    multi_func_msg_data_t msg = {0};
                    msg.union_body.extra.window_id = ID_WINDOW_HOME;
                    msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
                    proc->wm->dispatch_new_window_args(proc->wm, &msg);
                    LOG_WM("pump finish, goto home window.");
                }
            } else if (state == BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH) {
                proc->pump_will_finish = true;
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_TIMEOUT);
                LOG_WM("pump will finish, post pump will finish task.");
            } else if (state == BREAST_PUMP_BROADCAST_APP_COMPLETED) {
                multi_func_msg_data_t msg = {0};
                msg.union_body.extra.window_id = ID_WINDOW_HOME;
                msg.union_body.extra.arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT;
                proc->wm->dispatch_new_window_args(proc->wm, &msg);
                LOG_WM("pump app complete, post pump app complete task.");
            } else if (state == BREAST_PUMP_BROADCAST_PUMP_ERROR) {
                LOG_WM("pump error, post pump error task.");
            } else if (state == BREAST_PUMP_BROADCAST_AUTO_PAUSE_REQUEST) {
                pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_PAUSE_TIMEOUT);
                LOG_WM("pump pause timeout, post pump pause timeout task.");
            }
            break;
        }
        default:
            break;
    }
}

static void on_pump_ui_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
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
    POST_WM_TASK_HANDLER_NO_DELAY(pump_broadcast_receive_ui_cb, &proc->broadcast_args, PUMP_BROADCAST_TASK_NAME);
}

static void remove_breast_pump_result_cb(ui_proc_pump_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status > BREAST_PUMP_IDLE && pump_status < BREAST_PUMP_FINISH) {
        LOG_WM("next wm:%d pump status:%d => goto finish", proc->wm->get_next_window_id(), pump_status);
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
    } 
    LOG_WM("==>remove breast_pump_result_listener");
    proc->pump->remove_breast_pump_result_listener(on_breast_pump_result_cb);
}

static void on_pump_window_battery_state_change_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PUMP);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    
    if (info->charge_status == CHARGE_STATE_ERROR) {
        LOG_WM("charge status error");
        if (proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_MANUAL) {
            pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR);
            return;
        }
    }

#if IS_SUPPORT_LOW_POWER_AUTO_STOP_PUMP
    if (info->battery_state == BATTERY_STATE_EMPTY || 
        info->battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN || 
        info->battery_state == BATTERY_STATE_FORBID_START) {
        if (proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_TIMEOUT &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_CHARGE_ERROR &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_LOW_POWER &&
            proc->popup_ui_type != ID_POPUP_UI_PUMP_FINISH_MANUAL) {
            pump_popup_ui_type_update(proc, ID_POPUP_UI_PUMP_FINISH_LOW_POWER);
            return;
        }
    }
#endif
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_pump_t *proc = CONTAINER_OF(thiz, ui_proc_pump_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    /// 获取泵奶控制类型
    int control_type = 0;
    READ_PUMP_LEVEL_CONTROL_TYPE(proc->pump, control_type);
    proc->info.control_type = control_type;
    /// 处理窗口进入事件.
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "pump_wm", proc, PUMP_UI_KEY_CODE, PUMP_UI_KEY_TYPE,
            on_pump_ui_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_pump_window_battery_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        proc->current_ui_type = PUMP_UI_TYPE_NONE;
        int mode_index = 0;
        READ_PUMP_WORK_MODE(proc->pump, mode_index);
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 100);
        }
        switch_pump_status_ui(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_pump_t *proc = CONTAINER_OF(thiz, ui_proc_pump_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_pump_ui_animation(proc); 
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            unregister_broadcast_receiver(&proc->receiver);
            remove_breast_pump_result_cb(proc);
            REMOVE_KEYEVENT_CB(proc->key, on_pump_ui_keyevent_hdr);
            REMOVE_BATTERY_STATE_CB(proc->battery, on_pump_window_battery_state_change_cb);
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
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    LOG_WM("window entry extra arg1: %d.", extra->arg1);
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
    proc->receiver.receiver_cb = on_pump_ui_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    /// 配置泵奶管理器.
    if (proc->pump) {
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_IDLE, 1);
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

void pump_complete_event_handler(ui_proc_pump_t *proc)
{
    proc->pump_will_finish = false;
    POST_WM_TASK_HANDLER_NO_DELAY_ARG1(pump_ui_common_cb, proc, PUMP_HDR_ACTION_PUMP_COMPLETE,
        PUMP_COMPLETE_TASK_NAME);
}