/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_product_test.h"
#include "factory_tests_handler.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                    product_test
#define PAGE_NAME_STR                       "product_test"
#define PRODUCT_TEST_UI_KEY_CODE            (ALL_KEY_CODE_NO_COMBINE)
#define PRODUCT_TEST_UI_KEY_TYPE            (KEY_EVENT_CLICK | KEY_EVENT_UP | KEY_EVENT_LONG_2S)
#define PRODUCT_TEST_HOME_TASK_NAME         "product_test_home_ui"
#define PRODUCT_TEST_LCD_COLOR_TASK_NAME    "product_test_lcd_color_ui"
#define PRODUCT_TEST_LCD_SET_COLOR_TASK_NAME "product_test_lcd_set_color_ui"
#define PRODUCT_TEST_AGING_TEST_TASK_NAME   "product_test_aging_test_ui"
#define PRODUCT_TEST_BROADCAST_TASK_NAME    "product_test_broadcast_ui"
#define PRODUCT_TEST_POWER_OFF_HOLD_COMPLETE_TASK_NAME "product_test_power_off_hold_complete"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_HOME_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_LCD_COLOR_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_LCD_SET_COLOR_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_AGING_TEST_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_BROADCAST_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_POWER_OFF_HOLD_COMPLETE_TASK_NAME); \
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
static int product_test_window_async_evt_handler_cb(struct window_event_data *evt_data)
{
    window_context_t *w_ctx = (window_context_t *)evt_data->window_ctx;
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)w_ctx->user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_PRODUCT_TEST_MODE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    return 0;
}

static void factory_tests_event_report_cb(void *user_data, uint16_t event_id, void *data, int data_len)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    LOG_WM("factory_tests_event_report_cb event_id: %d", event_id);
    switch (event_id) {
        case FAC_TST_READ_SW_VER_EVENT: {
            EXCEPTION_CHECK_VOID(data);
            char *sw_ver = (char *)data;
            LOG_WM("factory test sw ver: %s", sw_ver);
            break;
        }
        default:
            break;
    }
}

static void product_test_lcd_color_env_prepare(ui_proc_product_test_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);

    if (proc->led && proc->led->hook) {
        #if IS_SUPPORT_NIGHT_LIGHT
        if (proc->led->hook->set_night_light_onoff) {
            proc->led->hook->set_night_light_onoff(proc->led, 0);
        }
        #endif
        if (proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 0);
        }
    }

    /// 屏幕背光 100%
    if (proc->led && proc->led->hook) {
        if (proc->led->hook->set_ui_backlight_level_rate) {
            proc->led->hook->set_ui_backlight_level_rate(proc->led, 100);
        }
        if (proc->led->hook->set_ui_backlight_level) {
            proc->led->hook->set_ui_backlight_level(proc->led, LCD_BACKLIGHT_LEVEL_MAX);
        }
    }
    LOG_WM("lcd color env: adaptive_off, night_off, key_bl_off, bl=100");
}

static void product_test_lcd_color_env_restore(ui_proc_product_test_t *proc)
{
    EXCEPTION_CHECK_VOID(proc);

    if (proc->led && proc->led->hook) {
        /// 恢复产测默认按键灯
        if (proc->led->hook->set_key_backlight) {
            proc->led->hook->set_key_backlight(proc->led, 100);
        }
    }
    LOG_WM("lcd color env restore: adaptive_on, key_bl=100");
}

static void product_test_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_product_test_t *proc = args->user_data;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    int action = args->arg1;
    switch (action) {
        case PRODUCT_TEST_HOME: 
            LOG_WM("%s", args->name);
            if (proc->tests) {
                proc->tests->add_product_tests_evt_lisenser(proc, TST_FACTORY_TYPE, factory_tests_event_report_cb);
                proc->tests->tests_manager_set_enable(true);
            } 
            break;
        case PRODUCT_TEST_LCD_COLOR:
            if (args->arg2 > 0) {
                proc->tests_mode = PRODUCT_TEST_LCD_COLOR_MODE;
                LOG_WM("show product home test ui for lcd color mode");
                product_test_lcd_color_env_prepare(proc);
                show_product_home_test_ui(proc);
            } else {
                proc->tests_mode = PRODUCT_TEST_HOME_MODE;
                LOG_WM("show home test ui");
                product_test_lcd_color_env_restore(proc);
                show_product_home_test_ui(proc);
            }
            break;
        case PRODUCT_TEST_LCD_SET_COLOR:
            if (product_test_set_lcd_color_by_id(proc, (uint8_t)args->arg2)) {
                proc->tests_mode = PRODUCT_TEST_LCD_COLOR_MODE;
                LOG_WM("set lcd color id:%d, color:0x%06x", proc->lcd_color_id, proc->lcd_color);
                show_lcd_color_test_ui(proc);
            } else {
                LOG_WM("invalid lcd color id:%d", args->arg2);
            }
            break;
        case PRODUCT_TEST_AGING_TEST:
            if (args->arg2 > 0) {
                proc->tests_mode = PRODUCT_TEST_AGING_TEST_MODE;
                if (proc->tests) {
                    if (proc->tests->tests_manager_set_current_mode) {
                        proc->tests->tests_manager_set_current_mode(TST_AGING_TEST_TYPE);
                    }
                    proc->tests->add_product_tests_evt_lisenser(proc, TST_FACTORY_TYPE, factory_tests_event_report_cb);
                    proc->tests->tests_manager_set_enable(true);
                }
                LOG_WM("show aging test ui");
                show_aging_test_ui(proc);
            }else {
                proc->tests_mode = PRODUCT_TEST_HOME_MODE;
                LOG_WM("show home test ui");
                show_product_home_test_ui(proc);
            }
            break;
        default:
            break;
    }
}

