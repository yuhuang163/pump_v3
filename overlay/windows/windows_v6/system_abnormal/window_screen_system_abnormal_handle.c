/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_system_abnormal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                        system_abnormal
#define PAGE_NAME_STR                           "system_abnormal"
#define SYSTEM_ABNORMAL_TASK_NAME               "sys_abnormal_task"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(SYSTEM_ABNORMAL_TASK_NAME); \
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
static int system_abnormal_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_SYSTEM_ABNORMAL, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}


/// 将 sys_abnormal_type_e 映射到 system_abnormal_ui_type_e
static uint16_t map_abnormal_type_to_ui_type(uint8_t abnormal_type)
{
    switch (abnormal_type) {
        case SYS_ABNORMAL_BATTERY_TEMP_HIGH:
            return SYSTEM_ABNORMAL_UI_TYPE_BATTERY_TEMP_HIGH;
        case SYS_ABNORMAL_STORAGE_FAILURE:
            return SYSTEM_ABNORMAL_UI_TYPE_STORAGE_FAILURE;
        case SYS_ABNORMAL_COMMUNICATION_ERROR:
            return SYSTEM_ABNORMAL_UI_TYPE_COMMUNICATION_ERROR;
        default:
            return SYSTEM_ABNORMAL_UI_TYPE_NONE;
    }
}

static void system_abnormal_exit_cb(ui_proc_system_abnormal_t *proc)
{
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    multi_func_msg_data_t msg = {0};
    if (proc->wm->is_window_factory_test_deployed() == 0) {
        LOG_WM("goto factory test screen");
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

#if IS_SUPPORT_THEME_GUIDE_WINDOW
    if (proc->res && proc->res->get_sys_theme_type() == RES_THEME_TYPE_NONE) {
        LOG_WM("goto none theme screen");
        msg.union_body.extra.window_id = ID_WINDOW_THEME_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#endif

    if (proc->battery && proc->battery->is_vbus_plug_in() > 0 &&
        proc->battery->get_charge_status() == CHARGE_STATE_ERROR) {
        LOG_WM("vbus plug in, goto charge screen");
        msg.union_body.extra.window_id = ID_WINDOW_CHARGE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->battery && proc->battery->is_low_battery()) {
        LOG_WM("battery is low power, goto battery screen");
        msg.union_body.extra.window_id = ID_WINDOW_BATTERY;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

    if (proc->airplane && proc->airplane->is_on_airplane_mode()) {
        LOG_WM("goto airplane mode screen");
        msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }

#if IS_SUPPORT_GUIDE_WINDOW
    if (is_need_show_guide_window(proc->wm)) {
        LOG_WM("goto guide screen");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        proc->wm->dispatch_new_window_args(proc->wm, &msg);
        return;
    }
#else
    if (proc->wm->is_window_guidance_deployed() == 0) {
        proc->wm->set_window_guidance_deployed_flag(true);
    }
#endif

    LOG_WM("goto home window.");
    msg.union_body.extra.window_id = ID_WINDOW_HOME;
    proc->wm->dispatch_new_window_args(proc->wm, &msg);
}

static void system_abnormal_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_system_abnormal_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    LOG_WM("system_abnormal_ui_common_cb, margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    switch (action) {
        case SYSTEM_ABNORMAL_HDR_ACTION_EXIT:
            LOG_WM("system abnormal exit.");
            system_abnormal_exit_cb(proc);
            break;
        default:
            LOG_WM("unknown action: %d, ignore.", action);
            break;
    }
}

static void on_system_abnormal_state_change_cb(void *user_data, sys_abnormal_state_info_t *info)
{
    ui_proc_system_abnormal_t *proc = (ui_proc_system_abnormal_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_SYSTEM_ABNORMAL);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }

    if (info == NULL) {
        return;
    }

    if (info->state == SYS_ABNORMAL_STATE_ACTIVE) {
        // 新异常激活，切换显示对应UI
        uint16_t ui_type = map_abnormal_type_to_ui_type(info->type);
        if (ui_type != SYSTEM_ABNORMAL_UI_TYPE_NONE) {
            proc->current_ui_type = ui_type;
            show_system_abnormal_ui(proc);
        }
    } else if (info->state == SYS_ABNORMAL_STATE_CLEARED) {
        // 检查是否所有异常已清除
        if (info->type == SYS_ABNORMAL_TYPE_MAX ||
            (proc->abnormal && !proc->abnormal->is_abnormal_active(info->type))) {
            // 扫描是否还有活动异常
            bool has_active = false;
            if (proc->abnormal) {
                for (uint8_t i = 0; i < SYS_ABNORMAL_TYPE_MAX; i++) {
                    if (proc->abnormal->is_abnormal_active(i)) {
                        has_active = true;
                        break;
                    }
                }
            }
            if (!has_active) {
                LOG_WM("all abnormals cleared, exit window.");
                POST_WM_TASK_HANDLER_NO_DELAY_ARG1(system_abnormal_ui_common_cb, proc,
                    SYSTEM_ABNORMAL_HDR_ACTION_EXIT, SYSTEM_ABNORMAL_TASK_NAME);
            }
        }
    }
}

static int on_system_abnormal_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_system_abnormal_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_SYSTEM_ABNORMAL, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }

    if (keycode != KEY_CODE_POWER) {
        return 0;
    }

    if (keytype == KEY_EVENT_LONG_10S) {
      ///看是否需要重启动作
    }
    return 0;
}

