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

/*
* DEFINES
*****************************************************************************************
*/

#define AGING1_TEST_DEFAULT_CYCLE_TIME               NORMAL_MODE_DEFAULT_WORK_TIME// 老化测试默认循环时间，单位秒
#define AGING1_TEST_IDLE_TIME                        10*60*1000// 老化测试空闲时间，单位毫秒
#define AGING2_TEST_DEFAULT_CYCLE_TIME               NORMAL_MODE_DEFAULT_WORK_TIME// 老化测试默认循环时间，单位秒
#define AGING2_TEST_IDLE_TIME                        10*60*1000// 老化测试空闲时间，单位毫秒

#define TESTS_KEY_CODE                                                                                                 \
    (KEY_CODE_POWER | KEY_CODE_START | KEY_CODE_MODE | KEY_CODE_FREQUENCY | KEY_CODE_BREASTFEEDING                     \
     | KEY_CODE_LEFT_CONTROL | KEY_CODE_RIGHT_CONTROL)
#define TESTS_KEY_KNOB_CODE (KEY_CODE_KNOB_LEFT | KEY_CODE_KNOB_RIGHT)
#define TESTS_KEY_CLICK_EVENT (KEY_EVENT_CLICK)
#define TESTS_KEY_LONG_PRESS_EVENT (KEY_EVENT_LONG_2S)

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
    servers_publish_data_args_t args = {
        .arg1 = 0x01, /// 短按
        .arg2 = 0x00, 
        .val = NULL,
    };
    uint16_t server_id = SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS;
    switch (keycode) {
        case KEY_CODE_POWER:
        case KEY_CODE_START:
        case KEY_CODE_MODE:
        case KEY_CODE_FREQUENCY:
        case KEY_CODE_BREASTFEEDING:
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
    servers_ctx->hook->factory_server_data_publish(SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS, &args);
    return 0;
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
    key_ctx->add_keyevent_handler("factory_key_tests", thiz, (TESTS_KEY_CODE | TESTS_KEY_KNOB_CODE),
        (TESTS_KEY_CLICK_EVENT | TESTS_KEY_LONG_PRESS_EVENT), on_tests_event_handler, KEY_HANDLER_SHIELD_FORBIDDEN);
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
    key_ctx->remove_keyevent_handler(on_tests_event_handler);
    return 0;
}

int factory_tests_aging_test_enter_mode1(void)
{
    int ret = 0;
    LOG_TEST("factory test aging test enter mode1");
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_INT(pump_ctx);
    motor_manager_context_t *motor_ctx = pump_ctx->priv.motor;
    EXCEPTION_CHECK_INT(motor_ctx);
    pump_ctx->set_breast_pump_mode(BREAST_PUMP_LACTATION_MODE, 0);
    pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
    msleep(100);
    uint8_t time_index = pump_ctx->get_breast_pump_step_index();
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }

    pump_setting_t setting = {0};
    pump_ctx->get_breast_pump_setting(&setting, time_index);
    setting.level[0] = BREAST_PUMP_LEVEL_15;
    setting.level[1] = BREAST_PUMP_LEVEL_15;
    pump_ctx->set_breast_pump_setting(&setting, time_index, false);

    LOG_TEST("tests_tlvs_suction_test_mode_handler: level[0] = %d, level[1] = %d", setting.level[0],
            setting.level[1]);
    return ret;
}

int factory_tests_aging_test_enter_mode2(void)
{
    int ret = 0;
    LOG_TEST("factory test aging test enter mode2");

    return ret;
}

void factory_tests_aging_test_check_result(void)
{
    LOG_TEST("factory test aging test check result");
    factory_flag_info_t flag_info = {0};
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    EXCEPTION_CHECK_VOID(thiz);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump_ctx);
    pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);

    st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    if (thiz->priv.current_mode == TST_AGING_TEST_TYPE
        && thiz->priv.factory_ctx.aging_target_info.aging_type == AGING_MODE_TYPE_1) {
        flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_time += AGING1_TEST_DEFAULT_CYCLE_TIME;
        thiz->priv.factory_ctx.aging_result_info.total_runtime_time += AGING1_TEST_DEFAULT_CYCLE_TIME;
        flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_cycle++;
        if (flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_time
            >= thiz->priv.factory_ctx.aging_target_info.total_target_time) {
            flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_flag = AGING_TEST_STATUS_SUCCESS;
            POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_exit, 1000, "factory_tests_aging_test_exit_task");
        } else {

            flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_flag = AGING_TEST_STATUS_FAILED;
            POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_enter_mode1, AGING1_TEST_IDLE_TIME,
                                           "factory_tests_aging_test_enter_mode1_task");
        }
        st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
        LOG_TEST("factory test aging test check result (MODE_1): age_flag = %d, age_time = %d, age_cycle = %d",
             flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_flag, flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_time,
             flag_info.age_info[AGING_MODE_TYPE_1 - 1].age_cycle);
    } else if (thiz->priv.current_mode == TST_AGING_TEST_TYPE
               && thiz->priv.factory_ctx.aging_target_info.aging_type == AGING_MODE_TYPE_2) {
        flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_time += AGING2_TEST_DEFAULT_CYCLE_TIME;
        thiz->priv.factory_ctx.aging_result_info.total_runtime_time += AGING2_TEST_DEFAULT_CYCLE_TIME;
        flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_cycle++;
        if (flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_time
            >= thiz->priv.factory_ctx.aging_target_info.total_target_time) {
            flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_flag = AGING_TEST_STATUS_SUCCESS;
            POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_exit, 1000, "factory_tests_aging_test_exit_task");
        } else {
            flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_flag = AGING_TEST_STATUS_FAILED;
            POST_TASK_HANDLER_DELAYED_ARG0(factory_tests_aging_test_enter_mode2, AGING2_TEST_IDLE_TIME,
                                           "factory_tests_aging_test_enter_mode2_task");
        }
        st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
        LOG_TEST("factory test aging test check result (MODE_2): age_flag = %d, age_time = %d, age_cycle = %d",
             flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_flag, flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_time,
             flag_info.age_info[AGING_MODE_TYPE_2 - 1].age_cycle);
    } else {
        LOG_TEST("factory test aging test check result: current_mode = %d, aging_type = %d, not in aging test",
             thiz ? thiz->priv.current_mode : -1, thiz ? thiz->priv.factory_ctx.aging_target_info.aging_type : -1);
    }
}

