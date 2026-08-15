/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_tests_handler.h"
#include "tests_manager.h"  // IWYU pragma: keep
#include "product_tests.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep
#include "pump_mode_default_config.h" // IWYU pragma: keep
#include "pump_parameters_table.h"

/*
* DEFINES
*****************************************************************************************
*/

#define AGING1_TEST_DEFAULT_CYCLE_TIME               (30*60*1000)// 老化测试默认循环时间，单位毫秒
#define AGING1_TEST_IDLE_TIME                        (10*60*1000)// 老化测试空闲时间，单位毫秒
#define AGING2_TEST_DEFAULT_CYCLE_TIME               (30*60*1000)// 老化测试默认循环时间，单位毫秒
#define AGING2_TEST_IDLE_TIME                        (10*60*1000)// 老化测试空闲时间，单位毫秒
#define AGING_TEST_ENTER_MODE1_TASK_NAME             "factory_tests_aging_test_enter_mode1_task"
#define AGING_TEST_ENTER_MODE2_TASK_NAME             "factory_tests_aging_test_enter_mode2_task"
#define AGING_TEST_EXIT_TASK_NAME                    "factory_tests_aging_test_exit_task"
#define AGING_TEST_ENTER_DELAY_TASK_NAME             "aging_test_cb" /* tests_tlvs 里延迟 1s 真正进入老化 */

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static tests_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int factory_event_printf(factory_tst_event_type_e event)
{
    char *str = "null";
    switch(event)
    {
        case FAC_TST_READ_SW_VER_EVENT: str = "FAC_TST_READ_SW_VER_EVENT"; break;
        case FAC_TST_READ_DEV_MAC_EVENT: str = "FAC_TST_READ_DEV_MAC_EVENT"; break;
        case FAC_TST_MOTOR_CTR_EVENT: str = "FAC_TST_MOTOR_CTR_EVENT"; break;
        // 部分事件不需要打印
        default:
            return -1;
    }
    LOG_TEST("product tst event-- %s = %d", str, event);
    return 0;
}

static int on_tests_event_handler(void *context, uint16_t keycode, uint16_t keytype)
{
    LOG_TEST("keycode = %d, keytype = %d", keycode, keytype);
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_INT(servers_ctx);
    /* 计算按键编号: 将按键bit位索引转为 1-based 的按键编号 */
    uint8_t key_id = 0;
    if (keycode) {
        uint16_t temp = keycode;
        uint8_t index = 0;
        while ((temp & 0x1) == 0 && index < 16) {
            temp >>= 1;
            index++;
        }
        /* key_index_e 从0开始定义，协议按键编号为1起始 */
        key_id = (uint8_t)(index + 1);
    }
    servers_publish_data_args_t args = {
        .arg1 = 0x01, /// 短按
        .arg2 = key_id, 
        .val = NULL,
    };
    uint16_t server_id = SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS;
    switch (keycode) {
    #if V3_UX_VERSION == V3_UX_V3
        case KEY_CODE_STIMULATION_MODE:
        case KEY_CODE_LACTATION_MODE:
        case KEY_CODE_MIX_MODE:
    #elif (V3_UX_VERSION == V3_UX_V2 || V3_UX_VERSION == V3_UX_V4)
        case KEY_CODE_MODE:
        case KEY_CODE_BREASTFEEDING:
        case KEY_CODE_CANCEL:
    #elif V3_UX_VERSION == V3_UX_V6
        case KEY_CODE_STIMULATION_MODE:
        case KEY_CODE_LACTATION_MODE:
        case KEY_CODE_FREQUENCY:
    #else
        case KEY_CODE_MODE:
        case KEY_CODE_BREASTFEEDING:
        case KEY_CODE_FREQUENCY:
    #endif
        case KEY_CODE_POWER:
        case KEY_CODE_START:
        case KEY_CODE_LEFT_CONTROL:
        case KEY_CODE_RIGHT_CONTROL:
            args.arg1 = keytype == KEY_EVENT_CLICK ? 1 : 2;
            server_id = SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS;
            break;
        case KEY_CODE_KNOB_LEFT:
            args.arg1 = 0x01;
            server_id = SERVERS_PUBLISH_DATA_TESTS_ENCODER_STATUS;
            break;
        case KEY_CODE_KNOB_RIGHT:
            args.arg1 = 0x02;
            server_id = SERVERS_PUBLISH_DATA_TESTS_ENCODER_STATUS;
            break;
        default:
            return 0;
    }
    servers_ctx->hook->factory_server_data_publish(server_id, &args);
    return 0;
}

