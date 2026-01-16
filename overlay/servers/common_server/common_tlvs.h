#ifndef __COMMON_TLVS_H__
#define __COMMON_TLVS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep
#include "comm_protocol_defs.h" // IWYU pragma: keep
#include "algo_tlvs_protocol.h" // IWYU pragma: keep
#include "pump_tlvs_protocol.h" // IWYU pragma: keep
#include "iot_tlvs_protocol.h" // IWYU pragma: keep
#include "sensors_tlvs_protocol.h" // IWYU pragma: keep
#include "sys_config_tlvs_protocol.h" // IWYU pragma: keep
#include "sys_state_tlvs_protocol.h" // IWYU pragma: keep
#include "tests_tlvs_protocol.h" // IWYU pragma: keep
#include "tracking_data_tlvs_protocol.h" // IWYU pragma: keep

/// TLVS处理接口.
typedef int (*protocol_service_tlv_func)(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len);

/// 描述写入存储处理结构.
typedef struct {
    uint16_t tlv_id;                        /// TLV ID
    protocol_service_tlv_func tlv_func;     /// TLV功能函数.
} protocol_service_tlvs_handle_t;

typedef struct {
    char *name;
    const protocol_service_tlvs_handle_t *list;
    uint16_t list_len;
} protocol_service_tlvs_handle_list_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
protocol_service_tlv_func get_protocol_service_tlvs(protocol_service_tlvs_handle_list_t *ctx, uint16_t tlv_id);

#ifdef __cplusplus
}
#endif

#endif // __PROTOCOL_SERVICE_H__

