#ifndef __TESTS_TLVS_PROTOCOL_H__
#define __TESTS_TLVS_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "comm_protocol_builder.h" // IWYU pragma: keep
#include "servers_manager_api.h" // IWYU pragma: keep

/// TLV ID定义 (产测服务 Service ID = 0x0007)
typedef enum {
    TESTS_TLV_FACTORY_TEST_MODE = 0x0001,      /// 产测模式进入/退出 (0x01:进入, 0x00:退出)
    TESTS_TLV_AGING_TEST_MODE = 0x0002,        /// 老化模式进入 (Length=5: byte0=模式类型, byte1-4=目标时长)
    TESTS_TLV_SUCTION_TEST_MODE = 0x0003,      /// 吸力测试模式进入/退出 (0x01:进入, 0x00:退出)
    TESTS_TLV_AGING_STATUS_QUERY = 0x0004,     /// 老化状态查询
    TESTS_TLV_AGING_TEST_MODE_EXIT = 0x0010,   /// 老化模式退出 (Length=0)
    TESTS_TLV_KEY_STATUS_REPORT = 0x0005,      /// 按键状态上报 NOTIFY (Length=2: byte0=0x01短按/0x02长按, byte1=按键编号0-255)
    TESTS_TLV_ENCODER_STATUS_REPORT = 0x0006, /// 编码器状态上报 NOTIFY (Length=2: byte0=0x01左旋/0x02右旋, byte1=按键编号0-255)
    TESTS_TLV_WRITE_SN = 0x0007,               /// 写入整机序列号SN (35字节)
    TESTS_TLV_WRITE_PRODUCT_ID = 0x0008,       /// 写入整机产品ID (6字节)
    TESTS_TLV_WRITE_DEVICE_ID = 0x0009,        /// 写入整机设备ID (16字节)
    TESTS_TLV_WRITE_SECRET_KEY = 0x000A,       /// 写入整机三元组密钥 (16字节)
    TESTS_TLV_READ_TRIPLE_DATA = 0x000B,       /// 读取整机三元组所有数据
    TESTS_TLV_DEVICE_ERROR_STATUS = 0x000C,    /// 设备异常状态查询
    TESTS_TLV_SUCTION_COMPENSATION = 0x000D,   /// 吸力补偿开关 (0x01:开, 0x00:关)
    TESTS_TLV_READ_SN = 0x000E,                /// 读取整机SN数据
    TESTS_TLV_WRITE_FACTORY_COMPLETE_FLAG = 0x000F, /// 产测完成标识写入 (0x00:未完成, 0x01:完成)
    TESTS_TLV_BT_SIGNALING_TEST = 0x0011,      /// 进入或退出蓝牙信令测试 (0x01:进入, 0x00:退出)
    TESTS_TLV_BT_NOSIGNALING_TEST = 0x0012,    /// 进入或退出蓝牙非信令测试 (0x01:进入, 0x00:退出)
    TESTS_TLV_BT_XTAL_CALIB_TEST = 0x0013,     /// 进入或退出蓝牙校频测试 (0x01:进入, 0x00:退出)
    TESTS_TLV_SET_TRIM_VALUE = 0x0014,         /// 设置设备trim值 (Length=4: byte0-3=trim值, 小端序)
    TESTS_TLV_GET_TRIM_VALUE = 0x0015,         /// 获取设备trim值 (Length=0)
    TESTS_TLV_STANDBY_MODE = 0x0016,           /// 进入或退出待机 (0x01:进入, 0x00:退出)
    TESTS_TLV_QUERY_SENSOR_STATUS = 0x0017,    /// 查询外设sensor状态 (Length=0)
    TESTS_TLV_READ_FACTORY_COMPLETE_FLAG = 0x0018, /// 产测完成标识读取 (Length=0)
    TESTS_TLV_GET_RSSI = 0x0019,               /// 获取蓝牙RSSI (Length=1: byte0=0x00获取BLE RSSI, 0x01获取BT RSSI)
    TESTS_TLV_READ_BATTERY_LEVEL = 0x001A,     /// 读取设备电池电量 (REQUEST Length=0, RESPONSE Length=1: byte0=0-100)
    TESTS_TLV_READ_KEY_CAP_SIGNAL = 0x001B,    /// 读取指定按键电容信号 (REQUEST Length=1: byte0=按键编号0-255, RESPONSE Length=4: 信号值小端uint32，通常为16位差分值)
    TESTS_TLV_LCD_BACKLIGHT_CONTROL = 0x001C,   /// LCD背光控制 (REQUEST Length=1: 0x01开启, 0x00关闭)
    TESTS_TLV_LIGHT_SENSOR_REPORT_CTRL = 0x001D, /// 光感采样上报控制 (REQUEST Length=1: 0x01开启, 0x00关闭)
    TESTS_TLV_LIGHT_SENSOR_CALIB_DATA_SET = 0x001E, /// 光感校准数据设置 (REQUEST Length=5: byte0=校准点位0~29, byte1-4=校准值int32小端有符号; RESPONSE Length=1: 0x00失败/0x01成功)
    TESTS_TLV_LIGHT_SENSOR_CALIB_DATA_READ = 0x001F, /// 光感校准数据读取 (REQUEST Length=1: byte0=校准点位0~29; RESPONSE Length=4: byte0-3=校准值int32小端有符号)
    TESTS_TLV_READ_BATTERY_CHARGE_CURRENT = 0x0020, /// 读取电池充电电流 (REQUEST Length=0; RESPONSE Length=4: byte0-3=电流uint32小端，单位mA)
    TESTS_TLV_LCD_COLOR_TEST_MODE = 0x0021, /// LCD颜色测试模式 (REQUEST Length=1: 0x01进入, 0x00退出; RESPONSE Length=1)
    TESTS_TLV_SET_BATTERY_CHARGE_CURRENT = 0x0022, /// 设置电池充电电流 (REQUEST Length=2: byte0-1=目标电流uint16小端，单位mA; RESPONSE Length=1: 0x00失败/0x01成功)
    TESTS_TLV_ID_MAX,
} tests_tlvs_id_e;