static void factory_tests_aging_test_cancel_pending_tasks(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(AGING_TEST_ENTER_DELAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(AGING_TEST_ENTER_MODE1_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(AGING_TEST_ENTER_MODE2_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(AGING_TEST_EXIT_TASK_NAME);
}

static int factory_tests_aging_test_enter_mode1(void)
{
    int ret = 0;
    LOG_TEST("factory test aging test enter mode1");
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_INT(motor);
    motor_policy_original_para_t para = {0};
    factory_suction_para_t *suction_para = NULL;
    if (!motor->get_local_suction_table || !motor->get_local_suction_table(&suction_para)) {
        LOG_TEST("suction_table is null ");
        return -1;
    }
    /// 初始化当前循环工作时间
    thiz->priv.factory_ctx.aging_result_info.time_offset = get_timestamp();
    thiz->priv.factory_ctx.aging_result_info.aging_run_status = AGING_RUN_STATUS_WORK;
    thiz->priv.factory_ctx.aging_result_info.start_time = get_timestamp();

    //吸乳模式15档，速度中，参数
    int pump_level = 0, pumping_time = 0, valve_on_time = 0, sleep_time = 0, hold_time = 0;
#if IS_USE_SPECTRA_SUCTION_CURVE
    const suction_parameters_t *factory_para =
        &suction_para->long_suction_table[BREAST_PUMP_LEVEL_15][SUCTION_SPEED_MID];
    pump_level   = factory_para->pump_level;
    pumping_time = factory_para->pumping_time;
    valve_on_time = factory_para->valve_on_time;
    hold_time    = factory_para->hold_time;
    uint16_t total_time = ONE_MIN_TIME_MS / suction_para->long_pump_freq_table[SUCTION_SPEED_MID];
#else
    pump_level = suction_para->long_suction_table[long_level_table[BREAST_PUMP_LEVEL_15]].pump_level;
    pumping_time = suction_para->long_suction_table[long_level_table[BREAST_PUMP_LEVEL_15]].pumping_time;
    valve_on_time = suction_para->long_suction_table[long_level_table[BREAST_PUMP_LEVEL_15]].valve_on_time;
    hold_time = LONG_SUCTION_HOLD_TIME;
    uint16_t total_time = ONE_MIN_TIME_MS / suction_para->long_pump_freq_table[BREAST_PUMP_LEVEL_15][SUCTION_SPEED_MID];
#endif
    sleep_time = total_time - pumping_time - valve_on_time - hold_time;
    if (sleep_time < PUMP_SLEEP_TIME_MIN || sleep_time > PUMP_SLEEP_TIME_MAX) {
        sleep_time = PUMP_SLEEP_TIME_MIN;
        LOG_TEST("work period error");
    }
    para.pump_level = pump_level * PUMP_DUTY_SCALE_FACTOR;
    para.pumping_time = pumping_time;
    para.valve_on_time = valve_on_time;
    para.sleep_time = sleep_time;
    para.hold_time = hold_time;
    LOG_TEST("factory test aging test enter mode1: pump_level = %d, pumping_time = %d, valve_on_time = %d, sleep_time = %d, hold_time = %d",
        pump_level, pumping_time, valve_on_time, sleep_time, hold_time);
    motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_DEBUG_POLICY, &para);
    return ret;
}

static int factory_tests_aging_test_enter_mode2(void)
{
    int ret = 0;
    LOG_TEST("factory test aging test enter mode2");
    /// 初始化当前循环工作时间
    thiz->priv.factory_ctx.aging_result_info.time_offset = get_timestamp();
    thiz->priv.factory_ctx.aging_result_info.aging_run_status = AGING_RUN_STATUS_WORK;
    thiz->priv.factory_ctx.aging_result_info.start_time = get_timestamp();
    return ret;
}

static void factory_tests_aging_test_enter_mode1_post_cb(hdr_post_args_t *args)
{
    (void)args;
    (void)factory_tests_aging_test_enter_mode1();
}

static void factory_tests_aging_test_enter_mode2_post_cb(hdr_post_args_t *args)
{
    (void)args;
    (void)factory_tests_aging_test_enter_mode2();
}

static void factory_tests_aging_test_exit_post_cb(hdr_post_args_t *args)
{
    tests_func_args_t exit_args = {0};

    (void)args;
    exit_args.ctx = thiz;
    (void)factory_tests_aging_test_exit(exit_args);
}

static void factory_tests_aging_test_check_result(void)
{
    LOG_TEST("factory test aging test check result");
    factory_flag_info_t flag_info = {0};
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    EXCEPTION_CHECK_VOID(thiz);
    //暂停电机
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_VOID(motor);

    //获取tick时间处理老化结果，如果老化结果成功，则退出老化测试，如果老化结果失败，则继续老化测试
    if (thiz->priv.current_mode == TST_AGING_TEST_TYPE
        && thiz->priv.factory_ctx.aging_target_info.aging_type == AGING_MODE_TYPE_1) {
        if (thiz->priv.factory_ctx.aging_result_info.aging_status == AGING_TEST_STATUS_SUCCESS) {
            return;
        }
        if (thiz->priv.factory_ctx.aging_result_info.aging_run_status == AGING_RUN_STATUS_IDLE ||
             thiz->priv.factory_ctx.aging_result_info.aging_run_status == AGING_RUN_STATUS_END) {
            return;
        }
        /// 先补齐本次检查前的工作时长，避免周期边界漏记
        thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time
            += get_timestamp_ms(thiz->priv.factory_ctx.aging_result_info.time_offset);
        thiz->priv.factory_ctx.aging_result_info.time_offset = get_timestamp();
        /// 循环完成，进入下一个循环
        if (get_timestamp_ms(thiz->priv.factory_ctx.aging_result_info.start_time) > AGING1_TEST_DEFAULT_CYCLE_TIME) {
            thiz->priv.factory_ctx.aging_result_info.start_time = get_timestamp();
            st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
            motor_debug_para_t para = {0};
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_DEBUG_PUMP_OFF, &para);
            flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_time += AGING1_TEST_DEFAULT_CYCLE_TIME / 1000;
            flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_cycle++;
            if (flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_time
                >= thiz->priv.factory_ctx.aging_target_info.total_target_time) {
                flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_flag = AGING_TEST_STATUS_SUCCESS;
                thiz->priv.factory_ctx.aging_result_info.aging_status = AGING_TEST_STATUS_SUCCESS;
                thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time = 0;
                POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_exit_post_cb, 1000,
                    AGING_TEST_EXIT_TASK_NAME);
            } else {
                flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_flag = AGING_TEST_STATUS_FAILED;
                thiz->priv.factory_ctx.aging_result_info.aging_status = AGING_TEST_STATUS_FAILED;
                thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time = 0;
                thiz->priv.factory_ctx.aging_result_info.aging_run_status = AGING_RUN_STATUS_IDLE;
                POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_enter_mode1_post_cb, AGING1_TEST_IDLE_TIME,
                    AGING_TEST_ENTER_MODE1_TASK_NAME);
            }
            st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
            LOG_TEST("factory test aging test check result (MODE_1): age_flag = "
                     "%d, age_time = %d, age_cycle = %d",
                     flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_flag,
                     flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_time,
                     flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_cycle);
        }
    } else if (thiz->priv.current_mode == TST_AGING_TEST_TYPE
               && thiz->priv.factory_ctx.aging_target_info.aging_type == AGING_MODE_TYPE_2) {
        if (thiz->priv.factory_ctx.aging_result_info.aging_status == AGING_TEST_STATUS_SUCCESS) {
            return;
        }
        if (thiz->priv.factory_ctx.aging_result_info.aging_run_status == AGING_RUN_STATUS_IDLE ||
             thiz->priv.factory_ctx.aging_result_info.aging_run_status == AGING_RUN_STATUS_END) {
            return;
        }
        /// 先补齐本次检查前的工作时长，避免周期边界漏记
        thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time
            += get_timestamp_ms(thiz->priv.factory_ctx.aging_result_info.time_offset);
        thiz->priv.factory_ctx.aging_result_info.time_offset = get_timestamp();
        /// 循环完成，进入下一个循环
        if (get_timestamp_ms(thiz->priv.factory_ctx.aging_result_info.start_time) > AGING2_TEST_DEFAULT_CYCLE_TIME) {
            thiz->priv.factory_ctx.aging_result_info.start_time = get_timestamp();
            st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
            motor_debug_para_t para = {0};
            motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_DEBUG_PUMP_OFF, &para);
            flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_time += AGING2_TEST_DEFAULT_CYCLE_TIME / 1000;
            flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_cycle++;
            if (flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_time
                >= thiz->priv.factory_ctx.aging_target_info.total_target_time) {
                flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_flag = AGING_TEST_STATUS_SUCCESS;
                thiz->priv.factory_ctx.aging_result_info.aging_status = AGING_TEST_STATUS_SUCCESS;
                thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time = 0;
                POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_exit_post_cb, 1000,
                    AGING_TEST_EXIT_TASK_NAME);
            } else {
                flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_flag = AGING_TEST_STATUS_FAILED;
                thiz->priv.factory_ctx.aging_result_info.aging_status = AGING_TEST_STATUS_FAILED;
                thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time = 0;
                thiz->priv.factory_ctx.aging_result_info.aging_run_status = AGING_RUN_STATUS_IDLE;
                POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_enter_mode2_post_cb, AGING2_TEST_IDLE_TIME,
                    AGING_TEST_ENTER_MODE2_TASK_NAME);
            }
            st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
            LOG_TEST("factory test aging test check result (MODE_2): age_flag = "
                     "%d, age_time = %d, age_cycle = %d",
                     flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_flag,
                     flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_time,
                     flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_cycle);
        }
    } else {
        LOG_TEST("factory test aging test check result: current_mode = %d, "
                 "aging_type = %d, not in aging test",
                 thiz ? thiz->priv.current_mode : -1,
                 thiz ? thiz->priv.factory_ctx.aging_target_info.aging_type : -1);
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int get_factory_tests_sw_ver(tests_func_args_t args)
{
    int ret = 0;
    char *sw_ver = "1.0.0";
    EXCEPTION_CHECK_INT(args.val);
    strncpy((char *)args.val, sw_ver, strlen(sw_ver) + 1);
    LOG_TEST("factory test get sw ver: %s", (char *)args.val);

    //test code
    multi_func_msg_data_t msg = {0};
    msg.union_body.product_test_extra.event_type = FAC_TST_READ_SW_VER_EVENT;
    EXCEPTION_CHECK_INT(thiz);
    MSG_QUEUE_SEND(thiz->priv.tests_thd.msg_id, &msg, 0);
    LOG_TEST("factory test ver event sent.");
    return ret;
}

int factory_tests_enter(tests_func_args_t args)
{
    LOG_TEST("factory test common enter test");
    key_input_context_t *key_ctx = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_INT(key_ctx);
    power_manager_context_t *power_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_INT(power_ctx);
    CHECK_IF_VALID(power_ctx, {
        power_ctx->acquire_wakelock("factory_tests", -1);
    });
    ADD_KEYEVENT_CB_WITH_SHIELD_LEVEL(key_ctx, "factory_tests", thiz, (ALL_KEY_CODE_NO_COMBINE),
        (KEY_EVENT_CLICK | KEY_EVENT_LONG_2S), on_tests_event_handler, KEY_HANDLER_SHIELD_FORBIDDEN);
    return 0;
}

int factory_tests_exit(tests_func_args_t args)
{
    LOG_TEST("factory test aging test exit");
    key_input_context_t *key_ctx = (key_input_context_t *)find_module(ID_MODULE_KEY_INPUT);
    EXCEPTION_CHECK_INT(key_ctx);
    power_manager_context_t *power_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_INT(power_ctx);
    CHECK_IF_VALID(power_ctx, {
        power_ctx->release_wakelock("factory_tests");
    });
    REMOVE_KEYEVENT_CB(key_ctx, on_tests_event_handler);
    return 0;
}

/// 获取老化时间的包装函数（用于事件处理接口）
int get_factory_tests_aging_test_current_age_time(tests_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.val);
    uint32_t current_cycle_work_time = thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time / 1000;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(st);
    factory_flag_info_t flag_info = {0};
    uint8_t aging_mode_type = (uint8_t)args.arg1;
    if (aging_mode_type != AGING_MODE_TYPE_1 && aging_mode_type != AGING_MODE_TYPE_2) {
        aging_mode_type = (uint8_t)thiz->priv.factory_ctx.aging_target_info.aging_type;
    }
    if (aging_mode_type != AGING_MODE_TYPE_1 && aging_mode_type != AGING_MODE_TYPE_2) {
        aging_mode_type = AGING_MODE_TYPE_1;
    }
    st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    uint32_t age_time = flag_info.age_info[aging_mode_type - 1].age_time;
    /// 仅当查询模式与当前运行老化模式一致时，叠加当前循环工作时间
    if ((thiz->priv.current_mode == TST_AGING_TEST_TYPE)
        && (thiz->priv.factory_ctx.aging_target_info.aging_type == aging_mode_type)) {
        age_time += current_cycle_work_time;
    }
    *(uint32_t *)args.val = age_time;
    LOG_TEST("factory test get current age time: mode = %u, age_time = %lu",
             aging_mode_type, (unsigned long)age_time);
    return ret;
}