static int product_test_ui_keyevent_hdr(void *user_data, uint16_t keycode, uint16_t keytype)
{
    ui_proc_product_test_t *proc = user_data;
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_PRODUCT_TEST_MODE, 0);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return 0;
    }
    if (keytype == KEY_EVENT_LONG_2S &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        product_test_power_off_hold_key_long_2s_handler(proc);
        return 0;
    }
    if (keytype == KEY_EVENT_UP &&
        (keycode == KEY_CODE_POWER || keycode == KEY_CODE_START)) {
        product_test_power_off_hold_key_up_handler(proc);
        return 0;
    }
    if (keytype != KEY_EVENT_CLICK) {
        return 0;
    }
    if (proc->current_ui_type == PRODUCT_TEST_UI_TYPE_POWER_OFF_HOLD) {
        return 0;
    }
    switch (proc->tests_mode) {
        case PRODUCT_TEST_HOME_MODE:
            product_home_test_keyevent_cb(proc, keycode, keytype);
            break;
        case PRODUCT_TEST_LCD_COLOR_MODE:
            lcd_color_test_keyevent_cb(proc, keycode, keytype);
            break;
        case PRODUCT_TEST_AGING_TEST_MODE:
            aging_test_keyevent_cb(proc, keycode, keytype);
            break;
        default:
            break;
    }
    LOG_WM("current_ui_type: %d, tests_mode: %d", proc->current_ui_type, proc->tests_mode);

    return 0;
}

static void product_test_broadcast_receive_ui_cb(hdr_post_args_t *args)
{
    window_broadcast_args_t *bcd_args = (window_broadcast_args_t *)args->user_data;
    EXCEPTION_CHECK_VOID(bcd_args);
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)bcd_args->context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    if (proc->window_run_state != WINDOW_RUN_STATE_RUNNING) {
        LOG_WM("wm not running, ignore.");
        return;
    }
    switch (bcd_args->code) {
        case ID_BROADCAST_FACTORY_TEST_CMD:
            if (bcd_args->arg1 == TST_LCD_COLOR_TYPE) {
                LOG_WM("lcd color test onoff:%d", bcd_args->arg2);
                POST_WM_TASK_HANDLER_NO_DELAY_ARG2(product_test_ui_common_cb, proc,
                    PRODUCT_TEST_LCD_COLOR, bcd_args->arg2, PRODUCT_TEST_LCD_COLOR_TASK_NAME);
            }
            if (bcd_args->arg1 == TST_LCD_COLOR_SET_TYPE) {
                LOG_WM("set lcd color id:%d", bcd_args->arg2);
                POST_WM_TASK_HANDLER_NO_DELAY_ARG2(product_test_ui_common_cb, proc,
                    PRODUCT_TEST_LCD_SET_COLOR, bcd_args->arg2, PRODUCT_TEST_LCD_SET_COLOR_TASK_NAME);
            }
            if (bcd_args->arg1 == TST_AGING_TEST_TYPE) {
                LOG_WM("aging test onoff:%d", bcd_args->arg2);
                POST_WM_TASK_HANDLER_NO_DELAY_ARG2(product_test_ui_common_cb, proc,
                    PRODUCT_TEST_AGING_TEST, bcd_args->arg2, PRODUCT_TEST_AGING_TEST_TASK_NAME);
            }
            break;
        default:
            break;
   }
}

static void product_test_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)context;
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
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
    POST_WM_TASK_HANDLER_NO_DELAY(product_test_broadcast_receive_ui_cb, &proc->broadcast_args, PRODUCT_TEST_BROADCAST_TASK_NAME);
}

