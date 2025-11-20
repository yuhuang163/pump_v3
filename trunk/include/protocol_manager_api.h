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
#include "root_message.h"

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
        struct rt_device_node *fcuartdev;
        struct ble_manager_context *ble_ctx; /// 蓝牙处理管理器
    } priv;

    uint8_t (*protocol_parse)(uint8_t *data, uint16_t len, uint8_t proto_type);

    struct protocol_manager_hook {
        int (*on_perpare_init_route)(struct protocol_manager_context *ctx);
        int (*on_post_init_route)(struct protocol_manager_context *ctx);
        /// 广播路由
        int (*on_protocol_manager_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        uint8_t (*factory_tests_protocol_parse_route)(uint8_t *data, uint16_t len);
        uint8_t (*app_commication_protocol_parse_route)(uint8_t *data, uint16_t len);
        
    } *hook;
} protocol_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