int factory_tests_aging_test_enter(tests_func_args_t args)
{
    /* 自动循环工作N小时（按摩→吸乳→混合，1-15档循环） */
    int ret = 0;
    memset(&thiz->priv.factory_ctx.aging_target_info, 0, sizeof(aging_target_info_t));
    memset(&thiz->priv.factory_ctx.aging_result_info, 0, sizeof(aging_result_info_t));
    thiz->priv.factory_ctx.aging_result_info.start_time = get_timestamp();
    thiz->priv.factory_ctx.aging_result_info.time_offset = get_timestamp();
    thiz->priv.factory_ctx.aging_target_info.total_target_time = (uint32_t)args.arg2; /// 老化目标时长
    thiz->priv.factory_ctx.aging_target_info.aging_type = (uint16_t)args.arg1;        /// 老化模式类型
    thiz->priv.factory_ctx.aging_result_info.aging_run_status = AGING_RUN_STATUS_IDLE;
    LOG_TEST("factory test aging test: total_target_time = %d, aging_type = %d",
         thiz->priv.factory_ctx.aging_target_info.total_target_time,
         thiz->priv.factory_ctx.aging_target_info.aging_type);
    power_manager_context_t *power_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_INT(power_ctx);
    CHECK_IF_VALID(power_ctx, {
            power_ctx->acquire_wakelock("factory_tests_aging_test", -1); });
    // 开始老化测试 
    switch (thiz->priv.factory_ctx.aging_target_info.aging_type) {
        case AGING_MODE_TYPE_1:
            ret = factory_tests_aging_test_enter_mode1();
            break;
        case AGING_MODE_TYPE_2: 
            ret = factory_tests_aging_test_enter_mode2();
            break;
        default:
            ret = -1;
            LOG_TEST("factory test aging test: aging_type = %d, invalid",
                 thiz->priv.factory_ctx.aging_target_info.aging_type);
            break;
    }
    return ret;
}

