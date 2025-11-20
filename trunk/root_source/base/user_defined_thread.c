/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "hal_motor.h"
#include "bsp_lcd_internal.h" // IWYU pragma: keep
#include "root_common.h"
#include "uart_extension_lcd.h" // IWYU pragma: keep
#include "peripheral_bus_api.h"
#include "root_common_api.h" // IWYU pragma: keep

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
    static uint8_t led_level = 0;
    LOG_PBUS("===> run led test!!!!! level: %d", led_level);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
    led_status_t led_status = {
        .mode = LED_MODE_LEVEL,
        .un.luminous.level = led_level,
    };
    led_level += 5;
    if (led_level > LED_LEVEL_MAX) {
        led_level = 0;
    }
    int args[] = {PUMP_LED_ALL_ID, (int)&led_status};
    led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)args);
    return 1000;
}

static void run_pump_test_cb(void)
{
    LOG_PBUS("===> run pump test!!!!!");
    breast_pump_manager_context_t *breast_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    // breast_ctx->set_breast_pump_mode(BREAST_PUMP_PP_MILK_TRACK_MODE);
    breast_ctx->set_breast_pump_status(BREAST_PUMP_CONTINUE);
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
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    rt_device_node_t *lcd_dev = pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
    EXCEPTION_CHECK_INT(lcd_dev);

    uint8_t img_id = LCD_DISPLAY_PUMPING_COMPLETED;

    lcd_tlvs_t tlvs = {
        .type = LCD_IMG_ID,
        .data_size = 1,
        .data[0] = img_id,
    };
    LOG_PBUS("===> lcd data send test!!!!! %p, %p", &img_id, &tlvs);
    int args[] = {1, (int)&tlvs};
    lcd_dev->ioctl(lcd_dev, OP_CODE_PANEL_DATA_SEND, (void *)args);
    return 1000;
}

static void motor_debug_policy_test_cb(void)
{
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_VOID(motor_ctx);
    EXCEPTION_CHECK_VOID(motor_ctx->hook);
    EXCEPTION_CHECK_VOID(motor_ctx->hook->motor_debug_with_user_para);
    motor_policy_original_para_t para = {
        .pump_level = 75,
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
    if(state > MOTOR_USER_DEBUG_VALVE_OFF) {
        state = MOTOR_USER_DEBUG_PUMP_ON;
    }
    motor_ctx->hook->motor_debug_with_user_para(motor_ctx, state, &para);
    state++;
    return 1000;
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

static void hdr_run_pressure_test_cb(hdr_post_args_t *args)
{
    // sys_dump_wdg_test_cb();
    // reboot_system_test_cb();
    // run_pump_test_cb();
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, lcd_data_send_test_cb(), "pressure_test:continue");
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, run_led_test_cb(), "pressure_test:continue"); 
    // motor_debug_policy_test_cb();
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, motor_debug_test_cb(), "pressure_test:continue");
}
#endif

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int run_user_thread_dispatch(void)
{
#if IS_SUPPORT_PRESSURE_TEST  
    POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, 2000, "pressure_test");
#endif
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm) {
        wm->window_scheduling_start();
    }
    return 0;
}
