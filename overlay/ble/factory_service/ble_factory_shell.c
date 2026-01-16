/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_factory_message.h" // IWYU pragma: keep
#include "ble_factory_shell.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_PUMP_POLICY_SHELL_CMD              "#pump"
#define BLE_PUMP_CTRL_SHELL_CMD                "#pCtrl"
#define BLE_VALVE_CTRL_SHELL_CMD               "#vCtrl"
#define BLE_SYSTEM_RESET_SHELL_CMD             "#sysRst"
#define BLE_FORBID_SLEEP_SHELL_CMD             "#forbidSlp"
#define BLE_IMG_TEST_SHELL_CMD                 "#imgTest"
#define BLE_ALGO_SAMPLING_DATA_SHELL_CMD       "#algoSmp"
#define BLE_ALGO_SAMPLING_INTERVAL_SHELL_CMD   "#algoSmpIntvl"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void ble_shell_pump_policy(const uint8_t *param, uint16_t len);
static void ble_shell_pump_ctrl(const uint8_t *param, uint16_t len);
static void ble_shell_valve_ctrl(const uint8_t *param, uint16_t len);
static void ble_shell_system_reset(const uint8_t *param, uint16_t len);
static void ble_shell_forbid_sleep(const uint8_t *param, uint16_t len);
static void ble_shell_img_test(const uint8_t *param, uint16_t len);
static void ble_shell_algo_sampling_control(const uint8_t *param, uint16_t len);
static void ble_shell_algo_sampling_interval(const uint8_t *param, uint16_t len);
static const ble_factory_shell_t shell_list[] = {
    {BLE_PUMP_POLICY_SHELL_CMD,   ble_shell_pump_policy},
    {BLE_PUMP_CTRL_SHELL_CMD,     ble_shell_pump_ctrl},
    {BLE_VALVE_CTRL_SHELL_CMD,    ble_shell_valve_ctrl},
    {BLE_SYSTEM_RESET_SHELL_CMD,  ble_shell_system_reset},
    {BLE_FORBID_SLEEP_SHELL_CMD,  ble_shell_forbid_sleep},
    {BLE_IMG_TEST_SHELL_CMD,      ble_shell_img_test},
    {BLE_ALGO_SAMPLING_DATA_SHELL_CMD, ble_shell_algo_sampling_control},
    {BLE_ALGO_SAMPLING_INTERVAL_SHELL_CMD, ble_shell_algo_sampling_interval},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ble_shell_pump_ctrl(const uint8_t *param, uint16_t len)
{
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    motor_debug_para_t para = {0};
    int duty = 0, hold_time = 0;
    sscanf((const char *)param, "%d %d", &duty, &hold_time);
    LOG_BLE("shell pump ctrl duty %d, hold_time %d", duty, hold_time);
    para.duty = duty;
    para.hold_time = hold_time;
    motor->hook->motor_debug_with_user_para(motor, para.duty == 0 ? MOTOR_USER_DEBUG_PUMP_OFF : MOTOR_USER_DEBUG_PUMP_ON, &para);
}

static void ble_shell_valve_ctrl(const uint8_t *param, uint16_t len)
{
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    motor_debug_para_t para = {0};
    int duty = 0, hold_time = 0;
    sscanf((const char *)param, "%d %d", &duty, &hold_time);
    LOG_BLE("shell valve ctrl duty %d, hold_time %d", duty, hold_time);
    para.duty = duty;
    para.hold_time = hold_time;
    motor->hook->motor_debug_with_user_para(motor, para.duty == 0 ? MOTOR_USER_DEBUG_VALVE_OFF : MOTOR_USER_DEBUG_VALVE_ON, &para);
}

static void ble_shell_pump_policy(const uint8_t *param, uint16_t len)
{
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    motor_policy_original_para_t para = {0};
    int pump_level = 0, pumping_time = 0, valve_on_time = 0, sleep_time = 0, hold_time = 0;
    sscanf((const char *)param, "%d %d %d %d %d", \
        &pump_level, &pumping_time, &valve_on_time, &sleep_time, &hold_time);
    LOG_BLE("shell pump policy pump_level %d, pumping_time %d, valve_on_time %d, sleep_time %d, hold_time %d", \
        pump_level, pumping_time, valve_on_time, sleep_time, hold_time);
    para.pump_level = pump_level;
    para.pumping_time = pumping_time;
    para.valve_on_time = valve_on_time;
    para.sleep_time = sleep_time;
    para.hold_time = hold_time;
    motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_DEBUG_POLICY, &para);
}