int factory_tests_aging_test_exit(tests_func_args_t args)
{
    (void)args;
    LOG_TEST("factory test aging test exit");
    /// 退出老化时，若休息再启/延迟退出任务还在，直接取消
    factory_tests_aging_test_cancel_pending_tasks();
    if (thiz == NULL) {
        return -1;
    }
    thiz->priv.factory_ctx.aging_result_info.aging_run_status = AGING_RUN_STATUS_END;
    power_manager_context_t *power_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_INT(power_ctx);
    CHECK_IF_VALID(power_ctx, {
        power_ctx->release_wakelock("factory_tests_aging_test");
    });
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(st);
    motor_manager_context_t *motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_INT(motor);
    motor_debug_para_t para = {0};
    motor->hook->motor_debug_with_user_para(motor, MOTOR_USER_DEBUG_PUMP_OFF, &para);
    factory_flag_info_t flag_info = {0};
    st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_time += thiz->priv.factory_ctx.aging_result_info.current_cycle_work_time/1000;
    if (flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_time >= thiz->priv.factory_ctx.aging_target_info.total_target_time) {
        flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_flag = AGING_TEST_STATUS_SUCCESS;
    } else {
        flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_flag = AGING_TEST_STATUS_FAILED;
    }
    st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    LOG_TEST("factory test aging test exit: age_flag = %d, age_time = %d", flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_flag, \
          flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_time);

    return 0;
}

