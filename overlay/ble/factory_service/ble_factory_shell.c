/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_factory_message.h" // IWYU pragma: keep
#include "ble_factory_shell.h" // IWYU pragma: keep
#include "adaptive_dimming.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define BLE_PUMP_POLICY_SHELL_CMD              "#pump"
#define BLE_PUMP_LEVEL_SHELL_CMD               "#pumpLvl"
#define BLE_PUMP_SPEED_SHELL_CMD               "#pumpSpd"
#define BLE_PUMP_CTRL_SHELL_CMD                "#pCtrl"
#define BLE_VALVE_CTRL_SHELL_CMD               "#vCtrl"
#define BLE_SYSTEM_RESET_SHELL_CMD             "#sysRst"
#define BLE_FORBID_SLEEP_SHELL_CMD             "#forbidSlp"
#define BLE_IMG_TEST_SHELL_CMD                 "#imgTest"
#define BLE_ALGO_SAMPLING_DATA_SHELL_CMD       "#algoSmp"
#define BLE_ALGO_SAMPLING_INTERVAL_SHELL_CMD   "#algoSmpIntvl"
#define BLE_NIGHT_LIGHT_ADAPTIVE_SHELL_CMD     "#LightAdaptive"
#define BLE_NIGHT_LIGHT_SWITCH_SHELL_CMD       "#NightLightSwitch"
#define BLE_GLOBAL_ADAPTIVE_DIMMING_SWITCH_SHELL_CMD "#GlobalAdaptive"
#define BLE_NIGHT_LIGHT_GEAR_SHELL_CMD         "#NightLightGear"
#define BLE_BACK_LIGHT_LEVEL_SHELL_CMD         "#BackLightLevel"
#define BLE_NIGHT_LIGHT_LEVEL_SHELL_CMD        "#NightLightLevel"
#define BLE_LCD_BACKLIGHT_SHELL_CMD            "#lcdBk"
#define BLE_LED_NIGHT_LIGHT_SHELL_CMD          "#ledNightLight"
#define BLE_LIGHT_SENSOR_CTL_SHELL_CMD         "#LsSmpCtrl"
#define BLE_LIGHT_SENSOR_SAMPLE_CTL_SHELL_CMD  "#LsSmpIntvl"
#define BLE_CHARGE_CURRENT_ADJUST_SHELL_CMD    "#chargeCurAdj"
#define BLE_UTC_TIME_CMD                       "#utcTime"
#define BLE_CUSTOM_MODE_UPDATE_CMD             "#customUpdate"
#define BLE_CUSTOM_MODE_CLEAR_CMD              "#customClear"
#define BLE_TRIPLE_WRITE_SHELL_CMD             "#tripleWrite"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void ble_shell_pump_policy(const uint8_t *param, uint16_t len);
static void ble_shell_pump_level(const uint8_t *param, uint16_t len);
static void ble_shell_pump_speed(const uint8_t *param, uint16_t len);
static void ble_shell_pump_ctrl(const uint8_t *param, uint16_t len);
static void ble_shell_valve_ctrl(const uint8_t *param, uint16_t len);
static void ble_shell_system_reset(const uint8_t *param, uint16_t len);
static void ble_shell_forbid_sleep(const uint8_t *param, uint16_t len);
static void ble_shell_img_test(const uint8_t *param, uint16_t len);
static void ble_shell_algo_sampling_control(const uint8_t *param, uint16_t len);
static void ble_shell_algo_sampling_interval(const uint8_t *param, uint16_t len);
static void ble_shell_night_light_adaptive(const uint8_t *param, uint16_t len); /// @deprecated
static void ble_shell_night_light_switch(const uint8_t *param, uint16_t len); /// 夜灯开关
static void ble_shell_global_adaptive_dimming_switch(const uint8_t *param, uint16_t len);
static void ble_shell_night_light_gear(const uint8_t *param, uint16_t len); /// 夜灯档位
static void ble_shell_back_light_level(const uint8_t *param, uint16_t len); /// 背光亮度
static void ble_shell_night_light_level(const uint8_t *param, uint16_t len); /// 夜灯亮度
static void ble_shell_lcd_backlight(const uint8_t *param, uint16_t len);
static void ble_shell_led_night_light(const uint8_t *param, uint16_t len);
static void ble_shell_adapte_light_sampling_control(const uint8_t *param, uint16_t len);
static void ble_shell_adapte_light_sampling_interval(const uint8_t *param, uint16_t len);
static void ble_shell_charge_current_adjust(const uint8_t *param, uint16_t len);
static void ble_shell_utc_time(const uint8_t *param, uint16_t len);
static void ble_shell_custom_mode_step_info_update(const uint8_t *param, uint16_t len);
static void ble_shell_custom_mode_step_info_clear(const uint8_t *param, uint16_t len);
static void ble_shell_triple_write(const uint8_t *param, uint16_t len);
static void ble_shell_triple_set_field(uint8_t *dst, uint8_t *dst_len, uint16_t max_len, const char *src);
static const ble_factory_shell_t shell_list[] = {
    /// 较长命令放前面，避免前缀误匹配（匹配逻辑也会校验命令边界）
    {BLE_PUMP_LEVEL_SHELL_CMD,    ble_shell_pump_level},
    {BLE_PUMP_SPEED_SHELL_CMD,    ble_shell_pump_speed},
    {BLE_PUMP_POLICY_SHELL_CMD,   ble_shell_pump_policy},
    {BLE_PUMP_CTRL_SHELL_CMD,     ble_shell_pump_ctrl},
    {BLE_VALVE_CTRL_SHELL_CMD,    ble_shell_valve_ctrl},
    {BLE_SYSTEM_RESET_SHELL_CMD,  ble_shell_system_reset},
    {BLE_FORBID_SLEEP_SHELL_CMD,  ble_shell_forbid_sleep},
    {BLE_IMG_TEST_SHELL_CMD,      ble_shell_img_test},
    {BLE_ALGO_SAMPLING_INTERVAL_SHELL_CMD, ble_shell_algo_sampling_interval},
    {BLE_ALGO_SAMPLING_DATA_SHELL_CMD, ble_shell_algo_sampling_control},
    {BLE_NIGHT_LIGHT_ADAPTIVE_SHELL_CMD, ble_shell_night_light_adaptive}, /// @deprecated
    {BLE_NIGHT_LIGHT_SWITCH_SHELL_CMD, ble_shell_night_light_switch},
    {BLE_GLOBAL_ADAPTIVE_DIMMING_SWITCH_SHELL_CMD, ble_shell_global_adaptive_dimming_switch},
    {BLE_NIGHT_LIGHT_GEAR_SHELL_CMD, ble_shell_night_light_gear},
    {BLE_BACK_LIGHT_LEVEL_SHELL_CMD, ble_shell_back_light_level},
    {BLE_NIGHT_LIGHT_LEVEL_SHELL_CMD, ble_shell_night_light_level},
    {BLE_LCD_BACKLIGHT_SHELL_CMD, ble_shell_lcd_backlight},
    {BLE_LED_NIGHT_LIGHT_SHELL_CMD, ble_shell_led_night_light},
    {BLE_LIGHT_SENSOR_CTL_SHELL_CMD, ble_shell_adapte_light_sampling_control},
    {BLE_LIGHT_SENSOR_SAMPLE_CTL_SHELL_CMD, ble_shell_adapte_light_sampling_interval},
    {BLE_CHARGE_CURRENT_ADJUST_SHELL_CMD, ble_shell_charge_current_adjust},
    {BLE_UTC_TIME_CMD, ble_shell_utc_time},
    {BLE_CUSTOM_MODE_UPDATE_CMD, ble_shell_custom_mode_step_info_update},
    {BLE_CUSTOM_MODE_CLEAR_CMD, ble_shell_custom_mode_step_info_clear},
    {BLE_TRIPLE_WRITE_SHELL_CMD, ble_shell_triple_write},
};

