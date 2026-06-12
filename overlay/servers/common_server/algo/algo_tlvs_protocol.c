/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "algorithm_manager.h"  // IWYU pragma: keep
#include "common_tlvs.h" // IWYU pragma: keep
#include <stdint.h>

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
int algo_tlvs_sample_onoff_set(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
#if IS_SUPPORT_ALGO_SAMPLING
    EXCEPTION_CHECK_INT(builder && service && data);
    algo_manager_context_t *algo_ctx = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    EXCEPTION_CHECK_INT(algo_ctx && algo_ctx->hook && algo_ctx->hook->algo_sampling_start && algo_ctx->hook->algo_sampling_stop);
    if (data[0] == 1) {
        algo_ctx->hook->algo_sampling_start(algo_ctx);
    } else {
        algo_ctx->hook->algo_sampling_stop(algo_ctx);
    }
#endif
    LOG_SRV("algo_tlvs_sample_onoff_set, data: %d", data[0]);
    return 0;
}

int algo_tlvs_sample_onoff_get(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
#if IS_SUPPORT_ALGO_SAMPLING
    EXCEPTION_CHECK_INT(builder && service && data);
    algo_manager_context_t *algo_ctx = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    EXCEPTION_CHECK_INT(algo_ctx && algo_ctx->hook && algo_ctx->hook->algo_sampling_is_sampling);
    uint8_t onoff = algo_ctx->hook->algo_sampling_is_sampling(algo_ctx);
    comm_protocol_tlv_node_t tlv_node = {
        .type = ALGO_TLV_SAMPLE_ONOFF_GET,
        .length = 1,
        .value = &onoff,
    };
    return comm_protocol_builder_add_tlv(builder, service, &tlv_node);
#endif
    return 0;
}

int algo_tlvs_sample_interval_set(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service && data);
    LOG_SRV("algo_tlvs_sample_interval_set, data: %d", data[0]);
    return 0;
}

int algo_tlvs_sample_interval_get(comm_protocol_builder_t *builder, comm_protocol_service_t *service,
                    uint8_t *data, uint16_t data_len)
{
    EXCEPTION_CHECK_INT(builder && service && data);
    return 0;
}

int algo_sampling_data_publish(servers_publish_data_args_t *args)
{
    int ret = COMM_PROTOCOL_ERROR_NONE;
    EXCEPTION_CHECK_INT(args);
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_INT(ble_ctx);
    comm_protocol_builder_t builder = {0};
    /// 初始化构建器
    ret = protocol_ctx->comm_protocol_builder_init(&builder, COMM_PROTOCOL_MAX_PAYLOAD_SIZE,
        protocol_ctx->get_packet_seq(), COMM_PROTOCOL_FRAME_TYPE_NOTIFY);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_init failed");
        return COMM_PROTOCOL_ERROR_INVALID_BUILDER;
    }
    /// 初始化服务节点
    comm_protocol_service_t *service = protocol_ctx->comm_protocol_builder_init_service(&builder, COMM_PROTOCOL_ALGO_SERVICE);
    if (service == NULL) {
        LOGE("==> algo service begin error");
        goto ERROR_EXIT;
    }
    comm_protocol_tlv_node_t rsp_tlv_node = {0};
    /// 添加数据个数TLV
    rsp_tlv_node.type = ALGO_TLV_SAMPLE_DATA_NUM;
    rsp_tlv_node.length = 2;
    rsp_tlv_node.value = (uint8_t *)&args->arg1;
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &rsp_tlv_node);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_add_tlv failed");
        goto ERROR_EXIT;
    }
    /// 添加气压数据TLV
    rsp_tlv_node.type = ALGO_TLV_SAMPLE_AIR_PRESSURE_DATA;
    rsp_tlv_node.length = sizeof(algo_sampling_data_t) * args->arg1;
    rsp_tlv_node.value = (uint8_t *)args->val;
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &rsp_tlv_node);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_add_tlv failed");
        goto ERROR_EXIT;
    }
    /// 最终化构建器
    uint16_t out_size = 0;
    uint8_t *data = protocol_ctx->comm_protocol_builder_finalize(&builder, &out_size);
    if (data == NULL) {
        LOGE("==> comm_protocol_builder_finalize failed");
        goto ERROR_EXIT;
    }
    /// 发送数据
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_FAC_SEND_DATA, (uint8_t *)data, out_size);

ERROR_EXIT:
    protocol_ctx->comm_protocol_builder_free(&builder);
    return ret;
}

int algo_light_sensor_data_publish(servers_publish_data_args_t *args)
{
    int ret = COMM_PROTOCOL_ERROR_NONE;
    EXCEPTION_CHECK_INT(args);
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    EXCEPTION_CHECK_INT(ble_ctx);
    comm_protocol_builder_t builder = {0};
    /// 初始化构建器
    ret = protocol_ctx->comm_protocol_builder_init(&builder, 256, protocol_ctx->get_packet_seq(), COMM_PROTOCOL_FRAME_TYPE_NOTIFY);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_init failed");
        return COMM_PROTOCOL_ERROR_INVALID_BUILDER;
    }
    /// 初始化服务节点
    comm_protocol_service_t *service = protocol_ctx->comm_protocol_builder_init_service(&builder, COMM_PROTOCOL_ALGO_SERVICE);
    if (service == NULL) {
        LOGE("==> algo service begin error");
        goto ERROR_EXIT;
    }
    comm_protocol_tlv_node_t rsp_tlv_node = {0};
    /// 添加数据个数TLV
    rsp_tlv_node.type = ALGO_TLV_SAMPLE_DATA_NUM;
    rsp_tlv_node.length = 2;
    rsp_tlv_node.value = (uint8_t *)&args->arg1;
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &rsp_tlv_node);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_add_tlv failed");
        goto ERROR_EXIT;
    }
    /// 添加光感数据TLV
    rsp_tlv_node.type = ALGO_TLV_SAMPLE_LIGHT_SENSOR_DATA;
    rsp_tlv_node.length = sizeof(uint16_t) * args->arg1;
    rsp_tlv_node.value = (uint8_t *)args->val;
    ret = protocol_ctx->comm_protocol_builder_add_tlv(&builder, service, &rsp_tlv_node);
    if (ret != 0) {
        LOGE("==> comm_protocol_builder_add_tlv failed");
        goto ERROR_EXIT;
    }
    /// 最终化构建器
    uint16_t out_size = 0;
    uint8_t *data = protocol_ctx->comm_protocol_builder_finalize(&builder, &out_size);
    if (data == NULL) {
        LOGE("==> comm_protocol_builder_finalize failed");
        goto ERROR_EXIT;
    }
    /// 发送数据
    ble_ctx->dispatch_ble_event_wrapper(ble_ctx, BLE_MSG_FAC_SEND_DATA, (uint8_t *)data, out_size);

ERROR_EXIT:
    protocol_ctx->comm_protocol_builder_free(&builder);
    return ret;
}