static void remove_product_test_pump_result_cb(ui_proc_product_test_t *proc)
{
    EXCEPTION_CHECK_VOID(proc && proc->pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(proc->pump, pump_status);
    if (pump_status > BREAST_PUMP_IDLE && pump_status < BREAST_PUMP_FINISH) {
        LOG_WM("pump status: %d => goto finish", pump_status);
        WRITE_PUMP_WORK_STATUS(proc->pump, BREAST_PUMP_FINISH, 1);
    }
    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_VOID(tests_ctx);
    ///如果当前是老化测试模式，则退出老化产测模式 并退出产测模式
    if (tests_ctx->tests_manager_get_current_mode() == TST_AGING_TEST_TYPE) {
        tests_ctx->set_factory_tests_event_process(FAC_TST_AGING_TEST_EXIT_EVENT, 0, 0, NULL);
        tests_ctx->set_factory_tests_event_process(FAC_TST_EXIT_EVENT, 0, 0, NULL);
        tests_ctx->tests_manager_set_enable(false);
        tests_ctx->tests_manager_set_current_mode(TST_NONE_TYPE);
    } else {
        tests_ctx->set_factory_tests_event_process(FAC_TST_EXIT_EVENT, 0, 0, NULL);
        tests_ctx->tests_manager_set_enable(false);
        tests_ctx->tests_manager_set_current_mode(TST_NONE_TYPE);
    }
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_product_test_t *proc = CONTAINER_OF(thiz, ui_proc_product_test_t, w_ctx);
    EXCEPTION_CHECK_INT(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_INT(ui);
    if (new_state == ID_SCR_STATE_ON_RESUME) {
        LOG_WM("==>%s window on resume", PAGE_NAME_STR);
        ADD_KEYEVENT_CB(proc->key, "product_test_wm", proc, PRODUCT_TEST_UI_KEY_CODE, 
            PRODUCT_TEST_UI_KEY_TYPE, product_test_ui_keyevent_hdr);
        POST_WM_TASK_HANDLER_DELAYED_ARG2(product_test_ui_common_cb, 100, PRODUCT_TEST_HOME, proc,
            PRODUCT_TEST_HOME_TASK_NAME);
        wm_ensure_ble_adv_started();
        proc->window_run_state = WINDOW_RUN_STATE_RUNNING;
        if (proc->tests && proc->tests->tests_manager_get_current_mode &&
            proc->tests->tests_manager_get_current_mode() == TST_LCD_COLOR_TYPE &&
            proc->tests->priv.lcd_set_color != 0) {
            POST_WM_TASK_HANDLER_NO_DELAY_ARG2(product_test_ui_common_cb, proc,
                PRODUCT_TEST_LCD_SET_COLOR, proc->tests->priv.lcd_set_color, PRODUCT_TEST_LCD_SET_COLOR_TASK_NAME);
        }
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>PRODUCT_TEST_ON_CREATE");
        /// 获取当前测试模式
        uint8_t current_mode = TST_NONE_TYPE;
        if (proc->tests->tests_manager_get_current_mode) {
            current_mode = proc->tests->tests_manager_get_current_mode();
        }
        /// 根据当前测试模式显示UI
        if (current_mode == TST_LCD_COLOR_TYPE) {
            proc->tests_mode = PRODUCT_TEST_LCD_COLOR_MODE;
            product_test_lcd_color_env_prepare(proc);
            show_product_home_test_ui(proc);
        } else {
            if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
                LOG_WM("set key backlight to 100");
                proc->led->hook->set_key_backlight(proc->led, 100);
            }
            proc->tests_mode = PRODUCT_TEST_HOME_MODE;
            show_product_home_test_ui(proc);
        }
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_product_test_t *proc = CONTAINER_OF(thiz, ui_proc_product_test_t, w_ctx);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_INT(wm);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE: 
            proc->window_run_state = WINDOW_RUN_STATE_PAUSED;
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            product_test_power_off_hold_ui_exit(proc);
            stop_product_test_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            remove_product_test_pump_result_cb(proc);
            if (proc->tests) {
                proc->tests->remove_product_tests_evt_lisenser(factory_tests_event_report_cb);
            } else {
                LOG_WM("product_tst not present, ignore.");
            }
            REMOVE_KEYEVENT_CB(proc->key, product_test_ui_keyevent_hdr);
            u_free(proc);
            proc = NULL;
            LOG_WM("==>ON_DESTORY end");
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
    LOG_WM("enter, time: %ld.", get_timestamp());
    ui_proc_product_test_t *proc = u_malloz(sizeof(ui_proc_product_test_t));
    EXCEPTION_CHECK_INT(proc);
    SET_WINDOW_CONTEXT(proc);
    /// 设置窗口标识.
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = PRODUCT_TEST_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->key = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    proc->battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    proc->tests = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    proc->ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    proc->airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AIRPLANE_MODE);
    proc->led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    proc->st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    proc->pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    /// 注册广播监听.
    proc->receiver.context = proc;
    proc->receiver.name = "product_test_brc";
    proc->receiver.id.code |= ID_BROADCAST_FACTORY_TEST_CMD;
    proc->receiver.receiver_cb = product_test_broadcast_receive_cb;
    register_broadcast_receiver(&proc->receiver);
    /// 设置窗口上下文.
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1; 
    w_ctx->is_popup_window = 0;
    w_ctx->user_data = proc;
    w_ctx->async_evt_handler_cb = product_test_window_async_evt_handler_cb;
    wm->dispatch_new_window(w_ctx);
    return ID_WINDOW_DISPLAY_OK;
}

