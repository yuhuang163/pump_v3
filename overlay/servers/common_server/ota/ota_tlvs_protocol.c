/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ble_manager_api.h"
#include "ota_manager_api.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "common_tlvs.h" // IWYU pragma: keep
#include "img_res_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ota_img_info_t ota_info;

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
int ota_tlvs_start_upgrade_request(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service && data);
    ota_info.img_id     = BYTES_TO_UINT32(data, 4, true);
    ota_info.total_size = BYTES_TO_UINT32(data + 4, 4, true);
    ota_info.version    = BYTES_TO_UINT32(data + 8, 4, true);
    ota_info.crc32      = BYTES_TO_UINT32(data + 12, 4, true);
    ota_info.type       = data[16];
    if (ota_info.type != OTA_OBJECT_IMAGE_RESOURCE) {
        return -1;
    }
    uint32_t offset_addr = 0;
    start_img_res_upgrade(ota_info, &offset_addr);
    uint8_t rsp_data[4 + 2] = {0};
    UINT32_TO_BYTES(offset_addr, rsp_data, true);
    UINT16_TO_BYTES(OTA_BLOCK_SIZE, rsp_data + 4, true);
    comm_protocol_tlv_node_t tlv_node = {
        .type = OTA_TLV_START_UPGRADE_RESPONSE,
        .length = sizeof(rsp_data),
        .value = (uint8_t *)rsp_data,
    };
    return comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int ota_tlvs_block_size_negotiation(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service && data);
    return 0;
}

int ota_tlvs_write_img_data(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service && data);
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_INT(ctx);
    uint32_t offset_addr = BYTES_TO_UINT32(data, 4, true);
    return ctx->write_image_data(OTA_OBJECT_IMAGE_RESOURCE, offset_addr, data + 4, data_len - 4);
}

int ota_tlvs_end_upgrade_request(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service && data);
    uint32_t img_id = BYTES_TO_UINT32(data, 4, true);
    int ret = end_img_res_upgrade(img_id);
    uint8_t result = (ret == 0) ? OTA_STATUS_CODE_SUCCESS : OTA_STATUS_CODE_FAIL;
    comm_protocol_tlv_node_t tlv_node = {
        .type = OTA_TLV_END_UPGRADE_RESPONSE,
        .length = 1,
        .value = (uint8_t *)&result,
    };
    return comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

int ota_tlvs_upgrade_status_notify(servers_publish_data_args_t *args)
{
    int ret = COMM_PROTOCOL_ERROR_NONE;
    EXCEPTION_CHECK_INT(args);
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_INT(ble_ctx);
    comm_protocol_builder_t builder = {0};

    uint16_t error_code = (uint16_t)args->arg1; 
    LOG_SRV("error_code=%u", error_code);

    /// 初始化构建器
    ret = protocol_ctx->comm_protocol_builder_init(&builder, 256, protocol_ctx->get_packet_seq(),
                                                   COMM_PROTOCOL_FRAME_TYPE_NOTIFY);
    if (ret != 0) {
        LOGE("==> builder initialize failed");
        return COMM_PROTOCOL_ERROR_INVALID_BUILDER;
    }

    /// 初始化服务节点
    comm_protocol_service_t *service =
        protocol_ctx->comm_protocol_builder_init_service(&builder, COMM_PROTOCOL_TESTS_SERVICE);
    if (service == NULL) {
        LOGE("==> service initialize failed");
        goto ERROR_EXIT;
    }

    comm_protocol_tlv_node_t tlv_node = {
        .type = OTA_TLV_UPGRADE_STATUS_NOTIFY,
        .length = 2,
        .value = (uint8_t *)&error_code,
    };
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &tlv_node);
    if (ret != 0) {
        LOGE("==> tlv add failed");
        goto ERROR_EXIT;
    }

    /// 最终化构建器
    uint16_t out_size = 0;
    uint8_t *data = protocol_ctx->comm_protocol_builder_finalize(&builder, &out_size);
    if (data == NULL) {
        LOGE("==> builder finalize failed");
        goto ERROR_EXIT;
    }

    /// 发送数据
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_REMOT_SEND_DATA, data, out_size);

ERROR_EXIT:
    protocol_ctx->comm_protocol_builder_free(&builder);
    return ret;
}