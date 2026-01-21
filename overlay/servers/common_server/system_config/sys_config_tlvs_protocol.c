/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "handler_post.h"
#include "peripheral_bus_api.h"
#include "root_common.h"
#include "root_common_api.h"      // IWYU pragma: keep
#include "common_tlvs.h"          // IWYU pragma: keep
#include "storage_manager_api.h"  // IWYU pragma: keep
#include "protocol_manager_api.h" // IWYU pragma: keep
#include "ble_manager_api.h"      // IWYU pragma: keep

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

static void test_led_post_task_handler(hdr_post_args_t *args)
{
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    EXCEPTION_CHECK_VOID(pbus);
    rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
    EXCEPTION_CHECK_VOID(led_dev);
    LOG_SRV("test_led_post_task_handler");
    led_status_msg_t msg = {
        .led_index = PUMP_LED_ALL_ID,
        .led_status = {
            .mode = LED_MODE_ONOFF,
            .un.onoff.status = LED_STATUS_ON,
        },
    };
    led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
    msg.led_index = NIGHT_LIGHT_LED_ID_0;
    msg.led_status.mode = LED_MODE_LEVEL;
    msg.led_status.un.luminous.level = LED_LEVEL_MAX;
    led_dev->ioctl(led_dev, OP_CODE_NIGHT_LIGHT_STATUS_SET, (void *)&msg);
}

static void test_factory_reset_post_task_handler(hdr_post_args_t *args)
{
    LOG_SRV("test_factory_reset_post_task_handler");
    storage_manager_context_t *st = get_storage_ctx();
    if (st && st->factory_reset) {
        LOG_SRV("factory_reset have executed after response sent");
        st->factory_reset(RESET_AND_REBOOT);
    }
    LOG_SRV("factory_reset not executed after response sent");
}
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/// ======================== 读取固件版本 ========================