static void ble_shell_system_reset(const uint8_t *param, uint16_t len)
{
    storage_manager_context_t *st_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    int reset_type = 0;
    sscanf((const char *)param, "%d", &reset_type);
    LOG_BLE("shell system reset type %d", reset_type);
    if (st_ctx->factory_reset) {
        st_ctx->factory_reset(reset_type == 0 ? RESET_NOT_REBOOT : RESET_AND_REBOOT);
    }
}

static void ble_shell_forbid_sleep(const uint8_t *param, uint16_t len)
{
    int forbid_sleep = 0;
    sscanf((const char *)param, "%d", &forbid_sleep);
    LOG_BLE("shell forbid sleep %d", forbid_sleep);
}

static int ble_factory_shell_cmd_handle(const uint8_t *param, uint16_t len)
{
    uint16_t str_len = 0;
    for (uint8_t i = 0; i < LENGTH_OF_ARRAY(shell_list); i++) {
        str_len = strlen(shell_list[i].cmd_str);
        if (str_len > len) {
            continue;
        }

        if (memcmp(param, shell_list[i].cmd_str, strlen(shell_list[i].cmd_str)) == 0) {
            shell_list[i].cmd_fun(param + str_len, len - str_len);
            return 0;
        }
    }
    return -1;
}

static void ble_shell_img_test(const uint8_t *param, uint16_t len)
{
    LOG_BLE("shell img test");
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    rt_device_node_t *lcd = pbus->find_device(ID_DEVICE_DISPLAY_PANEL);
    int img_id = 0, hold_time = 0, pump_level = 0x01070109, ble_status = 1, battery_status = 1, pump_mode = 1;
    sscanf((const char *)param, "%d %d %d %d %d", &img_id, &hold_time, &ble_status, &battery_status, &pump_mode);
    lcd_tlvs_msg_t msg = {
        .srv_id = LCD_DISPLAY_EVENT_SRV,
        .tlvs = u_malloz(sizeof(lcd_tlvs_t) * 6),
        .tlvs_num = 6,
    };
    EXCEPTION_CHECK_VOID(msg.tlvs);
    msg.tlvs[0].type = LCD_IMG_ID;
    msg.tlvs[0].data_size = 1;
    msg.tlvs[0].data[0] = img_id;
    msg.tlvs[1].type = LCD_DEVICE_RUNNING_TIME;
    msg.tlvs[1].data_size = 2;
    msg.tlvs[1].data[0] = hold_time >> 8;
    msg.tlvs[1].data[1] = hold_time & 0xFF;
    msg.tlvs[2].type = LCD_PUMP_LEVEL;
    msg.tlvs[2].data_size = 4;
    memcpy(msg.tlvs[2].data, &pump_level, 4);
    msg.tlvs[3].type = LCD_BLE_STATUS;
    msg.tlvs[3].data_size = 1;
    msg.tlvs[3].data[0] = ble_status > 2 ? 1 : ble_status;
    msg.tlvs[4].type = LCD_BATTERY_STATUS;
    msg.tlvs[4].data_size = 1;
    msg.tlvs[4].data[0] = battery_status > 100 ? 100 : battery_status;
    msg.tlvs[5].type = LCD_PUMP_MODE;
    msg.tlvs[5].data_size = 1;
    msg.tlvs[5].data[0] = pump_mode > 7 ? 3 : pump_mode;
    lcd->ioctl(lcd, OP_CODE_PANEL_DATA_SEND, (void *)&msg);
    u_free(msg.tlvs);
}

static void ble_shell_algo_sampling_control(const uint8_t *param, uint16_t len)
{
    algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    int control_type = 0;
    sscanf((const char *)param, "%d", &control_type);
    LOG_BLE("shell algo sampling control");
    if (control_type) {
        algo->hook->algo_sampling_start(algo);
    } else {
        algo->hook->algo_sampling_stop(algo);
    }
}

static void ble_shell_algo_sampling_interval(const uint8_t *param, uint16_t len)
{
    algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    int interval = 0;
    sscanf((const char *)param, "%d", &interval);
    LOG_BLE("shell algo sampling interval %d", interval);
    algo->hook->algo_change_sampling_interval(algo, interval);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ble_factory_shell_handle(uint8_t *p_data, uint16_t len)
{
    if (p_data[0] != '#') {
        return;
    }

    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    if (pm->is_pm_suspend()) {
        pm->pm_wakeup(PM_WAKEUP_REASON_BLE_REQ);
    }

    if (ble_factory_shell_cmd_handle(p_data, len) != 0) {
        LOGE("no ble factory shell cmd handle");
    }
}
