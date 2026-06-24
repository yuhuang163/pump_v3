/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "root_common_api.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep
#include "data_bury_persist_defs.h" // IWYU pragma: keep

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

typedef struct {
    servers_publish_data_id_e id;
    void *val;
    int arg1;
    const char *name;
} data_bury_pressure_test_case_t;

static void data_bury_pressure_test_init_payloads(
    uint32_t sid, uint32_t ts,
    data_bury_evt_pump_startup_t *startup,
    data_bury_evt_pump_shutdown_t *shutdown,
    data_bury_evt_bt_connect_t *bt_connect,
    data_bury_evt_pump_session_start_t *session_start,
    uint8_t *session_end_buf,
    data_bury_evt_pump_manual_adjust_t *manual_adjust,
    data_bury_evt_pump_user_evt_t *user_evt,
    data_bury_evt_milk_ejection_t *milk_ejection,
    data_bury_evt_bat_charge_start_t *charge_start,
    data_bury_evt_bat_charge_end_t *charge_end,
    data_bury_evt_exception_t *exception,
    data_bury_evt_firmware_update_t *fw_update,
    data_bury_evt_overflow_t *overflow,
    uint8_t *milk_volume_buf,
    data_bury_evt_heat_session_start_t *heat_start,
    data_bury_evt_heat_session_end_t *heat_end,
    data_bury_evt_massage_session_start_t *massage_start,
    data_bury_evt_massage_session_end_t *massage_end,
    data_bury_evt_wear_detection_t *wear)
{
    data_bury_evt_pump_session_end_fixed_t *session_end;
    data_bury_evt_milk_volume_fixed_t *milk_volume;
    data_bury_evt_program_duration_t *program_durations;
    uint16_t *volume_samples;
    data_bury_manager_context_t *bury_ctx;
    uint8_t device_side;
    bury_ctx = (data_bury_manager_context_t *)find_module(ID_MODULE_DATA_BURIAL);
    EXCEPTION_CHECK_VOID(bury_ctx);
    EXCEPTION_CHECK_VOID(bury_ctx->data_bury_get_device_side);
    device_side = bury_ctx->data_bury_get_device_side();

    memset(startup, 0, sizeof(*startup));
    startup->device_side = device_side;
    startup->use_session_id = sid;
    startup->evt_timestamp = ts;
    startup->firmware_version = 0x0102u;
    startup->battery_level_startup = 85u;

    memset(shutdown, 0, sizeof(*shutdown));
    shutdown->device_side = device_side;
    shutdown->use_session_id = sid;
    shutdown->evt_timestamp = ts;
    shutdown->battery_level_shutdown = 80u;
    shutdown->shutdown_reason = DATA_BURY_SHUTDOWN_USER_MANUAL;

    memset(bt_connect, 0, sizeof(*bt_connect));
    bt_connect->device_side = device_side;
    bt_connect->use_session_id = sid;
    bt_connect->evt_timestamp = ts;
    bt_connect->connect_result = 1u;
    bt_connect->connection_setup_duration = 320u;
    bt_connect->bt_signal_strength_rssi = -55;
    bt_connect->disconnect_timestamp = ts + 60000u;
    bt_connect->disconnect_reason = 0u;

    memset(session_start, 0, sizeof(*session_start));
    session_start->device_side = device_side;
    session_start->use_session_id = sid;
    session_start->evt_timestamp = ts;
    session_start->pump_session_id = 1001u;
    session_start->initial_mode_type = 1u;
    session_start->initial_suction_level = 0u;
    session_start->initial_suction_level_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(3u, 3u);
    session_start->initial_attitude_pitch_roll = 0x00010002u;
    session_start->initial_volume_ml = 0u;
    session_start->initial_battery_level = 84u;

    memset(session_end_buf, 0, DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(2u));
    session_end = (data_bury_evt_pump_session_end_fixed_t *)session_end_buf;
    session_end->device_side = device_side;
    session_end->use_session_id = sid;
    session_end->evt_timestamp = ts;
    session_end->pump_session_id = 1001u;
    session_end->final_mode_type = 2u;
    session_end->final_suction_level = 0u;
    session_end->final_attitude_pitch_roll = 0x00020003u;
    session_end->final_volume_ml = 120u;
    session_end->final_battery_level = 82u;
    session_end->end_reason = DATA_BURY_END_REASON_MANUAL;
    session_end->milk_drop_count = 2u;
    session_end->used_mode_count = 2u;
    session_end->final_suction_level_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(4u, 4u);
    program_durations = (data_bury_evt_program_duration_t *)(session_end_buf +
        DATA_BURY_EVT_PUMP_SESSION_END_PROGRAM_OFF);
    program_durations[0].mode_or_program_id = 1u;
    program_durations[0].duration_sec = 60u;
    program_durations[1].mode_or_program_id = 2u;
    program_durations[1].duration_sec = 120u;

    memset(manual_adjust, 0, sizeof(*manual_adjust));
    manual_adjust->device_side = device_side;
    manual_adjust->use_session_id = sid;
    manual_adjust->evt_timestamp = ts;
    manual_adjust->pump_session_id = 1001u;
    manual_adjust->evt_source = DATA_BURY_EVT_SOURCE_LOCAL;
    manual_adjust->evt_type = DATA_BURY_ADJUST_SUCTION_LEVEL;
    manual_adjust->adjust_from_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(3u, 3u);
    manual_adjust->adjust_to_desktop = DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(4u, 4u);

    memset(user_evt, 0, sizeof(*user_evt));
    user_evt->device_side = device_side;
    user_evt->use_session_id = sid;
    user_evt->evt_timestamp = ts;
    user_evt->pump_session_id = 1001u;
    user_evt->evt_source = DATA_BURY_EVT_SOURCE_LOCAL;
    user_evt->evt_type = DATA_BURY_USER_EVT_START;

    memset(milk_ejection, 0, sizeof(*milk_ejection));
    milk_ejection->device_side = device_side;
    milk_ejection->use_session_id = sid;
    milk_ejection->evt_timestamp = ts;
    milk_ejection->pump_session_id = 1001u;
    milk_ejection->detect_times_no = 1u;
    milk_ejection->start_mls = 30u;
    milk_ejection->start_timestamp = ts;
    milk_ejection->end_mls = 45u;
    milk_ejection->end_timestamp = ts + 5000u;

    memset(charge_start, 0, sizeof(*charge_start));
    charge_start->device_side = device_side;
    charge_start->use_session_id = sid;
    charge_start->evt_timestamp = ts;
    charge_start->start_battery_percent = 20u;
    charge_start->start_battery_voltage = 3600u;
    charge_start->start_battery_current = 500;
    charge_start->start_battery_temp = 25;

    memset(charge_end, 0, sizeof(*charge_end));
    charge_end->device_side = device_side;
    charge_end->use_session_id = sid;
    charge_end->evt_timestamp = ts;
    charge_end->end_battery_percent = 100u;
    charge_end->end_battery_voltage = 4200u;
    charge_end->end_battery_current = 0;
    charge_end->end_battery_temp = 28;
    charge_end->end_reason = DATA_BURY_BAT_CHARGE_END_REASON_FULL;

    memset(exception, 0, sizeof(*exception));
    exception->device_side = device_side;
    exception->use_session_id = sid;
    exception->evt_timestamp = ts;
    exception->error_code = 0x1001u;
    exception->error_data = 0xDEADBEEFu;

    memset(fw_update, 0, sizeof(*fw_update));
    fw_update->device_side = device_side;
    fw_update->use_session_id = sid;
    fw_update->evt_timestamp = ts;
    fw_update->update_from = 0x0100u;
    fw_update->update_result = DATA_BURY_FIRMWARE_UPDATE_RESULT_SUCCESS;
    fw_update->update_err = 0u;

    memset(overflow, 0, sizeof(*overflow));
    overflow->device_side = device_side;
    overflow->use_session_id = sid;
    overflow->evt_timestamp = ts;
    overflow->pump_session_id = 1001u;
    overflow->milk_volume_ml = 150u;
    overflow->real_time_attitude_pitch_roll = 0x00030004u;

    memset(milk_volume_buf, 0, DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN + 6u);
    milk_volume = (data_bury_evt_milk_volume_fixed_t *)milk_volume_buf;
    milk_volume->device_side = device_side;
    milk_volume->use_session_id = sid;
    milk_volume->evt_timestamp = ts;
    milk_volume->pump_session_id = 1001u;
    milk_volume->sampling_count = 3u;
    volume_samples = (uint16_t *)(milk_volume_buf + DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN);
    volume_samples[0] = 10u;
    volume_samples[1] = 25u;
    volume_samples[2] = 40u;

    memset(heat_start, 0, sizeof(*heat_start));
    heat_start->device_side = device_side;
    heat_start->use_session_id = sid;
    heat_start->evt_timestamp = ts;
    heat_start->heat_session_id = 2001u;
    heat_start->target_level = 2u;
    heat_start->initial_temp = 36u;

    memset(heat_end, 0, sizeof(*heat_end));
    heat_end->device_side = device_side;
    heat_end->use_session_id = sid;
    heat_end->evt_timestamp = ts;
    heat_end->heat_session_id = 2001u;
    heat_end->top_temp = 40u;
    heat_end->final_temp = 38u;

    memset(massage_start, 0, sizeof(*massage_start));
    massage_start->device_side = device_side;
    massage_start->use_session_id = sid;
    massage_start->evt_timestamp = ts;
    massage_start->massage_session_id = 3001u;
    massage_start->initial_level = 1u;

    memset(massage_end, 0, sizeof(*massage_end));
    massage_end->device_side = device_side;
    massage_end->use_session_id = sid;
    massage_end->evt_timestamp = ts;
    massage_end->massage_session_id = 3001u;
    massage_end->final_level = 2u;

    memset(wear, 0, sizeof(*wear));
    wear->device_side = device_side;
    wear->use_session_id = sid;
    wear->evt_timestamp = ts;
    wear->detection_type = 1u;
    wear->detection_data = 0x55AAu;
}

