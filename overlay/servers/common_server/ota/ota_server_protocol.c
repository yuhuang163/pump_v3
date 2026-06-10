/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "common_tlvs.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static protocol_service_tlvs_handle_t protocol_service_tlvs_list[] = {
    {OTA_TLV_IMG_ID, ota_tlvs_img_id},
    {OTA_TLV_VERSION, ota_tlvs_version},
    {OTA_TLV_CRC32, ota_tlvs_crc32},
    {OTA_TLV_TOTAL_SIZE, ota_tlvs_total_size},
    {OTA_TLV_TYPE, ota_tlvs_type},
    {OTA_TLV_IMG_OFFSET_ADDR, ota_tlvs_write_img_offset_addr},
    {OTA_TLV_WRITE_IMG_DATA, ota_tlvs_write_img_data},
    {OTA_TLV_START_UPGRADE_REQUEST, ota_tlvs_start_upgrade_request},
    {OTA_TLV_END_UPGRADE_REQUEST, ota_tlvs_end_upgrade_request},
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
int ota_subscribe_service_handle(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(srv_node && srv_node->tlvs);
    if (srv_node->srv_length < COMM_PROTOCOL_TLV_HEADER_SIZE || srv_node->srv_length > COMM_PROTOCOL_MAX_PAYLOAD_SIZE) {
        LOGE("==> service length error: %d", srv_node->srv_length);
        return -1;
    }
    protocol_service_tlvs_handle_list_t ctx = {
        .list = protocol_service_tlvs_list,
        .list_len = ARRAY_SIZE(protocol_service_tlvs_list),
        .name = "ota",
    };
    
    protocol_service_tlv_func tlv_func =  NULL;
    comm_protocol_tlv_t *tlv = NULL;
    uint32_t tlv_offset = 0;

    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    // LOG_HEX("==> ota service tlvs", srv_node->tlvs, srv_node->srv_length);
    comm_protocol_service_t *rsp_srv = NULL;
    if (builder != NULL) {
        /// 添加应答服务节点
        rsp_srv = protocol_ctx->comm_protocol_builder_init_service(builder, COMM_PROTOCOL_OTA_SERVICE);
        if (rsp_srv == NULL) {
            LOGE("==> ota service begin error");
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
            LOG_SRV("==> ota service tlv complete, tlv_offset: %d", tlv_offset);
            break;
        }
        /// 获取TLV
        tlv = (comm_protocol_tlv_t *)(srv_node->tlvs + tlv_offset);
        if (tlv_offset + COMM_PROTOCOL_TLV_HEADER_SIZE + tlv->length > srv_node->srv_length) {
            LOGE("==> ota service tlv length error: %d", tlv->type);
            ret = COMM_PROTOCOL_ERROR_INVALID_TLV_LENGTH;
            break;
        }
        /// 获取TLV函数
        tlv_func = get_protocol_service_tlvs(&ctx, tlv->type);
        if (tlv_func == NULL) {
            LOGW("==> ota service tlv func not found: %d", tlv->type);
            ret = COMM_PROTOCOL_ERROR_INVALID_TLV_ID;
            break;
        }
        /// 调用TLV函数
        ret = tlv_func(builder, rsp_srv, tlv->value, tlv->length);
        if (ret != 0) {
            LOGE("==> ota service tlv func error: %d, tlv_id: %d", ret, tlv->type);
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