int sys_config_tlvs_read_firmware_version(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                                          uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("sys_config_tlvs_read_firmware_version");

    // 版本格式: 3字节 byte0-2，例如 0x00 0x01 0x01 表示版本 11.1
    uint8_t version[SYS_CONFIG_FIRMWARE_VERSION_LENGTH] = {0};

    storage_manager_context_t *st = get_storage_ctx();
    if (st && st->get_persist_ver_info) {
        sec_sys_ver_t *ver_info = st->get_persist_ver_info();
        if (ver_info) {
            // sw_version格式转换: 例如 111 -> 0x00 0x0B 0x01 (版本11.1)
            // 假设 sw_version = 主版本*10 + 次版本
            uint16_t sw_ver = ver_info->sw_version;
            version[0] = 0x00;                // byte0: 高位或保留
            version[1] = sw_ver / 100; // byte1: 主版本号 (0-99)
            version[2] = sw_ver % 100;         // byte2: 次版本号 (0-99)
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = SYS_CONFIG_TLV_READ_FIRMWARE_VERSION,
        .length = SYS_CONFIG_FIRMWARE_VERSION_LENGTH,
        .value = version,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== MAC地址读写 ========================

int sys_config_tlvs_read_mac_address(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                     uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("sys_config_tlvs_read_mac_address");

    uint8_t mac_addr[SYS_CONFIG_MAC_ADDRESS_LENGTH] = {0};

    ble_manager_context_t *ble_ctx = get_ble_ctx();
    if (ble_ctx && ble_ctx->get_mac_addr) {
        ble_ctx->get_mac_addr(mac_addr);
        LOG_HEX("read_mac_address", mac_addr, SYS_CONFIG_MAC_ADDRESS_LENGTH);
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = SYS_CONFIG_TLV_READ_MAC_ADDRESS,
        .length = SYS_CONFIG_MAC_ADDRESS_LENGTH,
        .value = mac_addr,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int sys_config_tlvs_write_mac_address(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                      uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);

    LOG_SRV("sys_config_tlvs_write_mac_address, data_len: %d", data_len);

    if (data && data_len == SYS_CONFIG_MAC_ADDRESS_LENGTH) {
        LOG_HEX("write_mac_address", data, SYS_CONFIG_MAC_ADDRESS_LENGTH);

        ble_manager_context_t *ble_ctx = get_ble_ctx();
        if (ble_ctx && ble_ctx->set_mac_addr) {
            ble_ctx->set_mac_addr(data);
        }
    }

    // 应答无数据（Length=0），仅返回错误码TLV
    return 0;
}

/// ======================== 设备关机 ========================

int sys_config_tlvs_device_shutdown(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                    uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("sys_config_tlvs_device_shutdown");

    uint8_t result = 0x01; // 默认成功

    // TODO: 实现设备关机逻辑
    // 可能需要延时关机，先发送应答后再关机 ，使用广播发送关机命令，已经使用延时投递关机命令
    SEND_BROADCAST_ARGS1(ID_BROADCAST_SYS_POWER_STATE_CHANGE, ID_BROADCAST_SYS_POWER_OFF);

    comm_protocol_tlv_node_t tlv_node = {
        .type = SYS_CONFIG_TLV_DEVICE_SHUTDOWN,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 显示测试 ========================

int sys_config_tlvs_display_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                 uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("sys_config_tlvs_display_test, data_len: %d", data_len);

    uint8_t result = 0x00; // 默认失败

    // 检查请求数据长度，应该为1字节
    if (!data || data_len != 1) {
        LOGE("sys_config_tlvs_display_test: invalid data_len: %d", data_len);
        result = 0x00; // 失败
    } else {
        uint8_t cmd = data[0];
        LOG_SRV("sys_config_tlvs_display_test, cmd: 0x%02x", cmd);

        peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
        if (pbus == NULL) {
            LOGE("sys_config_tlvs_display_test: pbus is NULL");
            result = 0x00; // 失败
        } else {
            rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
            if (led_dev == NULL) {
                LOGE("sys_config_tlvs_display_test: led_dev is NULL");
                result = 0x00; // 失败
            } else {
                // 根据命令控制LED：0x00全灭，0x01全亮
                if (cmd == 0x00) {
                    // 全灭
                    led_status_msg_t msg = {
                        .led_index = PUMP_LED_ALL_ID,
                        .led_status = {
                            .mode = LED_MODE_ONOFF,
                            .un.onoff.status = LED_STATUS_OFF,
                        },
                    };
                    led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
                    msg.led_index = NIGHT_LIGHT_LED_ID_0;
                    msg.led_status.mode = LED_MODE_LEVEL;
                    msg.led_status.un.luminous.level = 0;
                    led_dev->ioctl(led_dev, OP_CODE_NIGHT_LIGHT_STATUS_SET, (void *)&msg);
                    result = 0x01; // 成功
                } else if (cmd == 0x01) {
                    // 全亮
                    led_status_msg_t msg = {
                        .led_index = PUMP_LED_ALL_ID,
                        .led_status = {
                            .mode = LED_MODE_ONOFF,
                            .un.onoff.status = LED_STATUS_ON,
                        },
                    };
                    led_dev->ioctl(led_dev, OP_CODE_LED_STATUS_SET, (void *)&msg);
                    msg.led_index = NIGHT_LIGHT_LED_ID_0;
                    msg.led_status.mode = LED_MODE_LEVEL;
                    msg.led_status.un.luminous.level = LED_LEVEL_MAX;
                    led_dev->ioctl(led_dev, OP_CODE_NIGHT_LIGHT_STATUS_SET, (void *)&msg);
                    // 100ms秒后恢复全亮状态（如果需要）
                    POST_TASK_HANDLER_DELAYED_ARG0(test_led_post_task_handler, 100, "display_test_cb");
                    result = 0x01; // 成功
                } else {
                    LOGE("sys_config_tlvs_display_test: invalid cmd: 0x%02x", cmd);
                    result = 0x00; // 失败：无效的命令值
                }
            }
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = SYS_CONFIG_TLV_DISPLAY_TEST,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 夜灯设置 ========================

int sys_config_tlvs_set_night_light(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                    uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    uint8_t level = (data && data_len >= 1) ? data[0] : 0;
    LOG_SRV("sys_config_tlvs_set_night_light, level: %d", level);

    uint8_t result = 0x00; // 默认失败

    // 检查亮度范围
    if (level <= SYS_CONFIG_NIGHT_LIGHT_MAX) {
        storage_manager_context_t *st = get_storage_ctx();
        if (st) {
            night_light_info_t night_light_info = {0};

            // 更新夜灯亮度
            night_light_info.level = level;
            night_light_info.onoff = (level > 0) ? 1 : 0;

            // TODO: 实际控制夜灯硬件
            peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
            EXCEPTION_CHECK_INT(pbus);
            rt_device_node_t *led_dev = pbus->find_device(ID_DEVICE_LED);
            EXCEPTION_CHECK_INT(led_dev);
            led_status_msg_t msg = {
                .led_index = NIGHT_LIGHT_LED_ID_0,
                .led_status = {
                    .mode = LED_MODE_LEVEL,
                    .un.luminous.level = night_light_info.onoff ? night_light_info.level : 0,
                },
            };
            led_dev->ioctl(led_dev, OP_CODE_NIGHT_LIGHT_STATUS_SET, (void *)&msg);
            result = 0x01; // 成功
        }
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = SYS_CONFIG_TLV_SET_NIGHT_LIGHT,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/// ======================== 恢复出厂设置 ========================

int sys_config_tlvs_factory_reset(comm_protocol_builder_t *builder, comm_protocol_service_t *service, uint8_t *data,
                                  uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    protocol_manager_context_t *protocol_ctx = get_protocol_ctx();
    EXCEPTION_CHECK_INT(protocol_ctx);

    LOG_SRV("sys_config_tlvs_factory_reset");

    uint8_t result = 0x01; // 默认成功
    POST_TASK_HANDLER_DELAYED_ARG0(test_factory_reset_post_task_handler, 1000, "factory_reset_cb");
    
    comm_protocol_tlv_node_t tlv_node = {
        .type = SYS_CONFIG_TLV_FACTORY_RESET,
        .length = 1,
        .value = &result,
    };
    return protocol_ctx->comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}
