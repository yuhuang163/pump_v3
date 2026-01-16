#ifndef __SYS_CONFIG_TLVS_PROTOCOL_H__
#define __SYS_CONFIG_TLVS_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "comm_protocol_builder.h" // IWYU pragma: keep

/// TLV ID定义 (系统配置服务 Service ID = 0x0005)
typedef enum {
    SYS_CONFIG_TLV_READ_FIRMWARE_VERSION = 0x0001,   /// 读取固件版本
    SYS_CONFIG_TLV_READ_MAC_ADDRESS = 0x0002,        /// 读取设备MAC地址
    SYS_CONFIG_TLV_WRITE_MAC_ADDRESS = 0x0003,       /// 写入设备MAC地址 (6字节)
    SYS_CONFIG_TLV_DEVICE_SHUTDOWN = 0x0004,         /// 设置设备关机
    SYS_CONFIG_TLV_DISPLAY_TEST = 0x0005,            /// 进入显示外设全亮8s
    SYS_CONFIG_TLV_SET_NIGHT_LIGHT = 0x0006,         /// 设置夜灯亮度 (0x00-0x10)
    SYS_CONFIG_TLV_FACTORY_RESET = 0x0007,           /// 系统恢复出厂设置
    SYS_CONFIG_TLV_ID_MAX,
} sys_config_tlvs_id_e;

/// 夜灯亮度范围
#define SYS_CONFIG_NIGHT_LIGHT_MIN      0x00
#define SYS_CONFIG_NIGHT_LIGHT_MAX      0x10

/// MAC地址长度
#define SYS_CONFIG_MAC_ADDRESS_LENGTH   6

/// 固件版本长度
#define SYS_CONFIG_FIRMWARE_VERSION_LENGTH  3

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 读取固件版本
int sys_config_tlvs_read_firmware_version(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// MAC地址读写
int sys_config_tlvs_read_mac_address(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int sys_config_tlvs_write_mac_address(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 设备关机
int sys_config_tlvs_device_shutdown(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 显示测试
int sys_config_tlvs_display_test(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 夜灯设置
int sys_config_tlvs_set_night_light(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 恢复出厂设置
int sys_config_tlvs_factory_reset(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 服务处理入口
int sys_config_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);

#ifdef __cplusplus
}
#endif

#endif 