/// 工厂三元组预设（productKey / deviceName / deviceSecret / sn，mac 仅作日志参考）
typedef struct {
    const char *mac;
    const char *product_key;
    const char *device_name;
    const char *device_secret;
    const char *sn;
} ble_factory_triple_preset_t;

static const ble_factory_triple_preset_t triple_preset_list[] = {
    {
        "DFF100003111",
        "j3rJ03",
        "3fg3d7flqv14ea6l",
        "dym6ba9km2zkfykh",
        "V300BBBB123DFF1000031110E662FN10001",
    },
    {
        "DFF100003101",
        "j3rJ03",
        "13wdsdf5n19xmj7m",
        "wy6g8udxjstd5v5c",
        "V300BBBB123DFF1000031010E662FN10002",
    },
    {
        "DFF1000030F0",
        "j3rJ03",
        "hvmm3kwj141k9g50",
        "rl6hjfxoqssvy233",
        "V300BBBB123DFF1000030F00E662FN10003",
    },
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
    motor->hook->motor_debug_with_user_para(motor, para.duty == 0 ?
        MOTOR_USER_DEBUG_PUMP_OFF : MOTOR_USER_DEBUG_PUMP_ON, &para);
}

static uint8_t ble_shell_get_level_speed_window_id(window_manager_context_t *wm)
{
    if (wm->is_current_window(wm, ID_WINDOW_HOME)) {
        return ID_WINDOW_HOME;
    }
    if (wm->is_current_window(wm, ID_WINDOW_PUMP)) {
        return ID_WINDOW_PUMP;
    }
    return ID_WINDOW_INVALID;
}