int factory_tests_suction_test(tests_func_args_t args)
{
    /* 吸乳模式→L15档→双边工作 */
    LOG_TEST("factory test suction test: %d", args.arg1);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_INT(pump_ctx);
    motor_manager_context_t *motor_ctx = pump_ctx->priv.motor;
    EXCEPTION_CHECK_INT(motor_ctx);
    if (args.arg1 == 1) {
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE, 1);
        WRITE_PUMP_WORK_MODE(pump_ctx, BREAST_PUMP_LACTATION_MODE, false, 1);
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_START, 1);
        msleep(100);
        int time_index = 0;
        READ_PUMP_MODE_STEP_INDEX(pump_ctx, time_index);
        pump_setting_t setting = {0};
        READ_PUMP_SETTING_INFO(pump_ctx, time_index, setting);
        setting.level[0] = BREAST_PUMP_LEVEL_15;
        setting.level[1] = BREAST_PUMP_LEVEL_15;
        WRITE_PUMP_SETTING_INFO(pump_ctx, time_index, false, setting);
        LOG_TEST("tests_tlvs_suction_test_mode_handler: level[0] = %d, level[1] = %d", setting.level[0],
                setting.level[1]);
    } else {
        WRITE_PUMP_WORK_STATUS(pump_ctx, BREAST_PUMP_PAUSE, 1);
    }
    return 0;
}

