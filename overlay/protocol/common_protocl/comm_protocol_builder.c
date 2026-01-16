/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "comm_protocol_builder.h"
#include "comm_protocol_defs.h"

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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int comm_protocol_builder_init(comm_protocol_builder_t *builder, uint16_t max_size, uint16_t seq, uint8_t frame_type)
{
    EXCEPTION_CHECK_INT(builder);
    if (max_size < COMM_PROTOCOL_FRAME_SIZE_MIN || max_size > COMM_PROTOCOL_FRAME_SIZE_MAX) {
        LOGE("invalid max size: %d", max_size);
        return -1;
    }
    builder->frame = (comm_protocol_frame_t *)u_malloz(max_size);
    EXCEPTION_CHECK_INT(builder->frame);
    builder->payload_size = max_size - COMM_PROTOCOL_PAYLOAD_SIZE_MIN;
    builder->frame->sof = COMM_PROTOCOL_SOF;
    builder->frame->seq = seq;
    builder->frame->frame_type = frame_type;
    builder->frame->payload_length = 0;
    return 0;
}

comm_protocol_service_t *comm_protocol_builder_init_service(comm_protocol_builder_t *builder, uint16_t service_id)
{
    comm_protocol_service_t *service = NULL;
    EXCEPTION_CHECK_POINT(builder);
    EXCEPTION_CHECK_POINT(builder->frame);
    if (builder->frame->payload_length + COMM_PROTOCOL_SERVICE_SIZE_MIN > builder->payload_size) {
        LOGE("payload size not enough: %d", builder->frame->payload_length);
        return NULL;
    }

    uint8_t *payload = (uint8_t*)builder->frame + builder->frame->payload_length + COMM_PROTOCOL_HEADER_SIZE;
    service = (comm_protocol_service_t *)payload;
    service->svr_id = service_id;
    service->srv_length = 0; 
    builder->frame->payload_length += COMM_PROTOCOL_SERVICE_HEADER_SIZE; 

    return service;
}

int comm_protocol_builder_add_tlv(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    comm_protocol_tlv_node_t *tlv_node)
{
    uint16_t tlv_size = 0;
    /// 验证参数
    EXCEPTION_CHECK_INT(builder && service && tlv_node);
    if (tlv_node->value == NULL && tlv_node->length > 0) {
        LOGE("invalid tlv: %d", tlv_node->length);
        return -1;
    }
    /// 验证TLV空间是否足够
    if (tlv_node->length + COMM_PROTOCOL_TLV_HEADER_SIZE + builder->frame->payload_length > builder->payload_size) {
        LOGE("payload size not enough: %d", tlv_node->length + builder->frame->payload_length);
        return -1;
    }
    /// 添加TLV
    comm_protocol_tlv_t *tlv = (comm_protocol_tlv_t *)(service->tlvs + service->srv_length);
    tlv->type = tlv_node->type;
    tlv->length = tlv_node->length;
    /// 计算TLV大小
    if (tlv_node->value != NULL) {
        memcpy(tlv->value, tlv_node->value, tlv_node->length);
    } 
    /// 更新Service长度和Payload长度
    tlv_size = tlv->length + COMM_PROTOCOL_TLV_HEADER_SIZE;
    service->srv_length += tlv_size;
    builder->frame->payload_length += tlv_size;
    return 0;
}

uint8_t* comm_protocol_builder_finalize(comm_protocol_builder_t *builder, uint16_t *out_size)
{
    EXCEPTION_CHECK_POINT(builder);
    EXCEPTION_CHECK_POINT(builder->frame);
    EXCEPTION_CHECK_POINT(out_size);
    uint16_t frame_size = COMM_PROTOCOL_HEADER_SIZE + builder->frame->payload_length + COMM_PROTOCOL_FOOTER_SIZE;
    uint8_t *buffer = (uint8_t *)builder->frame;
    uint16_t checksum = CRC16(buffer, COMM_PROTOCOL_HEADER_SIZE + builder->frame->payload_length);
    uint16_t footer_offset = COMM_PROTOCOL_HEADER_SIZE + builder->frame->payload_length;
    buffer[footer_offset] = checksum & 0xFF;         // 低字节在前（小端序）
    buffer[footer_offset + 1] = (checksum >> 8) & 0xFF;  // 高字节在后（小端序）
    *out_size = frame_size;
    return buffer;
}

void comm_protocol_builder_free(comm_protocol_builder_t *builder)
{
    EXCEPTION_CHECK_VOID(builder && builder->frame);
    u_free(builder->frame);
    builder->frame = NULL;
    builder->payload_size = 0;
}
