#ifndef __OTA_TLVS_PROTOCOL_H__
#define __OTA_TLVS_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "comm_protocol_builder.h" // IWYU pragma: keep
#include "servers_manager_api.h" // IWYU pragma: keep

/// TLV ID
typedef enum {
    OTA_TLV_START_UPGRADE_REQUEST = 0x0001,          /// 开始升级请求
    OTA_TLV_START_UPGRADE_RESPONSE = 0x0002,         /// 开始升级响应
    OTA_TLV_WRITE_IMG_DATA = 0x0003,                 /// 写图片数据
    OTA_TLV_END_UPGRADE_REQUEST = 0x0004,            /// 结束升级请求
    OTA_TLV_END_UPGRADE_RESPONSE = 0x0005,           /// 结束升级响应
    OTA_TLV_UPGRADE_STATUS_NOTIFY = 0x0006,          /// 升级状态通知
    OTA_TLV_ID_MAX,
} ota_tlvs_id_e;

typedef struct {
    uint32_t img_id;
    uint32_t total_size;
    uint32_t crc32;
    uint32_t version;
    uint16_t type;
    uint16_t block_size;
} ota_img_info_t;

typedef enum {
    OTA_STATUS_CODE_SUCCESS = 0x0000,
    OTA_STATUS_CODE_FAIL = 0x0001,
    OTA_STATUS_CODE_CRC_ERROR = 0x0002,
    OTA_STATUS_CODE_VERSION_ERROR = 0x0003,
    OTA_STATUS_CODE_TYPE_ERROR = 0x0004,
    OTA_STATUS_CODE_BLOCK_SIZE_ERROR = 0x0005,
    OTA_STATUS_CODE_DATA_SIZE_ERROR = 0x0006,
    OTA_STATUS_CODE_DATA_CRC_ERROR = 0x0007,
    OTA_STATUS_CODE_DATA_VERSION_ERROR = 0x0008,
    OTA_STATUS_CODE_DATA_WRITE_ERROR = 0x0009,
} ota_upgrade_status_code_e;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
int ota_tlvs_start_upgrade_request(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int ota_tlvs_write_img_data(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int ota_tlvs_end_upgrade_request(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);
int ota_tlvs_upgrade_status_notify(servers_publish_data_args_t *args);
#ifdef __cplusplus
}
#endif

#endif 