void factory_tests_thread_handler(struct tests_manager_context *ctx, void *msg)
{
    multi_func_msg_data_t *m = (multi_func_msg_data_t *)msg;
    uint16_t event = m->union_body.product_test_extra.event_type;
    uint16_t length = m->union_body.product_test_extra.data_length;
    //LOG_TEST("product_tests loop recv msg. %d", event);
    factory_event_printf((factory_tst_event_type_e)event);
    /* 仅在产测模式或老化测试模式处理事件 */
    if (ctx->priv.current_mode != TST_FACTORY_TYPE
        && ctx->priv.current_mode != TST_AGING_TEST_TYPE) {
        return;
    }
    switch (event) {
        case FAC_TST_READ_SW_VER_EVENT:
            {
                dispatch_listen_factory_event_report(ctx, FAC_TST_READ_SW_VER_EVENT, m->union_body.product_test_extra.extra_data, length);
                if (m->union_body.product_test_extra.extra_data) {
                    u_free(m->union_body.product_test_extra.extra_data);
                }
            }
            break;
        case FAC_TST_READ_DEV_MAC_EVENT:
            {

            }
            break;
        case FAC_TST_MOTOR_CTR_EVENT:
            {

            }
            break;
        case TST_POLL_EVENT://定时器轮询事件
            {
                if (ctx->priv.current_mode == TST_AGING_TEST_TYPE) {
                    factory_tests_aging_test_check_result();
                }
            }
            break;
        default:
            break;
    }

}

int on_factory_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    tests_manager_context_t *ctx = context;

    switch (id.code) {
        case ID_BROADCAST_BREAST_PUMP_STATE_CHANGE:
            if (arg1 == BREAST_PUMP_BROADCAST_COMPLETED) {
                if (ctx->priv.current_mode == TST_AGING_TEST_TYPE) {
                    factory_tests_aging_test_check_result();
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

/// 监听事件上报事
int dispatch_listen_factory_event_report(tests_manager_context_t *ctx, uint16_t event_id, void *data, uint16_t data_len)
{
    list_head_t *head = &ctx->priv.product_test_list;

    MUTEX_LOCK(ctx->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) {
        product_tst_evt_client_node_t *node = CONTAINER_OF(pos, product_tst_evt_client_node_t, list);
        if (node->report_type == TST_FACTORY_TYPE ) {
            node->cb(node->user_data, event_id, data, data_len);
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);

    return 0;
}

/// 初始化工厂测试模块，获取工厂测试单独需要相关设备句柄.
void on_perpare_factory_tests_init(tests_manager_context_t *ctx)
{
    thiz = ctx;
}

/// 初始化工厂测试模块，获取设备存储的产测信息等操作.
void on_post_factory_tests_init(tests_manager_context_t *ctx)
{
    thiz = ctx;
}