static int entry_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz,
    const struct window_context *old_scr)
{
    ui_proc_system_abnormal_t *proc = CONTAINER_OF(thiz, ui_proc_system_abnormal_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "sys_abnormal_wm", proc, KEY_CODE_POWER,
            KEY_EVENT_LONG_10S, on_system_abnormal_keyevent_hdr);
        ADD_ABNORMAL_STATE_CB(proc->abnormal, proc, on_system_abnormal_state_change_cb);
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        // 关闭按键灯和夜灯
        if (proc->led && proc->led->hook) {
            if (proc->led->hook->set_key_backlight) {
                proc->led->hook->set_key_backlight(proc->led, 0);
            }
#if IS_SUPPORT_NIGHT_LIGHT
            if (proc->led->hook->set_night_light_level) {
                proc->led->hook->set_night_light_level(proc->led, 0);
            }
#endif
        }

        // 根据当前最高优先级活动异常显示对应UI
        if (proc->abnormal) {
            uint16_t ui_type = SYSTEM_ABNORMAL_UI_TYPE_NONE;
            for (uint8_t i = 0; i < SYS_ABNORMAL_TYPE_MAX; i++) {
                if (proc->abnormal->is_abnormal_active(i)) {
                    ui_type = map_abnormal_type_to_ui_type(i);
                    break;
                }
            }
            if (ui_type != SYSTEM_ABNORMAL_UI_TYPE_NONE) {
                proc->current_ui_type = ui_type;
                show_system_abnormal_ui(proc);
            }
        }

        if (proc->pm) {
            proc->pm->acquire_wakelock("sys_abnormal_screen", -1); /// 永久持有锁
        }
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state, const struct window_context *thiz,
    const struct window_context *new_scr)
{
    ui_proc_system_abnormal_t *proc = CONTAINER_OF(thiz, ui_proc_system_abnormal_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_system_abnormal_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            REMOVE_ALL_HANDER_TASK();
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ABNORMAL_STATE_CB(proc->abnormal, on_system_abnormal_state_change_cb);
            REMOVE_KEYEVENT_CB(proc->key, on_system_abnormal_keyevent_hdr);
            if (proc->pm) {
                proc->pm->release_wakelock("sys_abnormal_screen");
            }
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
    ui_proc_system_abnormal_t *proc = u_malloz(sizeof(ui_proc_system_abnormal_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = SYSTEM_ABNORMAL_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->abnormal = (abnormal_manager_context_t *)find_module(ID_MODULE_SYS_ABNORMAL);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    /// 配置窗口上下文
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = system_abnormal_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}