/// 按键状态值
typedef enum {
    TESTS_KEY_STATUS_SHORT_PRESS = 0x01,       /// 短按
    TESTS_KEY_STATUS_LONG_PRESS = 0x02,        /// 长按
} tests_key_status_e;

/// 编码器状态值
typedef enum {
    TESTS_ENCODER_STATUS_LEFT = 0x01,          /// 左旋
    TESTS_ENCODER_STATUS_RIGHT = 0x02,         /// 右旋
} tests_encoder_status_e;

/// 设备异常状态值
typedef enum {
    TESTS_DEVICE_ERROR_NONE = 0x00,            /// 正常
    TESTS_DEVICE_ERROR_LOW_BATTERY = 0x01,     /// 电量低
    TESTS_DEVICE_ERROR_AIR_LEAK = 0x02,        /// 漏气
    TESTS_DEVICE_ERROR_MOTOR_STALL = 0x03,     /// 堵转
} tests_device_error_status_e;

/// 老化状态结果
typedef enum {
    TESTS_AGING_STATUS_NOT_AGE = 0x00,        /// 没有老化
    TESTS_AGING_STATUS_SUCCESS = 0x01, /// 老化成功
    TESTS_AGING_STATUS_FAILED = 0x02,  /// 老化失败
} tests_aging_status_e;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 模式控制
int tests_tlvs_factory_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_aging_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_aging_test_mode_exit(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_suction_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// LCD颜色测试模式
int tests_tlvs_lcd_color_test_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// 老化状态查询
int tests_tlvs_aging_status_query(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 三元组写入
int tests_tlvs_write_sn(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_write_product_id(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_write_device_id(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_write_secret_key(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 三元组读取
int tests_tlvs_read_triple_data(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_read_sn(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 设备状态
int tests_tlvs_device_error_status(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_suction_compensation(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 产测完成标识
int tests_tlvs_write_factory_complete_flag(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_read_factory_complete_flag(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 蓝牙测试
int tests_tlvs_bt_signaling_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_bt_nosignaling_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_bt_xtal_calib_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// Trim值操作
int tests_tlvs_set_trim_value(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_get_trim_value(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 待机和传感器状态
int tests_tlvs_standby_mode(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_query_sensor_status(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// RSSI获取
int tests_tlvs_get_rssi(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_read_battery_level(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int tests_tlvs_read_key_cap_signal(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// LCD背光控制
int tests_tlvs_lcd_backlight_control(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// 光感上报控制
int tests_tlvs_light_sensor_report_ctrl(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// 光感校准数据设置
int tests_tlvs_light_sensor_calib_data_set(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// 光感校准数据读取
int tests_tlvs_light_sensor_calib_data_read(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// 读取电池充电电流（充电芯片采样 ibat）
int tests_tlvs_read_battery_charge_current(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
/// 设置电池充电电流（产测）
int tests_tlvs_set_battery_charge_current(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// NOTIFY上报 (设备主动上报)
int tests_key_status_publish(servers_publish_data_args_t *args);
int tests_encoder_status_publish(servers_publish_data_args_t *args);

/// 服务处理入口
int tests_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);

#ifdef __cplusplus
}
#endif

#endif 
