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
#define PRODUCT_TEST_UI_KEY_TYPE            (KEY_EVENT_CLICK)
#define PRODUCT_TEST_HOME_TASK_NAME         "product_test_home_ui"
#define PRODUCT_TEST_LCD_COLOR_TASK_NAME    "product_test_lcd_color_ui"
#define PRODUCT_TEST_AGING_TEST_TASK_NAME   "product_test_aging_test_ui"
#define REMOVE_ALL_HANDER_TASK() \
do { \
    LOG_WM("==>REMOVE_ALL_HANDER_TASK"); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_HOME_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_LCD_COLOR_TASK_NAME); \
    REMOVE_TASK_HANDLER_DIRECTLY(PRODUCT_TEST_AGING_TEST_TASK_NAME); \
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
static void factory_tests_event_report_cb(void *user_data, uint16_t event_id, void *data, int data_len)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    LOG_WM("factory_tests_event_report_cb event_id: %d", event_id);
    switch (event_id) {
        case FAC_TST_READ_SW_VER_EVENT: {
            EXCEPTION_CHECK_VOID(data);
            char *sw_ver = (char *)data;
            LOG_WM("factory test sw ver: %s", sw_ver);
        }
        break;
        default:
            break;
    }
}

static void product_test_ui_common_cb(hdr_post_args_t *args)
{
    ui_proc_product_test_t *proc = args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
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
                LOG_WM("show lcd color test ui");
                // show_lcd_color_test_ui(proc);
            } else {
                proc->tests_mode = PRODUCT_TEST_HOME_MODE;
                LOG_WM("show home test ui");
                show_product_home_test_ui(proc);
            }
            break;
        case PRODUCT_TEST_AGING_TEST:
            if (args->arg2 > 0) {
                proc->tests_mode = PRODUCT_TEST_AGING_TEST_MODE;
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
    EXCEPTION_CHECK_INT(proc);
    WINDOW_CONTEXT_GUARD_INT(proc, ID_WINDOW_PRODUCT_TEST_MODE, 0);
    EXCEPTION_CHECK_INT(proc->key);
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

static void product_test_broadcast_receive_cb(broadcast_mask_id_t id, void *context, int arg1, int arg2,
     broadcast_priv_data_t *pdata)
{
    ui_proc_product_test_t *proc = (ui_proc_product_test_t *)context;
    EXCEPTION_CHECK_VOID(proc);
    WINDOW_CONTEXT_GUARD_VOID(proc, ID_WINDOW_PRODUCT_TEST_MODE);
    switch (id.code) {
        case ID_BROADCAST_FACTORY_TEST_CMD:
            if (arg1 == TST_LCD_COLOR_TYPE) {
                LOG_WM("lcd color test onoff:%d", arg2);
                POST_TASK_HANDLER_NO_DELAY_ARG2(product_test_ui_common_cb, proc,
                    PRODUCT_TEST_LCD_COLOR, arg2, PRODUCT_TEST_LCD_COLOR_TASK_NAME);
            }
            if (arg1 == TST_AGING_TEST_TYPE) {
                LOG_WM("aging test onoff:%d", arg2);
                POST_TASK_HANDLER_NO_DELAY_ARG2(product_test_ui_common_cb, proc,
                    PRODUCT_TEST_AGING_TEST, arg2, PRODUCT_TEST_AGING_TEST_TASK_NAME);
            }
            break;
        default:
            break;
    }
}

static void remove_product_test_pump_result_cb(breast_pump_manager_context_t *pump)
{
    EXCEPTION_CHECK_VOID(pump);
    int pump_status = 0;
    READ_PUMP_WORK_STATUS(pump, pump_status);
    if (pump_status > BREAST_PUMP_IDLE && pump_status < BREAST_PUMP_FINISH) {
        LOG_WM("pump status: %d => goto finish", pump_status);
        WRITE_PUMP_WORK_STATUS(pump, BREAST_PUMP_FINISH, 1);
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
        if (proc->key) {
            proc->key->add_keyevent_handler("product_test_wm", proc, PRODUCT_TEST_UI_KEY_CODE, 
                PRODUCT_TEST_UI_KEY_TYPE, product_test_ui_keyevent_hdr, KEY_HANDLER_SHIELD_LEVEL_1);
        }
        POST_TASK_HANDLER_DELAYED_ARG2(product_test_ui_common_cb, 100, PRODUCT_TEST_HOME, proc,
            PRODUCT_TEST_HOME_TASK_NAME);
    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>PRODUCT_TEST_ON_CREATE");
        if (proc->led && proc->led->hook && proc->led->hook->set_key_backlight) {
            LOG_WM("set key backlight to 100");
            proc->led->hook->set_key_backlight(proc->led, 100);
        }
        /// 获取当前测试模式
        uint8_t current_mode = TST_NONE_TYPE;
        if (proc->tests->tests_manager_get_current_mode) {
            current_mode = proc->tests->tests_manager_get_current_mode();
        }
        /// 根据当前测试模式显示UI
        if (current_mode == TST_LCD_COLOR_TYPE) {
            proc->tests_mode = PRODUCT_TEST_LCD_COLOR_MODE;
            // show_lcd_color_test_ui(proc);
            show_product_home_test_ui(proc); /// 临时首页默认home
        } else {
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
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_product_test_ui(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("==>%s window on destory", PAGE_NAME_STR);
            SET_WINDOW_CONTEXT(NULL);
            proc->margic = UI_WINDOW_MARGIC_CODE_INVALID;
            REMOVE_ALL_HANDER_TASK();
            unregister_broadcast_receiver(&proc->receiver);
            remove_product_test_pump_result_cb(proc->pump);
            if (proc->tests) {
                proc->tests->remove_product_tests_evt_lisenser(factory_tests_event_report_cb);
            } else {
                LOG_WM("product_tst not present, ignore.");
            }
            if (proc->key) {
                proc->key->remove_keyevent_handler(product_test_ui_keyevent_hdr);
            } else {
                LOG_WM("key not present, ignore.");
            }
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
    wm->dispatch_new_window(w_ctx);
    return ID_WINDOW_DISPLAY_OK;
}

