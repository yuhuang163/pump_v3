/*
* INCLUDE FILES
*****************************************************************************************
*/
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
int ota_tlvs_img_id(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data && data_len == sizeof(uint32_t));
    ota_info.img_id = BYTES_TO_UINT32(data, sizeof(uint32_t), true);
    return 0;
}

int ota_tlvs_version(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data && data_len == sizeof(uint32_t));
    ota_info.version = BYTES_TO_UINT32(data, sizeof(uint32_t), true);
    return 0;
}

int ota_tlvs_crc32(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data && data_len == sizeof(uint32_t));
    ota_info.crc32 = BYTES_TO_UINT32(data, sizeof(uint32_t), true);
    return 0;
}

int ota_tlvs_total_size(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data && data_len == sizeof(uint32_t));
    ota_info.total_size = BYTES_TO_UINT32(data, sizeof(uint32_t), true);
    return 0;
}

int ota_tlvs_type(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data && data_len == 1);
    ota_info.type = data[0];
    return 0;
}

int ota_tlvs_start_upgrade_request(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)data;
    EXCEPTION_CHECK_INT(builder && service);
    if (data_len != 0) {
        return -1;
    }
    if (ota_info.type != OTA_OBJECT_IMAGE_RESOURCE) {
        return -1;
    }
    ota_info.offset_addr = 0;
    uint32_t resume_offset = 0;
    int ret = start_img_res_upgrade(ota_info, &resume_offset);
    if (ret != 0) {
        return ret;
    }
    /// 添加图片偏移地址
    ret = common_tlv_add_u32(builder, service, OTA_TLV_IMG_OFFSET_ADDR, resume_offset);
    if (ret != 0) {
        return ret;
    }
    /// 添加块大小
    ret = common_tlv_add_u16(builder, service, OTA_TLV_BLOCK_SIZE, OTA_BLOCK_SIZE);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int ota_tlvs_write_img_offset_addr(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data && data_len == sizeof(uint32_t));
    ota_info.offset_addr = BYTES_TO_UINT32(data, sizeof(uint32_t), true);
    return 0;
}

int ota_tlvs_write_img_data(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    (void)builder;
    (void)service;
    EXCEPTION_CHECK_INT(data);
    if (data_len > OTA_BLOCK_SIZE || data_len == 0) {
        LOGE("invalid data length: %u", data_len);
        return -1;
    }
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_INT(ctx);
    int ret = ctx->write_image_data(OTA_OBJECT_IMAGE_RESOURCE, ota_info.offset_addr, data, data_len);
    return ret;
}

int ota_tlvs_end_upgrade_request(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    if (data_len != 0) {
        return -1;
    }
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    if (protocol_ctx != NULL && protocol_ctx->comm_protocol_assemble_reset != NULL) {
        protocol_ctx->comm_protocol_assemble_reset();
    }
    /// 添加升级结果
    uint16_t result = (end_img_res_upgrade(ota_info.img_id) == 0) ? OTA_STATUS_CODE_SUCCESS : OTA_STATUS_CODE_FAIL;
    int ret = common_tlv_add_u16(builder, service, OTA_TLV_UPGRADE_RESULT, result);
    /// 重置OTA信息
    memset(&ota_info, 0, sizeof(ota_info));
    return ret;
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

    ota_tlvs_notify_args_t *notify_args = (ota_tlvs_notify_args_t *)args->val;
    comm_protocol_tlv_node_t default_tlv = {0};
    comm_protocol_tlv_node_t *tlvs = NULL;
    uint16_t tlv_count = 0;

    if (notify_args != NULL && notify_args->tlvs != NULL && notify_args->tlv_count > 0) {
        tlvs = notify_args->tlvs;
        tlv_count = notify_args->tlv_count;
        LOG_SRV("tlv_count=%u", tlv_count);
    } else {
        uint16_t error_code = (uint16_t)args->arg1;
        LOG_SRV("error_code=%u", error_code);
        default_tlv.type = OTA_TLV_UPGRADE_STATUS_NOTIFY;
        default_tlv.length = sizeof(error_code);
        default_tlv.value = (uint8_t *)&error_code;
        tlvs = &default_tlv;
        tlv_count = 1;
    }

    /// 初始化构建器
    ret = protocol_ctx->comm_protocol_builder_init(&builder, 256, protocol_ctx->get_packet_seq(),
                                                   COMM_PROTOCOL_FRAME_TYPE_NOTIFY);
    if (ret != 0) {
        LOGE("==> builder initialize failed");
        return COMM_PROTOCOL_ERROR_INVALID_BUILDER;
    }

    /// 初始化服务节点
    comm_protocol_service_t *service =
        protocol_ctx->comm_protocol_builder_init_service(&builder, COMM_PROTOCOL_OTA_SERVICE);
    if (service == NULL) {
        LOGE("==> service initialize failed");
        goto ERROR_EXIT;
    }

    for (uint16_t i = 0; i < tlv_count; i++) {
        ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &tlvs[i]);
        if (ret != 0) {
            LOGE("==> tlv add failed, index=%u, type=0x%04x", i, tlvs[i].type);
            goto ERROR_EXIT;
        }
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