/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "common_service.h" // IWYU pragma: keep
#include "common_service_base.h" // IWYU pragma: keep
#include "common_tlvs.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/// 按照service_id升序添加，否则查找不到（使用二分法查找）
const static comm_protocol_subscribe_handle_t comm_protocol_subscribe_list[] = {
    {COMM_PROTOCOL_ALGO_SERVICE, algo_subscribe_service_handle},
    {COMM_PROTOCOL_BREAST_PUMP_SERVICE, pump_subscribe_service_handle},
    {COMM_PROTOCOL_SYSTEM_CONFIG_SERVICE, sys_config_subscribe_service_handle},
    {COMM_PROTOCOL_SYSTEM_STATE_SERVICE, sys_state_subscribe_service_handle},
    {COMM_PROTOCOL_TESTS_SERVICE, tests_subscribe_service_handle},
    {COMM_PROTOCOL_OTA_SERVICE, ota_subscribe_service_handle}
};

/// 按照servers_publish_data_id_e升序添加，否则查找不到（使用二分法查找）
const static comm_protocol_publish_handle_t comm_protocol_publish_list[] = {
    {SERVERS_PUBLISH_DATA_ALGO_SAMPLING, algo_sampling_data_publish},
    {SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS, tests_key_status_publish},
    {SERVERS_PUBLISH_DATA_TESTS_ENCODER_STATUS, tests_encoder_status_publish},
    {SERVERS_PUBLISH_DATA_LIGHT_SENSOR_SAMPLING, algo_light_sensor_data_publish}
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static comm_protocol_subscribe_cb_t s_subscribe_assemble_cb = NULL;

static void on_assemble_complete_frame_cb(const uint8_t *frame_data, uint16_t frame_len)
{
    comm_protocol_subscribe_context_t ctx = {
        .buffer = frame_data,
        .size = frame_len,
        .cb = s_subscribe_assemble_cb,
    };
    comm_protocol_subscribe_handler(&ctx);
}

static comm_protocol_subscribe_func get_comm_protocol_subscribe_func(uint16_t service_id)
{
    const comm_protocol_subscribe_handle_t *list = comm_protocol_subscribe_list;
    uint16_t list_len = LENGTH_OF_ARRAY(comm_protocol_subscribe_list);
    
    /// 二分查找
    int left = 0;
    int right = list_len - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (list[mid].service_id == service_id) {
            return list[mid].subscribe_func;
        } else if (list[mid].service_id < service_id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    LOGE("==> get comm protocol service func failed: %d", service_id);
    return NULL;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int comm_protocol_subscribe_handler(comm_protocol_subscribe_context_t *ctx)
{
    uint8_t *rsp_buffer = NULL;
    uint16_t rsp_buffer_size = 0;
    int err_code = COMM_PROTOCOL_ERROR_NONE;
    EXCEPTION_CHECK_INT(ctx);
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    /// 解析协议帧
    comm_protocol_frame_node_t *frame = u_malloc(sizeof(comm_protocol_frame_node_t));
    EXCEPTION_CHECK_INT(frame);
    int ret = protocol_ctx->comm_protocol_frame_deserialize(ctx->buffer, ctx->size, frame);
    if (ret != 0){
        err_code = ret;
        LOGE("==> comm protocol subscribe deserialize error: %d", ret);
        goto ERROR_LABEL;
    }
    /// 解析Payload
    comm_protocol_service_node_t *service = NULL;
    if (frame->payload.service_count > COMM_PROTOCOL_MAX_SERVICE_NUM){
        err_code = COMM_PROTOCOL_ERROR_INVALID_SERVICE_COUNT;
        goto ERROR_LABEL;
    }
    /// 收到请求帧类型，需要组装响应帧
    uint8_t need_response = (frame->frame_type == COMM_PROTOCOL_FRAME_TYPE_REQUEST);
    comm_protocol_builder_t builder = {0};
    if (need_response) {
        /// 构建响应帧
        ret = protocol_ctx->comm_protocol_builder_init(&builder, COMM_PROTOCOL_MAX_PAYLOAD_SIZE, frame->seq, COMM_PROTOCOL_FRAME_TYPE_RESPONSE);
        if (ret != 0){
            err_code = COMM_PROTOCOL_ERROR_INVALID_BUILDER;
            goto ERROR_LABEL;
        }
    }
    LOG_SRV("==> comm protocol subscribe service count: %d", frame->payload.service_count);
    /// 查找服务处理接口
    for (uint8_t i = 0; i < frame->payload.service_count; i++){
        service = &frame->payload.services[i];
        LOG_SRV("==> comm protocol subscribe service: %d", service->svr_id);
        comm_protocol_subscribe_func func = get_comm_protocol_subscribe_func(service->svr_id);
        if (func == NULL){
            LOGE("==> get comm protocol subscribe func failed: %d", service->svr_id);
            continue;
        }
        LOG_SRV("==> comm protocol subscribe func: %p", func);
        ret = func(need_response ? &builder : NULL, service);
        if (ret != 0){
            LOGE("==> comm protocol subscribe func error: %d, service_id: %d", ret, service->svr_id);
            continue;
        }
    }
    /// 结束服务
    if (need_response) {
        LOG_SRV("==> comm protocol subscribe finalize");
        rsp_buffer = protocol_ctx->comm_protocol_builder_finalize(&builder, &rsp_buffer_size);
        if (ctx->cb && rsp_buffer) {
            ctx->cb(err_code, rsp_buffer, rsp_buffer_size); /// 回调函数执行应答操作
        }
        LOG_SRV("==> comm protocol subscribe builder free");
        protocol_ctx->comm_protocol_builder_free(&builder);
    }
    u_free(frame);
    return 0;

ERROR_LABEL:
    LOGE("==> comm protocol subscribe error: %d", err_code);
    if (ctx->cb){
        ctx->cb(err_code, NULL, 0);
    }
    u_free(frame);
    return -1;
}

int comm_protocol_subscribe_assemble_handler(comm_protocol_subscribe_context_t *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    EXCEPTION_CHECK_INT(protocol_ctx);
    s_subscribe_assemble_cb = ctx->cb;
    return protocol_ctx->comm_protocol_assemble_packet(ctx->buffer, ctx->size, on_assemble_complete_frame_cb);
}

comm_protocol_publish_func get_comm_protocol_publish_func(servers_publish_data_id_e id)
{
    const comm_protocol_publish_handle_t *list = comm_protocol_publish_list;
    uint16_t list_len = LENGTH_OF_ARRAY(comm_protocol_publish_list);
    
    /// 二分查找
    int left = 0;
    int right = list_len - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (list[mid].id == id) {
            return list[mid].publish_func;
        } else if (list[mid].id < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    LOGE("==> get comm protocol publish func failed: %d", id);
    return NULL;
}