/// #pumpLvl <side> <level>  side: 0左/1右/2双边（home/pump 当前窗口）
static void ble_shell_pump_level(const uint8_t *param, uint16_t len)
{
    int side = 0, level = 0;
    window_manager_context_t *wm;
    window_evt_level_t *data;
    uint8_t window_id;

    sscanf((const char *)param, "%d %d", &side, &level);
    LOG_BLE("shell level side %d, level %d", side, level);

    wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm && wm->post_window_event);
    window_id = ble_shell_get_level_speed_window_id(wm);
    if (window_id == ID_WINDOW_INVALID) {
        LOG_BLE("not home/pump window, ignore level adjust.");
        return;
    }

    data = u_malloz(sizeof(*data));
    EXCEPTION_CHECK_VOID(data);
    data->side = (uint8_t)side;
    data->level = (uint8_t)level;
    wm->post_window_event(__func__, window_id, WINDOW_ASYNC_EVT_LEVEL_ADJUST,
        data, WINDOW_EVT_FLAG_FREE_USER_DATA);
}

/// #pumpSpd <speed>（home/pump 当前窗口）
static void ble_shell_pump_speed(const uint8_t *param, uint16_t len)
{
    int speed = 0;
    window_manager_context_t *wm;
    window_evt_speed_t *data;
    uint8_t window_id;

    sscanf((const char *)param, "%d", &speed);
    LOG_BLE("shell speed %d", speed);

    wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm && wm->post_window_event);
    window_id = ble_shell_get_level_speed_window_id(wm);
    if (window_id == ID_WINDOW_INVALID) {
        LOG_BLE("not home/pump window, ignore speed adjust.");
        return;
    }

    data = u_malloz(sizeof(*data));
    EXCEPTION_CHECK_VOID(data);
    data->speed = (uint8_t)speed;
    wm->post_window_event(__func__, window_id, WINDOW_ASYNC_EVT_SPEED_ADJUST,
        data, WINDOW_EVT_FLAG_FREE_USER_DATA);
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
    motor->hook->motor_debug_with_user_para(motor, para.duty == 0 ?
        MOTOR_USER_DEBUG_VALVE_OFF : MOTOR_USER_DEBUG_VALVE_ON, &para);
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
    para.pump_level = pump_level * PUMP_DUTY_SCALE_FACTOR;
    para.pumping_time = pumping_time;
    para.valve_on_time = valve_on_time;
    para.sleep_time = sleep_time;
    para.hold_time = hold_time;
    motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_DEBUG_POLICY, &para);
}