//添加获取老化时间的接口
uint32_t factory_tests_aging_test_get_current_age_time(void)
{
    EXCEPTION_CHECK_INT(thiz);
    return thiz->priv.factory_ctx.aging_result_info.total_runtime_time;
}

/// 获取老化时间的包装函数（用于事件处理接口）
int get_factory_tests_aging_test_current_age_time(tests_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.val);
    uint32_t age_time = factory_tests_aging_test_get_current_age_time();
    *(uint32_t *)args.val = age_time;
    LOG_TEST("factory test get current age time: %lu", (unsigned long)age_time);
    return ret;
}

int factory_tests_aging_test_enter(tests_func_args_t args)
{
    /* 自动循环工作N小时（按摩→吸乳→混合，1-15档循环） */
    int ret = 0;
    memset(&thiz->priv.factory_ctx.aging_target_info, 0, sizeof(aging_target_info_t));
    memset(&thiz->priv.factory_ctx.aging_result_info, 0, sizeof(aging_result_info_t));
    thiz->priv.factory_ctx.aging_target_info.total_target_time = (uint32_t)args.arg2; /// 老化目标时长
    thiz->priv.factory_ctx.aging_target_info.aging_type = (uint16_t)args.arg1;        /// 老化模式类型
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
    LOG_TEST("factory test aging test exit");
    power_manager_context_t *power_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_INT(power_ctx);
    CHECK_IF_VALID(power_ctx, {
        power_ctx->release_wakelock("factory_tests_aging_test");
    });
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_INT(pump_ctx);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_INT(st);
    pump_result_info_t *pumping_info = pump_ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
    factory_flag_info_t flag_info = {0};
    st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_time += pumping_info->current_mode_work_time/1000;
    if (flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_time >= thiz->priv.factory_ctx.aging_target_info.total_target_time) {
        flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_flag = AGING_TEST_STATUS_SUCCESS;
    } else {
        flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_flag = AGING_TEST_STATUS_FAILED;
    }
    st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    LOG_TEST("factory test aging test exit: age_flag = %d, age_time = %d", flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_flag, flag_info.age_info[thiz->priv.factory_ctx.aging_target_info.aging_type - 1].age_time);
    pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);

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
        pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
        pump_ctx->set_breast_pump_mode(BREAST_PUMP_LACTATION_MODE, 0);
        pump_ctx->set_breast_pump_status(BREAST_PUMP_START);
        msleep(100);
        uint8_t time_index = pump_ctx->get_breast_pump_step_index();
        if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
            time_index = 0;
        }
        pump_setting_t setting = {0};
        pump_ctx->get_breast_pump_setting(&setting, time_index);
        setting.level[0] = BREAST_PUMP_LEVEL_15;
        setting.level[1] = BREAST_PUMP_LEVEL_15;
        pump_ctx->set_breast_pump_setting(&setting, time_index, false);

        LOG_TEST("tests_tlvs_suction_test_mode_handler: level[0] = %d, level[1] = %d", setting.level[0],
                setting.level[1]);
    } else {
        pump_ctx->set_breast_pump_status(BREAST_PUMP_PAUSE);
    }
    return 0;
}

void factory_tests_thread_handler(struct tests_manager_context *ctx, void *msg)
{
    multi_func_msg_data_t *m = (multi_func_msg_data_t *)msg;
    factory_tst_event_type_e event = (factory_tst_event_type_e)m->union_body.product_test_extra.event_type;
    uint16_t length = m->union_body.product_test_extra.data_length;
    //LOG_TEST("product_tests loop recv msg. %d", event);
    factory_event_printf(event);
    /* 如果当前模式不是产测模式，则不处理 */
    if (ctx->priv.current_mode != TST_FACTORY_TYPE) {
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
        default:
            break;
    }

}

int on_factory_tests_br_receiver_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data)
{
    tests_manager_context_t *ctx = context;

    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            break;
        case ID_BROADCAST_BATTERY_STATE_CHANGE:
            break;
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
