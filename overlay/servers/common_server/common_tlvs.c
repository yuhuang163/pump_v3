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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
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