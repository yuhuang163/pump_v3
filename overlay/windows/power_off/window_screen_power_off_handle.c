/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_power_off.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            power_off
#define PAGE_NAME_STR                               "power_off"
#define POWER_OFF_PAGE_POLICY_TASK_NAME             "power_off_page_policy"

#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(POWER_OFF_PAGE_POLICY_TASK_NAME);\
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
#if IS_SUPPORT_POWER_OFF_WHEN_CHARGING
static void power_off_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_power_off_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc && proc->wm);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_POWER_OFF);
    LOG_WM("margic:%x, action:%d.", proc->margic, args->arg1);
    int action = args->arg1;
    multi_func_msg_data_t msg = {0};
    switch (action) {
        case POWER_OFF_HDR_EXIT_CHARGE_POLICY:
            LOG_WM("%s", args->name);
            if (proc->ble) {
                proc->ble->bt_adv_start_by_user();
            }
            LOG_WM("goto logo window.");
            if (proc->ui->hook && proc->ui->hook->set_ui_backlight_level_rate) {
                proc->ui->hook->set_ui_backlight_level_rate(proc->ui, 100);
            }
            msg.union_body.extra.window_id = ID_WINDOW_LOGO;
            proc->wm->dispatch_new_window_args(proc->wm, &msg);
            break;
        case POWER_OFF_HDR_DISCHARGE_POWER_OFF:
            LOG_WM("%s", args->name);
            power_off_handle(proc);
            break;
        default:
            break;
    }
}

static void on_power_off_window_battery_state_cb(void *user_data, battery_state_info_t *info)
{
    ui_proc_power_off_t *proc = (ui_proc_power_off_t *)user_data;
    EXCEPTION_CHECK_VOID(proc && info);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_POWER_OFF);
    LOG_WM("charge status: %d, show charge adapter ui", info->charge_status);
    if (proc->current_ui_type != POWER_OFF_UI_TYPE_CHARGING && 
        proc->current_ui_type != POWER_OFF_UI_TYPE_CHARGE_ERROR) {
        return;
    }

    if (BATTERY_IS_CHARGING(info->charge_status)) {
        show_power_off_charging_ui(proc);
    } else if(info->charge_status == CHARGE_STATE_ERROR) {
        show_power_off_charge_error_ui(proc);
    } else {
        power_off_handle(proc);
    } 
}

static int on_power_off_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_power_off_t *proc = user_data;
    EXCEPTION_CHECK_INT(proc && proc->battery);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_POWER_OFF, 0);
    if (keycode != KEY_CODE_POWER) {
        return 0;
    }

    if (proc->current_ui_type != POWER_OFF_UI_TYPE_CHARGING) {
        return 0;
    }

    if (keytype == KEY_EVENT_LONG_3S) {
        if (proc->battery->get_charge_status() != CHARGE_STATE_ERROR) {
            LOG_WM("key post exit charge policy task");
            POST_TASK_HANDLER_NO_DELAY_ARG1(power_off_ui_common_cb, proc, 
                POWER_OFF_HDR_EXIT_CHARGE_POLICY, POWER_OFF_PAGE_POLICY_TASK_NAME);
        } else {
            power_off_handle(proc);
        }
    } else if (keytype == KEY_EVENT_LONG_10S) {
        LOG_WM("power off reboot");
        msleep(20);
        platform_system_reboot();
    } 
    return 0;
}
#endif

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_power_off_t *proc = CONTAINER_OF(thiz, ui_proc_power_off_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;
    if (new_state == ID_SCR_STATE_ON_RESUME) {
#if IS_SUPPORT_POWER_OFF_WHEN_CHARGING
        ADD_KEYEVENT_CB(proc->key, "power_off_wm", proc, KEY_CODE_POWER, KEY_EVENT_LONG_3S | KEY_EVENT_LONG_10S,
            on_power_off_keyevent_hdr);
        ADD_BATTERY_STATE_CB(proc->battery, proc, on_power_off_window_battery_state_cb);
#endif
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        if (proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 0);
        }

        if (proc->led->hook->set_night_light_level) {   
            proc->led->hook->set_night_light_level(proc->led, 0);
        }

        if (proc->pm) {
            proc->pm->acquire_wakelock("power_off_screen", -1); /// 永久持有锁.
        }

        if (proc->ble) {
            if (proc->ble->ble_is_connected() > 0) {
                proc->ble->set_adv_speed(BLE_OPT_ADV_STOP);
                proc->ble->disconnect_bt_by_user();
            } else {
                proc->ble->bt_adv_stop_by_user();
            }
        }
 #if IS_SUPPORT_POWER_OFF_WHEN_CHARGING       
        if (proc->extra.arg1 == 0) {
            if (proc->battery) {
                int chage_state = proc->battery->get_charge_status();
                if (BATTERY_IS_CHARGING(chage_state)) {
                    show_power_off_charging_ui(proc);
                    return 0;
                } else if(chage_state == CHARGE_STATE_ERROR) {
                    show_power_off_charge_error_ui(proc);
                    return 0;
                }
            }
        }
#endif
        start_power_off_ui_animation(proc);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_power_off_t *proc = CONTAINER_OF(thiz, ui_proc_power_off_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_power_off_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            if (proc->pm) {
                proc->pm->release_wakelock("power_off_screen");
            }
            REMOVE_ALL_HANDER_TASK();
#if IS_SUPPORT_POWER_OFF_WHEN_CHARGING            
            REMOVE_BATTERY_STATE_CB(proc->battery, on_power_off_window_battery_state_cb);
            REMOVE_KEYEVENT_CB(proc->key, on_power_off_keyevent_hdr);
#endif
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
    ui_proc_power_off_t *proc = u_malloz(sizeof(ui_proc_power_off_t));
    SET_WINDOW_CONTEXT(proc);
    window_entry_extra_t *extra = (window_entry_extra_t *)wm->get_window_entry_extra(wm);
    memcpy(&proc->extra, extra, sizeof(window_entry_extra_t));
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = POWER_OFF_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    w_ctx->is_popup_window = 0;
    wm->dispatch_new_window(w_ctx);

    return ID_WINDOW_DISPLAY_OK;
}