static void ble_shell_system_reset(const uint8_t *param, uint16_t len)
{
    int reset_type = 0;
    sscanf((const char *)param, "%d", &reset_type);
    LOG_BLE("shell system reset type %d", reset_type);
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_VOID(wm);
    multi_func_msg_data_t msg = {0};
    msg.union_body.extra.window_id = ID_WINDOW_FACTORY_RESET;
    msg.union_body.extra.arg1 = reset_type ? 1 : 0;
    wm->dispatch_new_window_args(wm, &msg);
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

        if (memcmp(param, shell_list[i].cmd_str, str_len) != 0) {
            continue;
        }
        /// 命令边界：结束或空白，避免 "#pump" 误匹配 "#pumpLvl"
        if (str_len < len && param[str_len] != ' ' && param[str_len] != '\t' &&
            param[str_len] != '\r' && param[str_len] != '\n' && param[str_len] != '\0') {
            continue;
        }

        shell_list[i].cmd_fun(param + str_len, len - str_len);
        return 0;
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
#if IS_SUPPORT_ALGO_SAMPLING
    algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    int control_type = 0;
    sscanf((const char *)param, "%d", &control_type);
    LOG_BLE("shell algo sampling control");
    if (control_type) {
        algo->hook->algo_sampling_start(algo);
    } else {
        algo->hook->algo_sampling_stop(algo);
    }
#endif
}

static void ble_shell_algo_sampling_interval(const uint8_t *param, uint16_t len)
{
#if IS_SUPPORT_ALGO_SAMPLING
    algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    int interval = 0;
    sscanf((const char *)param, "%d", &interval);
    LOG_BLE("shell algo sampling interval %d", interval);
    algo->hook->algo_change_sampling_interval(algo, interval);
#endif
}

static void ble_shell_night_light_adaptive(const uint8_t *param, uint16_t len)
{
    int adaptive = 0;
    sscanf((const char *)param, "%d", &adaptive);
    LOG_BLE("shell night light adaptive %d", adaptive);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook->set_night_light_adaptive_dimming_switch) {
        led->hook->set_night_light_adaptive_dimming_switch(led, adaptive); /// 设置夜灯自适应开关.
    }
#endif
}

static void ble_shell_night_light_switch(const uint8_t *param, uint16_t len)
{
    int data = 0;
    sscanf((const char *)param, "%d", &data);
    LOG_BLE("shell night light switch %d", data);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook->set_night_light_switch) {
        led->hook->set_night_light_switch(led, data); /// 设置夜灯开关.
    }
#endif
}

static void ble_shell_global_adaptive_dimming_switch(const uint8_t *param, uint16_t len)
{
    int data = 0;
    sscanf((const char *)param, "%d", &data);
    LOG_BLE("shell global adaptive dimming switch %d", data);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
    if (led->hook->set_global_adaptive_dimming_switch) {
        led->hook->set_global_adaptive_dimming_switch(led, data); /// 设置全局自适应开关.
    }
}

static void ble_shell_night_light_gear(const uint8_t *param, uint16_t len)
{
    int gear = 0;
    sscanf((const char *)param, "%d", &gear);
    LOG_BLE("shell night light gear %d", gear);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook->set_night_light_gear) {
        led->hook->set_night_light_gear(led, gear); /// 设置夜灯档位.
    }   
#endif
}

static void ble_shell_night_light_level(const uint8_t *param, uint16_t len)
{
    int level = 0;
    sscanf((const char *)param, "%d", &level);
    LOG_BLE("shell night light level %d", level);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
#if IS_SUPPORT_NIGHT_LIGHT
    if (led->hook->set_night_light_level) {
        led->hook->set_night_light_level(led, level); /// 设置夜灯亮度.
    }   
#endif
}

static void ble_shell_back_light_level(const uint8_t *param, uint16_t len)
{
    int level = 0;
    sscanf((const char *)param, "%d", &level);
    LOG_BLE("shell back light level %d", level);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
    if (led->hook->set_key_backlight_level) {
        led->hook->set_key_backlight_level(led, level); /// 设置背光亮度.
    }

    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    EXCEPTION_CHECK_VOID(ui && ui->hook);   
    if (ui->hook->set_ui_backlight_level) {
        ui->hook->set_ui_backlight_level(ui, level); /// 设置屏背光亮度.
    }
}

