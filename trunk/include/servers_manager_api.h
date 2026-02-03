#ifndef __SERVERS_MANAGER_API_H__
#define __SERVERS_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h" // IWYU pragma: keep
#include "root_message.h" // IWYU pragma: keep
#include "app_old_protocol_parser.h"

typedef enum {
    SERVERS_PUBLISH_DATA_ALGO_SAMPLING = 100,        /// 算法采样数据
    SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS = 101,     /// 产测按键状态上报
    SERVERS_PUBLISH_DATA_TESTS_ENCODER_STATUS = 102, /// 产测编码器状态上报
    SERVERS_PUBLISH_DATA_LIGHT_SENSOR_SAMPLING = 103, /// 产测编码器状态上报
    SERVERS_PUBLISH_DATA_ID_MAX
} servers_publish_data_id_e;

typedef enum {
    OLD_PUBLISH_DATA_POLL_REPORT,                   /// 轮询上报
    OLD_PUBLISH_DATA_PUMP_LEVEL_CHANGE = 1,      /// 泵等级变化
    OLD_PUBLISH_DATA_PUMP_SPEED_CHANGE = 2,      /// 泵速变化
    OLD_PUBLISH_DATA_PUMP_MODE_CHANGE = 3,       /// 泵模式变化
    OLD_PUBLISH_DATA_PUMP_STEP_MODE_CHANGE = 4,  /// 步骤模式变化
    OLD_PUBLISH_DATA_PUMP_STEP_LEVEL_CHANGE = 5, /// 步骤等级变化
    OLD_PUBLISH_DATA_PUMP_STEP_SPEED_CHANGE = 6, /// 步骤速率变化
    OLD_PUBLISH_DATA_PUMP_STEP_ALL_PARAM_CHANGE = 7, /// 步骤所有参数变化
    OLD_PUBLISH_DATA_BATTARNG_CHANGE = 8,         /// 电池电量变化
    OLD_PUBLISH_DATA_SYSTEM_STATUS_CHANGE = 9,    /// 系统状态变化
    OLD_PUBLISH_DATA_TASTE_POLL_REPORT = 10,    /// 体验模式轮询上报
    OLD_PUBLISH_DATA_TASTE_START_REPORT = 11,    /// 体验模式启动上报
    OLD_PUBLISH_DATA_TASTE_STOP_REPORT = 12,    /// 体验模式停止上报
    OLD_PUBLISH_DATA_TASTE_EXIT_REPORT = 13,    /// 体验模式退出上报
    OLD_PUBLISH_DATA_TASTE_MODE_CHANGE = 14,    /// 体验模式模式变化
    OLD_PUBLISH_DATA_TASTE_LEVEL_CHANGE = 15,    /// 体验模式等级变化
    OLD_PUBLISH_DATA_TASTE_SPEED_CHANGE = 16,    /// 体验模式速率变化
    OLD_PUBLISH_DATA_PUMP_CONTROL_TYPE_CHANGE = 17, /// 泵控制类型变化
    OLD_PUBLISH_DATA_ID_MAX
} servers_old_protocol_publish_data_id_e;

typedef struct {
    int arg1;
    int arg2;
    void *val;
} servers_publish_data_args_t;

struct rt_device_node;
struct protocol_manager_context;
typedef struct servers_manager_context {
    /// private:
    struct __servers_manager_private {
        broadcast_obj_t *bcd; /// 广播监听对象
        struct protocol_manager_context *protocol_ctx; /// 协议管理器
        uint8_t encrypt_status;///payload 是否加密
    } priv;

    /// public:
    struct __servers_manager_hook {
        int (*on_perpare_init_route)(struct servers_manager_context *ctx);
        int (*on_post_init_route)(struct servers_manager_context *ctx);
        int (*factory_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*app_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*app_old_protocol_server_data_publish)(app_old_comm_cmd_id_e cid, servers_publish_data_args_t *args);
         /// 广播路由
        int (*on_servers_manager_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
    } *hook;
} servers_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

