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

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int common_tlv_add(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                          uint16_t tlv_type, const uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service);
    if (data_len > 0 && data == NULL) {
        return -1;
    }
    comm_protocol_tlv_node_t tlv_node = {
        .type = tlv_type,
        .length = data_len,
        .value = (uint8_t *)data,
    };
    return comm_protocol_builder_add_tlv(builder, service, &tlv_node);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int common_tlv_add_u8(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                      uint16_t tlv_type, uint8_t value)
{
    return common_tlv_add(builder, service, tlv_type, &value, sizeof(value));
}

int common_tlv_add_u16(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                       uint16_t tlv_type, uint16_t value)
{
    uint8_t bytes[2] = {0};
    UINT16_TO_BYTES(value, bytes, true);
    return common_tlv_add(builder, service, tlv_type, bytes, sizeof(bytes));
}

int common_tlv_add_u32(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                       uint16_t tlv_type, uint32_t value)
{
    uint8_t bytes[4] = {0};
    UINT32_TO_BYTES(value, bytes, true);
    return common_tlv_add(builder, service, tlv_type, bytes, sizeof(bytes));
}

int common_tlv_add_bytes(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                         uint16_t tlv_type, const uint8_t *data, uint16_t data_len)
{
    return common_tlv_add(builder, service, tlv_type, data, data_len);
}

int common_tlv_add_empty(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                         uint16_t tlv_type)
{
    return common_tlv_add(builder, service, tlv_type, NULL, 0);
}

protocol_service_tlv_func get_protocol_service_tlvs(protocol_service_tlvs_handle_list_t *ctx, uint16_t tlv_id)
{
    EXCEPTION_CHECK_POINT(ctx);
    EXCEPTION_CHECK_POINT(ctx->list);
    for (int i = 0; i < ctx->list_len; i++) {
        if (ctx->list[i].tlv_id == tlv_id) {
            return ctx->list[i].tlv_func;
        }
    }
    LOGE("==> get protocol service tlv func failed: %s, tlv_id: %d", ctx->name, tlv_id);
    return NULL;
}