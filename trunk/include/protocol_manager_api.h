#ifndef __PROTOCOL_MANAGER_API_H__
#define __PROTOCOL_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"
#include "root_message.h" // IWYU pragma: keep
#include "comm_protocol.h" // IWYU pragma: keep
#include "app_old_protocol_parser.h"

typedef enum {
    PROTO_TYPE_INVIAL = 0,
    PROTO_TYPE_FACTORY_TEST,
    PROTO_TYPE_APP_COMMINICATION,
    PROTO_TYPE_TYPE_MAX
} proto_type_id_e;

typedef struct protocol_manager_context {
/// private:
    struct __protocol_manager_private {
        broadcast_obj_t *bcd; /// 广播对象
#if IS_SUPPORT_DEVICE_UART
        struct rt_device_node *fcuartdev;
#endif
        struct ble_manager_context *ble_ctx; /// 蓝牙处理管理器
        uint16_t packet_seq; /// 包序列号
    } priv;

/// public:
    /// 获取包序列号
    uint16_t (*get_packet_seq)(void);
    /// 公共协议构建器接口.
    int (*comm_protocol_builder_init)(comm_protocol_builder_t *builder, uint16_t max_size, uint16_t seq, uint8_t frame_type);
    comm_protocol_service_t *(*comm_protocol_builder_init_service)(comm_protocol_builder_t *builder, uint16_t service_id);
    int (*comm_protocol_builder_add_tlv)(comm_protocol_builder_t *builder, comm_protocol_service_t *service, comm_protocol_tlv_node_t *tlv_node);
    uint8_t *(*comm_protocol_builder_finalize)(comm_protocol_builder_t *builder, uint16_t *out_size);
    void (*comm_protocol_builder_free)(comm_protocol_builder_t *builder);
    /// 组包回调函数
    int (*comm_protocol_assemble_packet)(const uint8_t *data, uint16_t len, comm_protocol_frame_callback_t callback);
    /// 反序列化协议帧
    int (*comm_protocol_frame_deserialize)(const uint8_t *buffer, uint16_t buffer_size, comm_protocol_frame_node_t *frame);
    ///old app组包回调函数
    int (*app_old_protocol_assemble_packet)(uint8_t *data, uint16_t len, app_old_protocol_frame_callback_t callback);
    /// old app反序列化协议帧
    int (*app_old_protocol_frame_deserialize)(uint8_t *frame_buf, uint16_t len, old_protocol_frame_node_t *frame);
    /// old app组帧发送
    int (*app_old_protocol_frame_builder_send)(app_old_commcmd_type_e cmdtype, app_old_comm_cmd_id_e cid, uint8_t data_len, uint8_t *data);
} protocol_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