static void ble_shell_lcd_backlight(const uint8_t *param, uint16_t len)
{
    int backlight = 0;
    sscanf((const char *)param, "%d", &backlight);
    LOG_BLE("shell lcd backlight:%d", backlight);
    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    EXCEPTION_CHECK_VOID(ui && ui->hook);
    ui->hook->set_ui_backlight_level(ui, backlight);
}

static void ble_shell_led_night_light(const uint8_t *param, uint16_t len)
{
#if IS_SUPPORT_NIGHT_LIGHT
    int night_light = 0;
    sscanf((const char *)param, "%d", &night_light);
    LOG_BLE("shell led night light pwm duty %d", night_light);
    led_manager_context_t *led = (led_manager_context_t *)find_module(ID_MODULE_LED);
    EXCEPTION_CHECK_VOID(led && led->hook);
    if (led->hook->set_night_light_adaptive_dimming_switch) {
        led->hook->set_night_light_adaptive_dimming_switch(led, 0); /// 关闭夜灯自适应调光.
    }

    if (led->hook->set_night_light_level) {
        led->hook->set_night_light_level(led, night_light); /// 设置夜灯亮度.
    }   
#endif
}

static void ble_shell_adapte_light_sampling_control(const uint8_t *param, uint16_t len)
{
#if IS_SUPPORT_ADAPTIVE_DIMMING
    int control_type = 0;
    sscanf((const char *)param, "%d", &control_type);
    LOG_BLE("shell light sensor sampling control: %d", control_type);
    if (control_type) {
        light_sensor_sampling_start();
    } else {
        light_sensor_sampling_stop();
    }
#endif
}

static void ble_shell_adapte_light_sampling_interval(const uint8_t *param, uint16_t len)
{
    int interval = 0;
    sscanf((const char *)param, "%d", &interval);
    LOG_BLE("shell algo sampling interval %d", interval);
    light_sensor_change_sampling_interval(interval);
}

static void ble_shell_charge_current_adjust(const uint8_t *param, uint16_t len)
{
    uint32_t current = 0;
    sscanf((const char *)param, "%d", &current);
    LOG_BLE("shell charge current adjust %d", current);
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    rt_device_node_t *battery = pbus->find_device(ID_DEVICE_BATTERY);
    EXCEPTION_CHECK_VOID(battery);
    battery->ioctl(battery, OP_CODE_BAT_CHARGE_CURRENT_ADJUST, &current);
}

static void ble_shell_utc_time(const uint8_t *param, uint16_t len)
{
    uint32_t utc_time = 0;
    int time_zone = 0;
    sscanf((const char *)param, "%u %d", &utc_time, &time_zone);
    LOG_BLE("shell utc time %u, time_zone %d", utc_time, time_zone);
    time_system_t *time = (time_system_t *)find_module(ID_MODULE_TIME);
    EXCEPTION_CHECK_VOID(time);
    if (abs(time_zone) <= 12) {
        int64_t adjusted_time = (int64_t)utc_time + (int64_t)time_zone * 3600;
        if (adjusted_time >= 0 && adjusted_time <= UINT32_MAX) {
            utc_time = (uint32_t)adjusted_time;
        } else {
            LOGE("shell utc time overflow, utc %u, tz %d", utc_time, time_zone);
            return;
        }
    }
    time->sync_utc_time(utc_time);
}

