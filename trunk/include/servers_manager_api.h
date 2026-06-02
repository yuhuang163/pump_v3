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
    SERVERS_PUBLISH_DATA_LIGHT_SENSOR_SAMPLING = 103, /// 产测光感采样数据上报
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
    OLD_PUBLISH_DATA_SYSTEM_STATUS_RESET = 18,    /// 强制上报系统状态复位
    OLD_PUBLISH_DATA_PUMP_STATUS_FINISH = 19,    /// 强制上报泵状态结束
    OLD_PUBLISH_DATA_ID_MAX
} servers_old_protocol_publish_data_id_e;

/** V3 设备异常类型（配合 APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL 首字节） */
typedef enum {
    OLD_PUBLISH_ABNORMAL_LOW_BAT_CANNOT_START = 0x01, ///< 电量不足无法启动
    OLD_PUBLISH_ABNORMAL_LOW_BATTERY = 0x02,           ///< 设备低电
    OLD_PUBLISH_ABNORMAL_LOW_BAT_SHUTDOWN_SOON = 0x03, ///< 低电即将关机
    OLD_PUBLISH_ABNORMAL_AIR_LEAK = 0x04,              ///< 设备漏气提醒
    OLD_PUBLISH_ABNORMAL_TRAVEL_LOCK = 0x05,           ///< 进入旅行锁
    OLD_PUBLISH_ABNORMAL_ADAPTER_ERROR = 0x06,         ///< 适配器异常
    OLD_PUBLISH_ABNORMAL_GUIDE_WINDOW = 0x07,          ///< 设备引导窗口不允许启动
} old_publish_abnormal_type_e;

/** V3 设备异常状态（配合 APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL 次字节） */
typedef enum {
    OLD_PUBLISH_ABNORMAL_STATE_CLEARED = 0x00, ///< 异常解除
    OLD_PUBLISH_ABNORMAL_STATE_ACTIVE = 0x01,  ///< 异常产生
} app_old_comm_v3_abnormal_state_e;

typedef struct {
    int arg1;
    int arg2;
    void *val;
} servers_publish_data_args_t;

typedef enum {
    SERVERS_BLE_DISCONNECT_REPORT = 100,
    SERVERS_BLE_CONNECT_REPORT,
    SERVERS_AIRPLANE_MODE_EXIT_REPORT,
    SERVERS_AIRPLANE_MODE_ENTER_REPORT,
    SERVERS_BATTERY_STATE_CHANGE,
    SERVERS_CHARGE_STATE_CHANGE,
    SERVERS_PUMP_MODE_CHANGE,
    SERVERS_PUMP_STEP_CHANGE,
    SERVERS_PUMP_LEVEL_CHANGE,
    SERVERS_PUMP_SPEED_CHANGE,
    SERVERS_PUMP_TIME_CHANGE,
    SERVERS_WORK_MSG_ID_MAX,
} servers_work_msg_id_e;

struct rt_device_node;
struct protocol_manager_context;
typedef struct servers_manager_context {
    /// private:
    struct __servers_manager_private {
        broadcast_obj_t *bcd; /// 广播监听对象
        thread_context_t servers_thd; /// 服务器线程
        struct protocol_manager_context *protocol_ctx; /// 协议管理器
        uint8_t encrypt_status;///payload 是否加密
    } priv;

    /// public:
    int (*dispatch_servers_event_wrapper)(struct servers_manager_context *ctx, int arg1, int arg2, void *user_data);
    struct __servers_manager_hook {
        int (*on_perpare_init_route)(struct servers_manager_context *ctx);
        int (*on_post_init_route)(struct servers_manager_context *ctx);
        int (*factory_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*app_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*app_old_protocol_server_data_publish)(app_old_comm_cmd_id_e cid, servers_publish_data_args_t *args);
         /// 广播路由
        int (*on_servers_manager_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        int (*servers_work_thread_route)(struct servers_manager_context *ctx, void *msg);
    } *hook;
} servers_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

