/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "common_tlvs.h"     // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static protocol_service_tlvs_handle_t protocol_service_tlvs_list[] = {
    /// 模式控制
    {TESTS_TLV_FACTORY_TEST_MODE, tests_tlvs_factory_test_mode},
    {TESTS_TLV_AGING_TEST_MODE, tests_tlvs_aging_test_mode},
    {TESTS_TLV_AGING_TEST_MODE_EXIT, tests_tlvs_aging_test_mode_exit},
    {TESTS_TLV_SUCTION_TEST_MODE, tests_tlvs_suction_test_mode},
    /// 老化状态查询
    {TESTS_TLV_AGING_STATUS_QUERY, tests_tlvs_aging_status_query},
    /// 三元组写入
    {TESTS_TLV_WRITE_SN, tests_tlvs_write_sn},
    {TESTS_TLV_WRITE_PRODUCT_ID, tests_tlvs_write_product_id},
    {TESTS_TLV_WRITE_DEVICE_ID, tests_tlvs_write_device_id},
    {TESTS_TLV_WRITE_SECRET_KEY, tests_tlvs_write_secret_key},
    /// 三元组读取
    {TESTS_TLV_READ_TRIPLE_DATA, tests_tlvs_read_triple_data},
    {TESTS_TLV_READ_SN, tests_tlvs_read_sn},
    /// 设备状态
    {TESTS_TLV_DEVICE_ERROR_STATUS, tests_tlvs_device_error_status},
    {TESTS_TLV_SUCTION_COMPENSATION, tests_tlvs_suction_compensation},
    /// 产测完成标识
    {TESTS_TLV_WRITE_FACTORY_COMPLETE_FLAG, tests_tlvs_write_factory_complete_flag},
    {TESTS_TLV_READ_FACTORY_COMPLETE_FLAG, tests_tlvs_read_factory_complete_flag},
    /// 蓝牙测试
    {TESTS_TLV_BT_SIGNALING_TEST, tests_tlvs_bt_signaling_test},
    {TESTS_TLV_BT_NOSIGNALING_TEST, tests_tlvs_bt_nosignaling_test},
    {TESTS_TLV_BT_XTAL_CALIB_TEST, tests_tlvs_bt_xtal_calib_test},
    /// Trim值操作
    {TESTS_TLV_SET_TRIM_VALUE, tests_tlvs_set_trim_value},
    {TESTS_TLV_GET_TRIM_VALUE, tests_tlvs_get_trim_value},
    /// 待机和传感器状态
    {TESTS_TLV_STANDBY_MODE, tests_tlvs_standby_mode},
    {TESTS_TLV_QUERY_SENSOR_STATUS, tests_tlvs_query_sensor_status},
    /// RSSI获取
    {TESTS_TLV_GET_RSSI, tests_tlvs_get_rssi},
    /// 电池电量读取
    {TESTS_TLV_READ_BATTERY_LEVEL, tests_tlvs_read_battery_level},
    /// 电容按键信号读取
    {TESTS_TLV_READ_KEY_CAP_SIGNAL, tests_tlvs_read_key_cap_signal},
    ///LCD背光控制
    {TESTS_TLV_LCD_BACKLIGHT_CONTROL, tests_tlvs_lcd_backlight_control},
    ///光感数据上报
    {TESTS_TLV_LIGHT_SENSOR_REPORT_CTRL, tests_tlvs_light_sensor_report_ctrl},
    ///设置光感校准数据
    {TESTS_TLV_LIGHT_SENSOR_CALIB_DATA_SET, tests_tlvs_light_sensor_calib_data_set},
    ///读取光感校准数据
    {TESTS_TLV_LIGHT_SENSOR_CALIB_DATA_READ, tests_tlvs_light_sensor_calib_data_read},
    ///读取电池充电电流
    {TESTS_TLV_READ_BATTERY_CHARGE_CURRENT, tests_tlvs_read_battery_charge_current},
    ///设置电池充电电流（产测）
    {TESTS_TLV_SET_BATTERY_CHARGE_CURRENT, tests_tlvs_set_battery_charge_current},
    /// LCD颜色测试模式
    {TESTS_TLV_LCD_COLOR_TEST_MODE, tests_tlvs_lcd_color_test_mode},
    /// 泵电机/电池厂商类型
    {TESTS_TLV_WRITE_MOTOR_TYPE, tests_tlvs_write_motor_type},
    {TESTS_TLV_READ_MOTOR_TYPE, tests_tlvs_read_motor_type},
    {TESTS_TLV_WRITE_BATTERY_VENDOR_TYPE, tests_tlvs_write_battery_vendor_type},
    {TESTS_TLV_READ_BATTERY_VENDOR_TYPE, tests_tlvs_read_battery_vendor_type},
    {TESTS_TLV_READ_UI_RES_VERSION, tests_tlvs_read_ui_res_version},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int tests_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(srv_node && srv_node->tlvs);
    if (srv_node->srv_length < COMM_PROTOCOL_TLV_HEADER_SIZE || srv_node->srv_length > COMM_PROTOCOL_MAX_PAYLOAD_SIZE) {
        LOGE("==> tests server protocol service length error: %d", srv_node->srv_length);
        return -1;
    }
    protocol_service_tlvs_handle_list_t ctx = {
        .list = protocol_service_tlvs_list,
        .list_len = ARRAY_SIZE(protocol_service_tlvs_list),
        .name = "tests",
    };

    protocol_service_tlv_func tlv_func = NULL;
    comm_protocol_tlv_t *tlv = NULL;
    uint32_t tlv_offset = 0;

    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    LOG_HEX("==> tests service tlvs", srv_node->tlvs, srv_node->srv_length);
    comm_protocol_service_t *rsp_srv = NULL;
    if (builder != NULL) {
        /// 添加应答服务节点
        rsp_srv = protocol_ctx->comm_protocol_builder_init_service(builder, COMM_PROTOCOL_TESTS_SERVICE);
        if (rsp_srv == NULL) {
            LOGE("==> tests service begin error");
            return -1;
        }
    } else {
        /// 如果builder为空，说明是notify类型，则不需要组装响应帧
        rsp_srv = NULL;
    }
    /// 处理TLV
    for (;;) {
        /// 检查是否已处理完所有TLV
        if (tlv_offset >= srv_node->srv_length) {
            LOG_SRV("==> tests service tlv complete, tlv_offset: %d", tlv_offset);
            break;
        }
        /// 获取TLV
        tlv = (comm_protocol_tlv_t *)(srv_node->tlvs + tlv_offset);
        if (tlv_offset + COMM_PROTOCOL_TLV_HEADER_SIZE + tlv->length > srv_node->srv_length) {
            LOGE("==> tests service tlv length error: %d", tlv->type);
            ret = COMM_PROTOCOL_ERROR_INVALID_TLV_LENGTH;
            break;
        }
        /// 获取TLV函数
        tlv_func = get_protocol_service_tlvs(&ctx, tlv->type);
        if (tlv_func == NULL) {
            LOGW("==> tests service tlv func not found: %d", tlv->type);
            ret = COMM_PROTOCOL_ERROR_INVALID_TLV_ID;
            break;
        }
        /// 调用TLV函数
        ret = tlv_func(builder, rsp_srv, tlv->value, tlv->length);
        if (ret != 0) {
            LOGE("==> tests service tlv func error: %d, tlv_id: %d", ret, tlv->type);
            ret = COMM_PROTOCOL_ERROR_INVALID_TLV_FUNC;
            break;
        }
        /// 更新TLV偏移
        tlv_offset += COMM_PROTOCOL_TLV_HEADER_SIZE + tlv->length;
    }
    /// 如果builder为空，说明是notify类型，则不需要组装响应帧
    if (builder != NULL) {
        /// 添加应答TLV
        uint16_t err_code = (ret == 0) ? 0 : 1;
        comm_protocol_tlv_node_t rsp_tlv_node = {
            .type = PROTOCOL_SERVICE_ERROR_CODE_TLV_ID,
            .length = sizeof(err_code),
            .value = (uint8_t *)&err_code,
        };
        protocol_ctx->comm_protocol_builder_add_tlv(builder, rsp_srv, &rsp_tlv_node);
    }
    return ret;
}
