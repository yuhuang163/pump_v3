#ifndef __PROTOCOL_SERVICE_H__
#define __PROTOCOL_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "stdint.h" // IWYU pragma: keep
#include "comm_protocol_builder.h" // IWYU pragma: keep
#include "comm_protocol_defs.h" // IWYU pragma: keep
#include "servers_manager_api.h" // IWYU pragma: keep

/// 描述订阅服务回调函数.
typedef void (*comm_protocol_subscribe_cb_t)(int err_code, uint8_t *data, uint16_t len);
/// 描述订阅服务上下文.
typedef struct {
    const uint8_t  *buffer;      /// 帧总大小
    uint16_t size;         /// 帧大小
    comm_protocol_subscribe_cb_t cb; /// 订阅服务回调函数
} comm_protocol_subscribe_context_t;

/// 订阅服务处理函数.
typedef int (*comm_protocol_subscribe_func)(comm_protocol_builder_t *builder, comm_protocol_service_node_t *srv_node);
/// 描述订阅服务处理结构.
typedef struct {
    uint16_t service_id;                             /// 服务id   
    comm_protocol_subscribe_func subscribe_func;     /// 订阅服务功能函数.
} comm_protocol_subscribe_handle_t;

/// 发布服务处理函数.
typedef int (*comm_protocol_publish_func)(servers_publish_data_args_t *args);
/// 描述发布服务处理结构.
typedef struct {
    servers_publish_data_id_e id;                    /// 数据id   
    comm_protocol_publish_func publish_func;         /// 发布服务功能函数.
} comm_protocol_publish_handle_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
int comm_protocol_subscribe_handler(comm_protocol_subscribe_context_t *ctx);
comm_protocol_publish_func get_comm_protocol_publish_func(servers_publish_data_id_e id);
#ifdef __cplusplus
}
#endif

#endif // __PROTOCOL_SERVICE_H__