static int data_bury_pressure_test_cb(void)
{
    static uint8_t inited;
    static uint8_t case_idx;
    static data_bury_evt_pump_startup_t evt_startup;
    static data_bury_evt_pump_shutdown_t evt_shutdown;
    static data_bury_evt_bt_connect_t evt_bt_connect;
    static data_bury_evt_pump_session_start_t evt_session_start;
    static uint8_t evt_session_end_buf[DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(2u)];
    static data_bury_evt_pump_manual_adjust_t evt_manual_adjust;
    static data_bury_evt_pump_user_evt_t evt_user_evt;
    static data_bury_evt_milk_ejection_t evt_milk_ejection;
    static data_bury_evt_bat_charge_start_t evt_charge_start;
    static data_bury_evt_bat_charge_end_t evt_charge_end;
    static data_bury_evt_exception_t evt_exception;
    static data_bury_evt_firmware_update_t evt_fw_update;
    static data_bury_evt_overflow_t evt_overflow;
    static uint8_t evt_milk_volume_buf[DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN + 6u];
    static data_bury_evt_heat_session_start_t evt_heat_start;
    static data_bury_evt_heat_session_end_t evt_heat_end;
    static data_bury_evt_massage_session_start_t evt_massage_start;
    static data_bury_evt_massage_session_end_t evt_massage_end;
    static data_bury_evt_wear_detection_t evt_wear;
    static const data_bury_pressure_test_case_t cases[] = {
        { SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP, &evt_startup, 0, "pump_startup" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN, &evt_shutdown, 0, "pump_shutdown" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT, &evt_bt_connect, 0, "bt_connect" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START, &evt_session_start, 0, "session_start" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END, evt_session_end_buf,
          (int)DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(2u), "session_end" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST, &evt_manual_adjust, 0, "manual_adjust" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT, &evt_user_evt, 0, "user_event" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION, &evt_milk_ejection, 0, "milk_ejection" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START, &evt_charge_start, 0, "charge_start" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END, &evt_charge_end, 0, "charge_end" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION, &evt_exception, 0, "exception" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE, &evt_fw_update, 0, "firmware_update" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW, &evt_overflow, 0, "overflow" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME, evt_milk_volume_buf,
          (int)(DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN + 6u), "milk_volume" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_START, &evt_heat_start, 0, "heat_start" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_END, &evt_heat_end, 0, "heat_end" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_START, &evt_massage_start, 0, "massage_start" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_END, &evt_massage_end, 0, "massage_end" },
        { SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION, &evt_wear, 0, "wear_detection" },
    };
    data_bury_manager_context_t *bury_ctx;
    storage_manager_context_t *storage_ctx;
    servers_publish_data_args_t args;
    uint16_t ring_pending = 0;
    int ret;

    bury_ctx = (data_bury_manager_context_t *)find_module(ID_MODULE_DATA_BURIAL);
    if (!bury_ctx || !bury_ctx->data_bury_publish) {
        LOGE("data bury pressure test: bury ctx not ready");
        return 5000;
    }
    if (!bury_ctx->data_bury_is_enable || !bury_ctx->data_bury_is_enable()) {
        LOGW("data bury pressure test: bury disabled, retry later");
        return 5000;
    }

    if (!inited) {
        data_bury_pressure_test_init_payloads(0x20260608u, get_timestamp(),
                                              &evt_startup, &evt_shutdown, &evt_bt_connect, &evt_session_start,
                                              evt_session_end_buf, &evt_manual_adjust, &evt_user_evt,
                                              &evt_milk_ejection, &evt_charge_start, &evt_charge_end,
                                              &evt_exception, &evt_fw_update, &evt_overflow, evt_milk_volume_buf,
                                              &evt_heat_start, &evt_heat_end, &evt_massage_start, &evt_massage_end,
                                              &evt_wear);
        inited = 1u;
        case_idx = 0u;
        LOG_PBUS("data bury pressure test start, cases=%u", (unsigned)LENGTH_OF_ARRAY(cases));
    }

    if (case_idx >= LENGTH_OF_ARRAY(cases)) {
        if (bury_ctx->data_bury_flush_pending) {
            (void)bury_ctx->data_bury_flush_pending();
        }
        storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (storage_ctx) {
            (void)storage_ctx->get_store_ram_val(STORE_ID_DATA_BURY_RING_PENDING_COUNT, 0, 0,
                                                 (void *)&ring_pending);
        }
        LOG_PBUS("data bury pressure test round done, ring_pending=%u", (unsigned)ring_pending);
        case_idx = 0u;
        return 5000;
    }

    args.val = cases[case_idx].val;
    args.arg1 = cases[case_idx].arg1;
    args.arg2 = 0;
    ret = bury_ctx->data_bury_publish(cases[case_idx].id, &args);
    LOG_PBUS("data bury test [%u/%u] %s id=%d arg1=%d ret=%d",
             (unsigned)(case_idx + 1u), (unsigned)LENGTH_OF_ARRAY(cases), cases[case_idx].name,
             (int)cases[case_idx].id, cases[case_idx].arg1, ret);
    case_idx++;
    return 30000;
}

static void hdr_run_pressure_test_cb(hdr_post_args_t *args)
{
    (void)args;
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
    // POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, bottom_key_switch_test_cb(), "pressure_test:continue");
    POST_TASK_HANDLER_DELAYED_ARG0(hdr_run_pressure_test_cb, data_bury_pressure_test_cb(), "pressure_test:continue");
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