static void ble_shell_custom_mode_step_info_update(const uint8_t *param, uint16_t len)
{
    LOG_BLE("shell custom mode step info update");
    uint32_t id = 0;
    sscanf((const char *)param, "%d", &id);
    storage_manager_context_t *storage = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage);
    pump_mode_steps_info_t *steps_info = u_malloz(sizeof(pump_mode_steps_info_t));
    EXCEPTION_CHECK_VOID(steps_info);
    storage->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, steps_info);
    steps_info->id.dat32 = id;
    msleep(10);
    storage->put_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, steps_info);
    msleep(10);
    memset(steps_info, 0, sizeof(pump_mode_steps_info_t));
    storage->get_store_ram_val(STORE_ID_PUMP_MODE_STEPS_INFO, BREAST_PUMP_CUSTOM_MODE, 0, steps_info);
    LOG_BLE("shell custom mode steps info: %d", steps_info->id.dat32);
    msleep(10);
    char mode_name[PUMP_MODE_NAME_LEN];   /// 模式名称
    snprintf(mode_name, sizeof(mode_name), "%d", id % 100);
    storage->put_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE, 0, mode_name);
    msleep(10);
    memset(mode_name, 0, PUMP_MODE_NAME_LEN);
    storage->get_store_ram_val(STORE_ID_PUMP_MODE_NAME_INFO, BREAST_PUMP_CUSTOM_MODE, 0, mode_name);
    LOG_BLE("shell custom mode id: %s", mode_name);
    msleep(10);
    storage->put_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, BREAST_PUMP_CUSTOM_MODE, true, 0);
    u_free(steps_info);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    WRITE_PUMP_MODE_ACTIVATE(pump_ctx, BREAST_PUMP_CUSTOM_MODE, 1);
    int activate_status = 0;
    READ_PUMP_MODE_ACTIVATE(pump_ctx, BREAST_PUMP_CUSTOM_MODE, activate_status);
    LOG_BLE("shell custom mode step info update activate status: %d", activate_status);
    LOG_BLE("shell custom mode step info update done");
}

static void ble_shell_custom_mode_step_info_clear(const uint8_t *param, uint16_t len)
{
    LOG_BLE("shell custom mode step info clear");
    storage_manager_context_t *storage = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage);
    storage->put_store_ram_val(STORE_ID_MODE_NEW_STEP_INFO_FLAG, BREAST_PUMP_CUSTOM_MODE, false, 0);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    WRITE_PUMP_MODE_ACTIVATE(pump_ctx, BREAST_PUMP_CUSTOM_MODE, 0);
    LOG_BLE("shell custom mode step info clear done");
}

static void ble_shell_triple_set_field(uint8_t *dst, uint8_t *dst_len, uint16_t max_len, const char *src)
{
    uint16_t copy_len = (uint16_t)strlen(src);
    if (copy_len > max_len) {
        copy_len = max_len;
    }
    memset(dst, 0, max_len);
    memcpy(dst, src, copy_len);
    *dst_len = (uint8_t)copy_len;
}

static void ble_shell_triple_write(const uint8_t *param, uint16_t len)
{
    int index = 0;
    if (len > 0) {
        sscanf((const char *)param, "%d", &index);
    }
    if (index < 0 || (uint32_t)index >= LENGTH_OF_ARRAY(triple_preset_list)) {
        LOGE("shell triple write invalid index %d, max %u", index,
             (unsigned)(LENGTH_OF_ARRAY(triple_preset_list) - 1));
        return;
    }

    const ble_factory_triple_preset_t *preset = &triple_preset_list[index];
    storage_manager_context_t *storage = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(storage);

    factory_dev_info_t dev_info = {0};
    storage->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
    ble_shell_triple_set_field(dev_info.product_id, &dev_info.product_id_len,
                               DEVICE_PRODUCT_ID_LEN, preset->product_key);
    ble_shell_triple_set_field(dev_info.device_id, &dev_info.device_id_len,
                               DEVICE_DEVICE_ID_LEN, preset->device_name);
    ble_shell_triple_set_field(dev_info.enctyption_key, &dev_info.enctyption_key_len,
                               DEVICE_ENCTYPTION_KEY_LEN, preset->device_secret);
    ble_shell_triple_set_field(dev_info.sn, &dev_info.sn_len,
                               DEVICE_UNIQUE_SN_LEN, preset->sn);

    int ret = storage->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
    LOG_BLE("shell triple write idx=%d mac=%s ret=%d", index, preset->mac, ret);
    LOG_BLE("shell triple pk=%s dn=%s ds=%s sn=%s",
            preset->product_key, preset->device_name, preset->device_secret, preset->sn);
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
