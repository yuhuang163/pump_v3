/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"      // IWYU pragma: keep
#include "common_tlvs.h"          // IWYU pragma: keep
#include "storage_manager_api.h"  // IWYU pragma: keep
#include "protocol_manager_api.h" // IWYU pragma: keep
#include "ble_manager_api.h"      // IWYU pragma: keep
#include "persist_storage.h"      // IWYU pragma: keep
#include "cmsis_os_api.h"         // IWYU pragma: keep
#include "root_log_define.h"      // IWYU pragma: keep
/*
* DEFINES
*****************************************************************************************
*/
#define TESTS_SN_LENGTH 35          /// SN数据长度
#define TESTS_PRODUCT_ID_LENGTH 6   /// 产品ID长度
#define TESTS_DEVICE_ID_LENGTH 16   /// 设备ID长度
#define TESTS_SECRET_KEY_LENGTH 16  /// 密钥长度
#define TESTS_TRIPLE_DATA_LENGTH 38 /// 三元组数据长度 (6+16+16)
#define TESTS_TRIM_VALUE_LENGTH 4   /// Trim值长度 (4字节)

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
static storage_manager_context_t *get_storage_ctx(void)
{
    return (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
}

static protocol_manager_context_t *get_protocol_ctx(void)
{
    return (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
}

static ble_manager_context_t *get_ble_ctx(void)
{
    return (ble_manager_context_t *)find_module(ID_MODULE_BLE);
}

static void test_factory_suction_test_post_task_handler(hdr_post_args_t *args)
{
    LOG_SRV("enter");
    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_VOID(tests_ctx);
    tests_ctx->set_factory_tests_event_process(FAC_TST_SUCTION_TEST_EVENT, 1, 0, NULL);
}

static void test_factory_aging_test_enter_post_task_handler(hdr_post_args_t *args)
{
    LOG_SRV("enter");
    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_VOID(tests_ctx);
    tests_ctx->set_factory_tests_event_process(FAC_TST_AGING_TEST_ENTER_EVENT, args->arg1, args->arg2, NULL);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/// ======================== 模式控制 ========================

int tests_tlvs_factory_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                 uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;

    LOG_SRV("tests_tlvs_factory_test_mode: %s", mode ? "ENTER" : "EXIT");
    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_INT(tests_ctx);
    // TODO: 实现产测模式进入/退出逻辑
    if (mode == 1) {
        /* 进入产测模式 */
        set_log_ouput_level(US_LOG_LEVEL_NONE);
        //关闭自适应夜灯
        algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
        EXCEPTION_CHECK_INT(algo);
        algo->hook->set_adaptive_dimming_state(false);

        tests_ctx->tests_manager_set_current_mode(TST_FACTORY_TYPE);
        tests_ctx->set_factory_tests_event_process(FAC_TST_ENTER_EVENT, 0, 0, NULL);
        //唤醒电源管理模块
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        if (pm && pm->is_pm_suspend()) {
            pm->pm_wakeup(PM_WAKEUP_REASON_FACTORY_RESUME);
        }
        /* 打开测试窗口 */
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_INT(wm);
        if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE) == 0) {
                multi_func_msg_data_t msg = {0};
                msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
                wm->dispatch_new_window_args(wm, &msg);
            }
        } else {
            /* 退出产测模式 */
            set_log_ouput_level(US_LOG_LEVEL_VERBOSE);
            //开启自适应夜灯
            algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
            EXCEPTION_CHECK_INT(algo);
            algo->hook->set_adaptive_dimming_state(true);
             
            tests_ctx->tests_manager_set_current_mode(TST_NONE_TYPE);
            tests_ctx->set_factory_tests_event_process(FAC_TST_EXIT_EVENT, 0, 0, NULL);
            /* 关闭测试窗口 */
            /*ndow_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
            EXCEPTION_CHECK_INT(wm);
            if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE)) {   // 如果当前窗口是测试窗口，则关闭测试窗口
                multi_func_msg_data_t msg = {0};
                msg.union_body.extra.window_id = ID_WINDOW_LOGO;
                wm->dispatch_new_window_args(wm, &msg);
            }*/
        }
    // 应答格式: Length=1, Value=0x00失败/0x01成功
    uint8_t result = 0x01; // 默认成功

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_FACTORY_TEST_MODE,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_aging_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                               uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t result = 0x00; // 默认失败

    // 协议格式: Length=5, byte0=老化模式类型, byte1-4=老化目标时长(小端序，单位s)
    if (data && data_len == 5) {
        uint8_t aging_mode_type = data[0]; // 0x01:吸乳模式等级15，IDLE十分钟循环; 0x02:吸乳模式等级15，循环
        uint32_t target_duration = (uint32_t)data[1] | ((uint32_t)data[2] << 8) | 
                                   ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 24); // 小端序

        LOG_SRV("tests_tlvs_aging_test_mode: ENTER, mode_type=0x%02X, duration=%d s", aging_mode_type, target_duration);

        tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
        EXCEPTION_CHECK_INT(tests_ctx);

        // TODO: 实现老化模式进入逻辑，设置模式类型和目标时长
        tests_ctx->tests_manager_set_current_mode(TST_AGING_TEST_TYPE);
        //启动产测模块 老化测试，传入模式类型和目标时长

        /* 打开测试窗口 */
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_INT(wm);
        if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE) == 0) {
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
            wm->dispatch_new_window_args(wm, &msg);
        }
        //唤醒电源管理模块
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        if (pm && pm->is_pm_suspend()) {
            pm->pm_wakeup(PM_WAKEUP_REASON_FACTORY_RESUME);
        }
        //启动产测模块 老化测试 吸乳模式→L15档→双边工作
        POST_TASK_HANDLER_DELAYED_ARG4(test_factory_aging_test_enter_post_task_handler, 1000, aging_mode_type, target_duration, "aging_test_cb");
        result = 0x01; // 成功
    } else {
        LOG_SRV("tests_tlvs_aging_test_mode: invalid data_len=%d, expected 5", data_len);
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_AGING_TEST_MODE,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_aging_test_mode_exit(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                     uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_aging_test_mode_exit: EXIT");

    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_INT(tests_ctx);

    /* 退出老化模式 */
    tests_ctx->tests_manager_set_current_mode(TST_NONE_TYPE);
    /* 关闭测试窗口 */
    /*window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    EXCEPTION_CHECK_INT(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE)) {   // 如果当前窗口是测试窗口，则关闭测试窗口
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_LOGO;
        wm->dispatch_new_window_args(wm, &msg);
    }*/

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    uint8_t result = 0x01; // 默认成功
    tests_ctx->set_factory_tests_event_process(FAC_TST_AGING_TEST_EXIT_EVENT, 0, 0, NULL);

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_AGING_TEST_MODE_EXIT,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_suction_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                 uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;

    LOG_SRV("tests_tlvs_suction_test_mode: %s", mode ? "ENTER" : "EXIT");

    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_INT(tests_ctx);
    // TODO: 实现吸力测试模式进入/退出逻辑
    if (mode == 1) {
        /* 进入吸力测试模式 */
        tests_ctx->tests_manager_set_current_mode(TST_SUCTION_TEST_TYPE);
        
        /* 打开测试窗口 */
        window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_INT(wm);
        if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE) == 0) {
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
            wm->dispatch_new_window_args(wm, &msg);
        }
        //唤醒电源管理模块
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        if (pm && pm->is_pm_suspend()) {
            pm->pm_wakeup(PM_WAKEUP_REASON_FACTORY_RESUME);
        }
        //启动产测模块 吸力测试 吸乳模式→L15档→双边工作
        POST_TASK_HANDLER_DELAYED_ARG0(test_factory_suction_test_post_task_handler, 1000, "suction_test_cb");
    } else {
        /* 退出吸力测试模式 */
        tests_ctx->tests_manager_set_current_mode(TST_NONE_TYPE);
        /* 关闭测试窗口 */
        /*ndow_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
        EXCEPTION_CHECK_INT(wm);
        if (wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE)) {   // 如果当前窗口是测试窗口，则关闭测试窗口
            multi_func_msg_data_t msg = {0};
            msg.union_body.extra.window_id = ID_WINDOW_LOGO;
            wm->dispatch_new_window_args(wm, &msg);
        }*/
         //关闭模块产测 吸力测试 吸乳模式→L15档→双边工作
        tests_ctx->set_factory_tests_event_process(FAC_TST_SUCTION_TEST_EVENT, 0, 0, NULL);
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    uint8_t result = 0x01; // 默认成功

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_SUCTION_TEST_MODE,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 老化状态查询 ========================

int tests_tlvs_aging_status_query(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                  uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    // 协议格式: REQUEST Length=1, byte0=老化模式类型(0x01或0x02)
    uint8_t aging_mode_type = 0;
    if (data && data_len >= 1) {
        aging_mode_type = data[0];
        LOG_SRV("tests_tlvs_aging_status_query, mode_type=0x%02X", aging_mode_type);
    } else {
        LOG_SRV("tests_tlvs_aging_status_query, invalid data_len=%d, expected 1", data_len);
    }

    // 协议格式: RESPONSE Length=7
    // byte0: 没有老化0x00，老化成功0x01，老化失败0x02
    // byte1-2: 老化循环次数(小端序)
    // byte3-6: 老化时长(小端序，单位s)
    uint8_t response[7] = {0};

    // 根据aging_mode_type从存储中读取对应模式的老化状态
    // 模式类型0x01对应age_info[0], 0x02对应age_info[1]
    storage_manager_context_t *st = get_storage_ctx();
    EXCEPTION_CHECK_INT(st);
    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_INT(tests_ctx);
    breast_pump_manager_context_t *pump_ctx = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_INT(pump_ctx);
    if (st) {
        factory_flag_info_t flag_info = {0};
        st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
        
        // 根据模式类型选择对应的老化信息索引
        uint8_t age_info_index = 0;
        if (aging_mode_type == 0x01) {
            age_info_index = 0; // 模式0x01: 吸乳模式等级15，IDLE十分钟循环
        } else if (aging_mode_type == 0x02) {
            age_info_index = 1; // 模式0x02: 吸乳模式等级15，循环
        } else {
            age_info_index = 0; // 默认使用索引0
        }
        
        // 确保索引有效
        if (age_info_index < FACTORY_AGE_INFO_MAX) {
            factory_age_info_t *age_info = &flag_info.age_info[age_info_index];
            response[0] = age_info->age_flag;
            
            uint16_t cycle_count = (uint16_t)age_info->age_cycle;
            response[1] = (uint8_t)(cycle_count & 0xFF);        // 小端序低字节
            response[2] = (uint8_t)((cycle_count >> 8) & 0xFF);  // 小端序高字节
            
            // 单位s，小端序
            // 如果当前模式正在老化模式运行中，则加上模式正在运行的时间
            uint32_t age_time = 0;
            if( tests_ctx->tests_manager_get_current_mode() == TST_AGING_TEST_TYPE ) {
                pump_result_info_t *pumping_info = pump_ctx->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
                age_time = age_info->age_time + pumping_info->current_mode_work_time/1000;
            } else {
                age_time =  age_info->age_time;
            }            
            response[3] = (uint8_t)(age_time & 0xFF);        // 小端序低字节
            response[4] = (uint8_t)((age_time >> 8) & 0xFF);  // 小端序高字节
            response[5] = (uint8_t)((age_time >> 16) & 0xFF); // 小端序高字节
            response[6] = (uint8_t)((age_time >> 24) & 0xFF); // 小端序高字节
            LOG_SRV("age_info[%d]: age_flag=%d, age_cycle=%d, age_time=%d, mode_type=0x%02X", 
                    age_info_index, age_info->age_flag, age_info->age_cycle, age_time, aging_mode_type);
        } else {
            LOG_SRV("Invalid age_info_index=%d, max=%d", age_info_index, FACTORY_AGE_INFO_MAX);
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_AGING_STATUS_QUERY,
        .length = sizeof(response),
        .value = response,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 三元组写入 ========================

int tests_tlvs_write_sn(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                        uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_write_sn, data_len: %d", data_len);
    LOG_HEX("write_sn", data, data_len);

    uint8_t result = 0x00; // 默认失败

    if (data && data_len > 0) {
        storage_manager_context_t *st = get_storage_ctx();
        if (st) {
            factory_dev_info_t dev_info = {0};
            st->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
            
            /// 参考app_protocol_auth_server.c中的写法进行长度判断
            if (data_len >= DEVICE_UNIQUE_SN_LEN) {
                dev_info.sn_len = DEVICE_UNIQUE_SN_LEN;
            } else {
                dev_info.sn_len = data_len;
            }
            
            memset(dev_info.sn, 0, DEVICE_UNIQUE_SN_LEN);
            memcpy(dev_info.sn, data, dev_info.sn_len);
            
            if (st->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info) == 0) {
                result = 0x01; // 成功
            }
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_WRITE_SN,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_write_product_id(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_write_product_id, data_len: %d", data_len);
    LOG_HEX("write_product_id", data, data_len);

    uint8_t result = 0x00; // 默认失败

    if (data && data_len > 0) {
        storage_manager_context_t *st = get_storage_ctx();
        if (st) {
            factory_dev_info_t dev_info = {0};
            st->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
            
            /// 参考app_protocol_auth_server.c中的写法进行长度判断
            if (data_len >= DEVICE_PRODUCT_ID_LEN) {
                dev_info.product_id_len = DEVICE_PRODUCT_ID_LEN;
            } else {
                dev_info.product_id_len = data_len;
            }
            
            memset(dev_info.product_id, 0, DEVICE_PRODUCT_ID_LEN);
            memcpy(dev_info.product_id, data, dev_info.product_id_len);
            
            if (st->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info) == 0) {
                result = 0x01; // 成功
            }
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_WRITE_PRODUCT_ID,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_write_device_id(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                               uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_write_device_id, data_len: %d", data_len);
    LOG_HEX("write_device_id", data, data_len);

    uint8_t result = 0x00; // 默认失败

    if (data && data_len > 0) {
        storage_manager_context_t *st = get_storage_ctx();
        if (st) {
            factory_dev_info_t dev_info = {0};
            st->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
            
            /// 参考app_protocol_auth_server.c中的写法进行长度判断
            if (data_len >= DEVICE_DEVICE_ID_LEN) {
                dev_info.device_id_len = DEVICE_DEVICE_ID_LEN;
            } else {
                dev_info.device_id_len = data_len;
            }
            
            memset(dev_info.device_id, 0, DEVICE_DEVICE_ID_LEN);
            memcpy(dev_info.device_id, data, dev_info.device_id_len);
            
            if (st->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info) == 0) {
                result = 0x01; // 成功
            }
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_WRITE_DEVICE_ID,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_write_secret_key(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_write_secret_key, data_len: %d", data_len);
    LOG_HEX("write_secret_key", data, data_len);

    uint8_t result = 0x00; // 默认失败

    if (data && data_len > 0) {
        storage_manager_context_t *st = get_storage_ctx();
        if (st) {
            factory_dev_info_t dev_info = {0};
            st->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
            
            /// 参考app_protocol_auth_server.c中的写法进行长度判断
            if (data_len >= DEVICE_ENCTYPTION_KEY_LEN) {
                dev_info.enctyption_key_len = DEVICE_ENCTYPTION_KEY_LEN;
            } else {
                dev_info.enctyption_key_len = data_len;
            }
            
            memset(dev_info.enctyption_key, 0, DEVICE_ENCTYPTION_KEY_LEN);
            memcpy(dev_info.enctyption_key, data, dev_info.enctyption_key_len);
            
            if (st->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info) == 0) {
                result = 0x01; // 成功
            }
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_WRITE_SECRET_KEY,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 三元组读取 ========================

int tests_tlvs_read_triple_data(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_read_triple_data");

    // 应答格式: byte0-5=产品ID(6字节), byte6-21=设备ID(16字节), byte22-37=密钥(16字节)
    // 使用最大长度作为缓冲区大小，但使用实际存储的长度进行复制
    uint8_t response[DEVICE_PRODUCT_ID_LEN + DEVICE_DEVICE_ID_LEN + DEVICE_ENCTYPTION_KEY_LEN] = {0};
    uint16_t response_len = 0;

    storage_manager_context_t *st = get_storage_ctx();
    if (st) {
        factory_dev_info_t dev_info = {0};
        st->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
        
        /// 参考app_protocol_auth_server.c中的写法，使用实际存储的长度
        uint16_t product_id_len = dev_info.product_id_len > DEVICE_PRODUCT_ID_LEN ? DEVICE_PRODUCT_ID_LEN : dev_info.product_id_len;
        uint16_t device_id_len = dev_info.device_id_len > DEVICE_DEVICE_ID_LEN ? DEVICE_DEVICE_ID_LEN : dev_info.device_id_len;
        uint16_t key_len = dev_info.enctyption_key_len > DEVICE_ENCTYPTION_KEY_LEN ? DEVICE_ENCTYPTION_KEY_LEN : dev_info.enctyption_key_len;
        
        memcpy(response, dev_info.product_id, product_id_len);
        memcpy(response + product_id_len, dev_info.device_id, device_id_len);
        memcpy(response + product_id_len + device_id_len, dev_info.enctyption_key, key_len);
        
        response_len = product_id_len + device_id_len + key_len;
        
        //打印dev_info.product_id, dev_info.device_id, dev_info.enctyption_key
        LOG_HEX("dev_info.product_id", dev_info.product_id, product_id_len);
        LOG_HEX("dev_info.device_id", dev_info.device_id, device_id_len);
        LOG_HEX("dev_info.enctyption_key", dev_info.enctyption_key, key_len);
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_READ_TRIPLE_DATA,
        .length = response_len,
        .value = response,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_read_sn(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                       uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_read_sn");

    // 使用最大长度作为缓冲区大小，但使用实际存储的长度进行复制
    uint8_t response[DEVICE_UNIQUE_SN_LEN] = {0};
    uint16_t response_len = 0;

    storage_manager_context_t *st = get_storage_ctx();
    if (st) {
        factory_dev_info_t dev_info = {0};
        st->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &dev_info);
        
        /// 参考app_protocol_auth_server.c中的写法，使用实际存储的长度
        response_len = dev_info.sn_len > DEVICE_UNIQUE_SN_LEN ? DEVICE_UNIQUE_SN_LEN : dev_info.sn_len;
        memcpy(response, dev_info.sn, response_len);
        
        LOG_HEX("SN:", dev_info.sn, response_len);
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_READ_SN,
        .length = response_len,
        .value = response,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 设备状态 ========================

int tests_tlvs_device_error_status(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                   uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_device_error_status");

    // TODO: 获取设备异常状态
    uint8_t error_status = TESTS_DEVICE_ERROR_NONE;

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_DEVICE_ERROR_STATUS,
        .length = 1,
        .value = &error_status,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_suction_compensation(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                    uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t onoff = (data && data_len >= 1) ? data[0] : 0;

    LOG_SRV("tests_tlvs_suction_compensation: %s", onoff ? "ON" : "OFF");

    // TODO: 实现吸力补偿开关逻辑

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    uint8_t result = 0x01; // 默认成功

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_SUCTION_COMPENSATION,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 产测完成标识写入 ========================

int tests_tlvs_write_factory_complete_flag(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                                           uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_write_factory_complete_flag, data_len: %d", data_len);

    uint8_t result = 0x00; // 默认失败

    if (data && data_len >= 1) {
        uint8_t complete_flag = data[0]; // 0x00=产测未完成, 0x01=产测完成

        storage_manager_context_t *st = get_storage_ctx();
        if (st) {
            factory_flag_info_t flag_info = {0};
            st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
            // 使用 deploy_flag 字段存储产测完成标识
            flag_info.deploy_flag = complete_flag;
            if (st->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info) == 0) {
                result = 0x01; // 成功
                LOG_SRV("tests_tlvs_write_factory_complete_flag: %s", complete_flag ? "COMPLETE" : "NOT_COMPLETE");
            }
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_WRITE_FACTORY_COMPLETE_FLAG,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_read_factory_complete_flag(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                                         uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_read_factory_complete_flag, data_len: %d", data_len);

    uint8_t complete_flag = 0x00; // 默认产测未完成

    storage_manager_context_t *st = get_storage_ctx();
    if (st) {
        factory_flag_info_t flag_info = {0};
        if (st->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info) == 0) {
            // 读取 deploy_flag 字段作为产测完成标识
            complete_flag = flag_info.deploy_flag;
            LOG_SRV("tests_tlvs_read_factory_complete_flag: %s", complete_flag ? "COMPLETE" : "NOT_COMPLETE");
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_READ_FACTORY_COMPLETE_FLAG,
        .length = 1,
        .value = &complete_flag,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 蓝牙测试 ========================

int tests_tlvs_bt_signaling_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                 uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;

    LOG_SRV("tests_tlvs_bt_signaling_test: %s", mode ? "ENTER" : "EXIT");

    uint8_t result = 0x00; // 默认失败

    if (mode == 1) {
        /* 进入蓝牙信令测试模式 */
        platform_bt_enter_signalingtest_mode(); // 此函数会设置启动模式并重启
        result = 0x01; // 成功（实际会在重启后生效）
    } else {
        /* 退出蓝牙信令测试模式 */
        // 退出需要重启到正常模式，这里返回成功
        result = 0x01; // 默认成功
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_BT_SIGNALING_TEST,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_bt_nosignaling_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                   uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;

    LOG_SRV("tests_tlvs_bt_nosignaling_test: %s", mode ? "ENTER" : "EXIT");

    uint8_t result = 0x00; // 默认失败

    if (mode == 1) {
        /* 进入蓝牙非信令测试模式 */
        platform_bt_enter_nosignalingtest_mode(); // 此函数会设置启动模式并重启
        result = 0x01; // 成功（实际会在重启后生效）
    } else {
        /* 退出蓝牙非信令测试模式 */
        // 退出需要重启到正常模式，这里返回成功
        result = 0x01; // 默认成功
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_BT_NOSIGNALING_TEST,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_bt_xtal_calib_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                  uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;

    LOG_SRV("tests_tlvs_bt_xtal_calib_test: %s", mode ? "ENTER" : "EXIT");

    uint8_t result = 0x00; // 默认失败

    if (mode == 1) {
        /* 进入蓝牙校频测试模式 */
        // 调用校频测试函数
        if (platform_bt_xtal_calib_proc_start() == 0) {
            result = 0x01; // 成功
        }
    } else {
        /* 退出蓝牙校频测试模式 */
        // TODO: 实现退出逻辑
        platform_bt_xtal_calib_end();
        result = 0x01; // 默认成功
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_BT_XTAL_CALIB_TEST,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== Trim值操作 ========================

int tests_tlvs_set_trim_value(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                               uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_set_trim_value, data_len: %d", data_len);
    LOG_HEX("set_trim_value", data, data_len);

    uint8_t result = 0x00; // 默认失败

    // 协议格式: Length=4, byte0-3=trim值(小端序)
    if (data && data_len == TESTS_TRIM_VALUE_LENGTH) {
        // 解析小端序的trim值
        uint32_t trim_value = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | 
                              ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
        
        LOG_SRV("tests_tlvs_set_trim_value: trim_value=0x%08X (%u)", trim_value, trim_value);
        
        platform_bt_xtal_calib_set(trim_value);
        result = 0x01; // 成功
    } else {
        LOGE("tests_tlvs_set_trim_value: invalid data_len=%d, expected=%d", data_len, TESTS_TRIM_VALUE_LENGTH);
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_SET_TRIM_VALUE,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_get_trim_value(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                               uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_get_trim_value");

    // 应答格式: Length=4, byte0-3=trim值(小端序)
    uint8_t response[TESTS_TRIM_VALUE_LENGTH] = {0};
    /// Trim值存储
    uint32_t trim_value = 0;
    platform_bt_xtal_calib_get(&trim_value);
    // 将trim值转换为小端序字节数组
    response[0] = (uint8_t)(trim_value & 0xFF);
    response[1] = (uint8_t)((trim_value >> 8) & 0xFF);
    response[2] = (uint8_t)((trim_value >> 16) & 0xFF);
    response[3] = (uint8_t)((trim_value >> 24) & 0xFF);

    LOG_SRV("tests_tlvs_get_trim_value: trim_value=0x%08X (%u)", trim_value);
    LOG_HEX("get_trim_value_response", response, TESTS_TRIM_VALUE_LENGTH);

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_GET_TRIM_VALUE,
        .length = TESTS_TRIM_VALUE_LENGTH,
        .value = response,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 待机和传感器状态 ========================

int tests_tlvs_standby_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                            uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;
    uint8_t result = 0x00; // 默认失败

    LOG_SRV("tests_tlvs_standby_mode: %s", mode ? "ENTER" : "EXIT");

    if (mode == 1) {
        /* 进入待机模式 */
        SEND_BROADCAST_ARGS2(ID_BROADCAST_REQ_PM_SUSPEND_BY_USER, 0,
                             SUSPEND_BY_FAC);
        result = 0x01; // 成功
    } else {
        /* 退出待机模式 */
        power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
        if (pm) {
            pm->pm_wakeup(PM_WAKEUP_REASON_FACTORY_RESUME);
            result = 0x01; // 成功
        }
    }

    // 应答格式: Length=1, Value=0x00失败/0x01成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_STANDBY_MODE,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int tests_tlvs_query_sensor_status(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                   uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_query_sensor_status");

    // 查询各个传感器的通信状态
    // 应答格式: Length=N+1, byte0-N=各设备状态(0x00=通信失败, 0x01=通信成功)
    // 传感器顺序：IMU, 力传感器, 气压传感器, 光传感器(如果支持)
    uint8_t response[16] = {0}; // 最多支持16个传感器
    uint8_t sensor_count = 0;

    // 通过 peripheral_bus 查询各个传感器的通信状态
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);

    // 查询气压传感器状态
    #if IS_SUPPORT_DEVICE_AIR_PRESSURE
    {
        uint8_t pressure_is_ready_status[AIR_PRESSURE_NUM] = {0};
        rt_device_node_t *air_pressure_dev = pbus->find_device(ID_DEVICE_AIR_PRESSURE);
        if (air_pressure_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(air_pressure_dev, OP_CODE_AIR_PRESSURE_IS_READY, pressure_is_ready_status);
            for (uint8_t i = 0; i < AIR_PRESSURE_NUM; i++) {
                response[sensor_count] = pressure_is_ready_status[i];
                sensor_count++;
            }
        }
    }
    #else
        response[sensor_count] = 0x00;
        sensor_count++;
    #endif

    // 查询电池状态
    #if IS_SUPPORT_DEVICE_BATTERY
    {
        uint8_t charge_is_ready_status = 0;
        rt_device_node_t *battery_dev = pbus->find_device(ID_DEVICE_BATTERY);
        if (battery_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(battery_dev, OP_CODE_BAT_IS_READY, &charge_is_ready_status);
            response[sensor_count] = charge_is_ready_status;
            sensor_count++;
        }
    }   
    #else
        response[sensor_count] = 0x00;
        sensor_count++;
    #endif
    // 查询触摸按键状态
    #if IS_SUPPORT_DEVICE_TOUCH_KEY
    {
        uint8_t touch_is_ready_status = 0;
        rt_device_node_t *input_key_dev = pbus->find_device(ID_DEVICE_INPUT_KEY);
        if (input_key_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(input_key_dev, OP_CODE_INPUT_TOUCH_IS_READY, &touch_is_ready_status);
            response[sensor_count] = touch_is_ready_status;
            sensor_count++;
        }
    }
    #else
        response[sensor_count] = 0x00;
        sensor_count++;
    #endif

    // 查询LED状态
    #if IS_SUPPORT_DEVICE_LED
    {
        uint8_t led_is_ready_status = 0;
        rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
        if (led_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(led_dev, OP_CODE_LED_IS_READY, &led_is_ready_status);
            response[sensor_count] = led_is_ready_status;
            sensor_count++;
        }
    }
    #else
        response[sensor_count] = 0x00;
        sensor_count++;
    #endif
    // 查询PD充电状态
    #if IS_SUPPORT_PD_CHARGE
    {
        uint8_t charge_pd_is_ready_status = 0;
        rt_device_node_t *battery_dev = pbus->find_device(ID_DEVICE_BATTERY);
        if (battery_dev) {
            DEVICE_IOCTL_ARGS_NO_RET(battery_dev, OP_CODE_BAT_PD_IS_READY, &charge_pd_is_ready_status);
            response[sensor_count] = charge_pd_is_ready_status;
            sensor_count++;
        }
    }
    #else
        response[sensor_count] = 0x00;
        sensor_count++;
    #endif

    LOG_SRV("tests_tlvs_query_sensor_status: sensor_count=%d", sensor_count);
    LOG_HEX("sensor_status_response", response, sensor_count);

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_QUERY_SENSOR_STATUS,
        .length = sensor_count,
        .value = response,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== RSSI获取 ========================

int tests_tlvs_get_rssi(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                        uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("tests_tlvs_get_rssi, data_len: %d", data_len);

    // 协议格式: REQUEST Length=1, byte0: 0x00获取BLE RSSI, 0x01获取BT RSSI
    uint8_t rssi_type = 0;
    if (data && data_len >= 1) {
        rssi_type = data[0];
    } else {
        LOG_SRV("tests_tlvs_get_rssi: invalid data_len=%d, expected 1", data_len);
    }

    // 应答格式: RESPONSE Length=4, byte0-3: RSSI值(小端序，int32_t)
    int32_t rssi_value = 0;

    if (rssi_type == 0x00) {
        // 获取BLE RSSI
        if (platform_bt_get_rssi_value(&rssi_value) == 0) {
            LOG_SRV("tests_tlvs_get_rssi: BLE RSSI=%d", rssi_value);
        } else {
            LOG_SRV("tests_tlvs_get_rssi: BLE RSSI获取失败");
            rssi_value = 0; // 失败时返回0
        }
    } else if (rssi_type == 0x01) {
        // 获取BT RSSI - 暂未实现
        LOG_SRV("tests_tlvs_get_rssi: BT RSSI获取功能暂未实现");
        rssi_value = 0; // 暂未实现，返回0
    } else {
        LOG_SRV("tests_tlvs_get_rssi: invalid rssi_type=0x%02X, expected 0x00(BLE) or 0x01(BT)", rssi_type);
        rssi_value = 0; // 无效类型时返回0
    }
    uint8_t response[4] = {0};
    response[0] = (uint8_t)(rssi_value & 0xFF);
    response[1] = (uint8_t)((rssi_value >> 8) & 0xFF);
    response[2] = (uint8_t)((rssi_value >> 16) & 0xFF);
    response[3] = (uint8_t)((rssi_value >> 24) & 0xFF);

    LOG_SRV("tests_tlvs_get_rssi: rssi_type=0x%02X, rssi_value=%d", rssi_type, rssi_value);
    LOG_HEX("rssi_response", response, sizeof(response));

    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_GET_RSSI,
        .length = sizeof(response),
        .value = response,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== NOTIFY上报 ========================

int tests_key_status_publish(servers_publish_data_args_t *args)
{
    int ret = COMM_PROTOCOL_ERROR_NONE;
    EXCEPTION_CHECK_INT(args);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);
    ble_manager_context_t *ble_ctx = get_ble_ctx();
    EXCEPTION_CHECK_INT(ble_ctx);

    uint8_t key_status = (uint8_t)args->arg1; // 0x01=短按, 0x02=长按
    LOG_SRV("tests_key_status_publish: %s", key_status == TESTS_KEY_STATUS_SHORT_PRESS ? "SHORT" : "LONG");

    comm_protocol_builder_t builder = {0};
    /// 初始化构建器
    ret = protocol_ctx->comm_protocol_builder_init(&builder, 256, protocol_ctx->get_packet_seq(),
                                                   COMM_PROTOCOL_FRAME_TYPE_NOTIFY);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_init failed");
        return COMM_PROTOCOL_ERROR_INVALID_BUILDER;
    }

    /// 初始化服务节点
    comm_protocol_service_t *service =
        protocol_ctx->comm_protocol_builder_init_service(&builder, COMM_PROTOCOL_TESTS_SERVICE);
    if (service == NULL) {
        LOGE("==> tests service begin error");
        goto ERROR_EXIT;
    }

    /// 添加按键状态TLV
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_KEY_STATUS_REPORT,
        .length = 1,
        .value = &key_status,
    };
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &tlv_node);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_add_tlv failed");
        goto ERROR_EXIT;
    }

    /// 最终化构建器
    uint16_t out_size = 0;
    uint8_t *data = protocol_ctx->comm_protocol_builder_finalize(&builder, &out_size);
    if (data == NULL) {
        LOGE("==> comm_protocol_builder_finalize failed");
        goto ERROR_EXIT;
    }

    /// 发送数据
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_FAC_SEND_DATA, data, out_size);

ERROR_EXIT:
    protocol_ctx->comm_protocol_builder_free(&builder);
    return ret;
}

int tests_encoder_status_publish(servers_publish_data_args_t *args)
{
    int ret = COMM_PROTOCOL_ERROR_NONE;
    EXCEPTION_CHECK_INT(args);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);
    ble_manager_context_t *ble_ctx = get_ble_ctx();
    EXCEPTION_CHECK_INT(ble_ctx);

    uint8_t encoder_status = (uint8_t)args->arg1; // 0x01=左旋, 0x02=右旋
    LOG_SRV("tests_encoder_status_publish: %s", encoder_status == TESTS_ENCODER_STATUS_LEFT ? "LEFT" : "RIGHT");

    comm_protocol_builder_t builder = {0};
    /// 初始化构建器
    ret = protocol_ctx->comm_protocol_builder_init(&builder, 256, protocol_ctx->get_packet_seq(),
                                                   COMM_PROTOCOL_FRAME_TYPE_NOTIFY);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_init failed");
        return COMM_PROTOCOL_ERROR_INVALID_BUILDER;
    }

    /// 初始化服务节点
    comm_protocol_service_t *service =
        protocol_ctx->comm_protocol_builder_init_service(&builder, COMM_PROTOCOL_TESTS_SERVICE);
    if (service == NULL) {
        LOGE("==> tests service begin error");
        goto ERROR_EXIT;
    }

    /// 添加编码器状态TLV
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_ENCODER_STATUS_REPORT,
        .length = 1,
        .value = &encoder_status,
    };
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &tlv_node);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_add_tlv failed");
        goto ERROR_EXIT;
    }

    /// 最终化构建器
    uint16_t out_size = 0;
    uint8_t *data = protocol_ctx->comm_protocol_builder_finalize(&builder, &out_size);
    if (data == NULL) {
        LOGE("==> comm_protocol_builder_finalize failed");
        goto ERROR_EXIT;
    }

    /// 发送数据
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_FAC_SEND_DATA, data, out_size);

ERROR_EXIT:
    protocol_ctx->comm_protocol_builder_free(&builder);
    return ret;
}

int tests_tlvs_lcd_color_test_mode(comm_protocol_builder_t *builder, 
    comm_protocol_service_t *service, uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t mode = (data && data_len >= 1) ? data[0] : 0;
    LOG_SRV("lcd color test: %s", mode ? "ENTER" : "EXIT");
    tests_manager_context_t *tests_ctx = (tests_manager_context_t *)find_module(ID_MODULE_TESTS);
    EXCEPTION_CHECK_INT(tests_ctx);
    tests_ctx->tests_manager_set_current_mode(mode == 1 ? TST_LCD_COLOR_TYPE : TST_NONE_TYPE);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    if (pm && pm->is_pm_suspend()) {
        pm->pm_wakeup(PM_WAKEUP_REASON_FACTORY_RESUME);
    }

    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    if (wm && wm->is_current_window(wm, ID_WINDOW_PRODUCT_TEST_MODE) == 0) {
        multi_func_msg_data_t msg = {0};
        msg.union_body.extra.window_id = ID_WINDOW_PRODUCT_TEST_MODE;
        wm->dispatch_new_window_args(wm, &msg);
    } else {
        SEND_BROADCAST_ARGS2(ID_BROADCAST_FACTORY_TEST_CMD, TST_LCD_COLOR_TYPE, mode);
    }
    uint8_t result = 0x01; // 默认成功
    comm_protocol_tlv_node_t tlv_node = {
        .type = TESTS_TLV_LCD_COLOR_TEST_MODE,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}