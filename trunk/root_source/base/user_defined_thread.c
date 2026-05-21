/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
#if IS_SUPPORT_PRESSURE_TEST  
static int run_led_test_cb(void)
{
    static uint8_t led_status = LED_STATUS_OFF;
    LOG_PBUS("===> run led test!!!!! status: %d", led_status);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    led_status = (led_status == LED_STATUS_OFF) ? LED_STATUS_ON : LED_STATUS_OFF;
    return 1000;
}

static void run_pump_test_cb(void)
{
    LOG_PBUS("===> run pump test!!!!!");
}

static void reboot_system_test_cb(void)
{
    static int reboot_count = 0;
    reboot_count++;
    if (reboot_count < 10) {
        return;
    } 
    reboot_count = 0;
    LOG_PBUS("===> reboot system after 100ms");
    msleep(100);
    platform_system_reboot();
}

static int lcd_data_send_test_cb(void)
{
#if (V3_HARDWARE_VERSION == V3_2710IMP_HARDWARE_V1)
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *lcd_dev = pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
    EXCEPTION_CHECK_INT(lcd_dev);
    uint8_t img_id = LCD_DISPLAY_PUMPING_COMPLETED;
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t)),
        .tlvs_num = 1,
    };
    EXCEPTION_CHECK_INT(msg.tlvs);
    msg.tlvs[0].type = LCD_IMG_ID;
    msg.tlvs[0].data_size = 1;
    msg.tlvs[0].data[0] = img_id;
    lcd_dev->ioctl(lcd_dev, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
#endif
    return 1000;
}

static void motor_debug_policy_test_cb(void)
{
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_VOID(motor_ctx);
    EXCEPTION_CHECK_VOID(motor_ctx->hook);
    EXCEPTION_CHECK_VOID(motor_ctx->hook->motor_debug_with_user_para);
    motor_policy_original_para_t para = {
        .pump_level = 75 * PUMP_DUTY_SCALE_FACTOR,
        .pumping_time = 1000,
        .valve_on_time = 500,
        .sleep_time = 200,
        .hold_time = 100,
    };
    LOG_PBUS("===> motor debug test!!!!! pump_level: %d, pumping_time: %d, valve_on_time: %d, sleep_time: %d, hold_time: %d",
        para.pump_level, para.pumping_time, para.valve_on_time, para.sleep_time, para.hold_time);
    motor_ctx->hook->motor_debug_with_user_para(motor_ctx, MOTOR_USER_DEBUG_POLICY, &para);
}

static int motor_debug_test_cb(void)
{
    static uint8_t state = MOTOR_USER_DEBUG_PUMP_ON;
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    motor_debug_para_t para = {
        .duty = 75,
        .hold_time = 300,
    };
    if (state > MOTOR_USER_DEBUG_VALVE_OFF) {
        state = MOTOR_USER_DEBUG_PUMP_ON;
    }
    LOG_PBUS("===> motor debug test!!!!! state: %d, duty: %d, hold_time: %d", state, para.duty, para.hold_time);
    motor_ctx->hook->motor_debug_with_user_para(motor_ctx, state, &para);
    state++;
    return 1000;
}

static int pumping_test_cb(void)
{
    return 1000*30;
}

///模拟用户操作按键切换
static int bottom_key_switch_test_cb(void)
{   
    static uint8_t key_code = KEY_CODE_START;
    key_input_context_t *key_input = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_INT(key_input);
    /// 启动按键模拟
    key_input->dispatch_client_input_key(key_code, KEY_EVENT_CLICK);
    if (key_code == KEY_CODE_START) {
        key_code = KEY_CODE_MODE;
    } else if (key_code == KEY_CODE_MODE) {
        key_code = KEY_CODE_FREQUENCY;
    } else if (key_code == KEY_CODE_FREQUENCY) {
        key_code = KEY_CODE_BREASTFEEDING;
    } else if (key_code == KEY_CODE_BREASTFEEDING) {
        key_code = KEY_CODE_LEFT_CONTROL;
    } else if (key_code == KEY_CODE_LEFT_CONTROL) {
        key_code = KEY_CODE_RIGHT_CONTROL;
    } else if (key_code == KEY_CODE_RIGHT_CONTROL) {
        key_code = KEY_CODE_START;
    }
    LOG_PBUS("===> bottom key switch test!!!!! key_code: %d", key_code);
    return 15000;
}

static void sys_dump_wdg_test_cb(void)
{
    window_manager_context_t *wm = NULL;
    LOG_PBUS("===> sys dump wdg test!!!!! wm: %p", wm);
    msleep(100);
    wm->window_scheduling_start();
    while(1) {
    }
}

static int external_flash_test_cb(void)
{
    int ret = 0;
    uint32_t addr = 0;
    uint32_t size = 0;
    uint32_t start_time = 0;
    start_time = get_timestamp();
    addr = 0x80000;
    size = 50 * 1024;
    uint8_t *buff = u_malloz_psram(size, 0);
    if (buff) {
        ret = platform_flash_read(PARTITION_IMG_EXT_DATA_NAME, addr, buff, size);
        if (ret != 0) {
            LOGE("===> read failed, ret: %d", ret);
        }
        LOGI("===>read size: %d, addr: %d, time: %dms", size, addr, get_timestamp_ms(start_time));
        u_free_psram(buff, 0);
    }
    return 1000;
}

static int popup_window_test_cb(void)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
    if (wm->is_current_window(wm, ID_WINDOW_POPUP_WINDOW)) {
        LOG_PBUS("===> popup window already exists, ignore.");
        return 8000;
    }
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_POPUP_WINDOW;
    msg.union_body.extra.arg1 = ID_POPUP_UI_LOW_POWER;
    msg.union_body.extra.popup_exit_time = 8000;
    wm->dispatch_new_window_args(wm, &msg);
    LOG_PBUS("===> popup window test!!!!!");
    return 15000;
}

static void hdr_run_pressure_test_cb(hdr_post_args_t *args)
{
    // sys_dump_wdg_test_cb();
    // reboot_system_test_cb();
    // run_pump_test_cb();
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, lcd_data_send_test_cb(), "pressure_test:continue");
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, run_led_test_cb(), "pressure_test:continue"); 
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, external_flash_test_cb(), "pressure_test:continue");
    //POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, popup_window_test_cb(), "pressure_test:continue");
    // motor_debug_policy_test_cb();
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, motor_debug_test_cb(), "pressure_test:continue");
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, 1000, "pressure_test:continue");
    //POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, pumping_test_cb(), "pressure_test:continue");
    POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, bottom_key_switch_test_cb(), "pressure_test:continue");
}
#endif

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int run_user_thread_dispatch(void)
{
#if IS_SUPPORT_PRESSURE_TEST  
     //POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, 5000, "pressure_test");
#endif
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm) {
        wm->window_scheduling_start();
    }
    return 0;
}
