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

typedef enum {
    SERVERS_PUBLISH_DATA_ALGO_SAMPLING = 100,        /// 算法采样数据
    SERVERS_PUBLISH_DATA_TESTS_KEY_STATUS = 101,     /// 产测按键状态上报
    SERVERS_PUBLISH_DATA_TESTS_ENCODER_STATUS = 102, /// 产测编码器状态上报
    SERVERS_PUBLISH_DATA_LIGHT_SENSOR_SAMPLING = 103, /// 产测光感采样数据上报

    SERVERS_PUBLISH_DATA_BLE_DISCONNECT_REPORT = 500,        /// 蓝牙断开上报
    SERVERS_PUBLISH_DATA_BLE_CONNECT_REPORT,           /// 蓝牙连接上报
    SERVERS_PUBLISH_DATA_AIRPLANE_MODE_EXIT_REPORT,    /// 飞行模式退出上报
    SERVERS_PUBLISH_DATA_AIRPLANE_MODE_ENTER_REPORT,   /// 飞行模式进入上报
    SERVERS_PUBLISH_DATA_BATTERY_LOW_POWER_SHUTDOWN_REPORT, /// 电池低电关机上报
    SERVERS_PUBLISH_DATA_BATTERY_CHARGE_ERROR_REPORT,  /// 电池充电错误上报
    SERVERS_PUBLISH_DATA_BATTERY_CHARGE_NORMAL_REPORT, /// 电池充电正常上报
    SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_RESET,     /// 泵系统状态复位上报
    SERVERS_PUBLISH_DATA_PUMP_SYSTEM_STATUS_CHANGE,    /// 泵系统状态变化上报
    SERVERS_PUBLISH_DATA_PUMP_MODE_CHANGE,             /// 泵模式变化上报
    SERVERS_PUBLISH_DATA_PUMP_CONTROL_TYPE_CHANGE,     /// 泵控制类型变化上报
    SERVERS_PUBLISH_DATA_PUMP_STEP_CHANGE,             /// 泵步骤变化上报
    SERVERS_PUBLISH_DATA_PUMP_LEVEL_CHANGE,            /// 泵等级变化上报
    SERVERS_PUBLISH_DATA_PUMP_SPEED_CHANGE,            /// 泵速率变化上报
    SERVERS_PUBLISH_DATA_PUMP_TIME_CHANGE,             /// 泵时间变化上报
    SERVERS_PUBLISH_DATA_PUMP_CUSTOM_MODE_REPORT,      /// 泵自定义模式上报
    SERVERS_PUBLISH_DATA_MOTOR_TASTE_START_REPORT,     /// 马达体验模式启动上报
    SERVERS_PUBLISH_DATA_MOTOR_TASTE_STOP_REPORT,      /// 马达体验模式停止上报
    SERVERS_PUBLISH_DATA_LIGHT_STATUS_CHANGE,           /// 灯光状态变化上报
    SERVERS_PUBLISH_DATA_BATTERY_LOW_POWER_RUN_REPORT, /// 电池仅低电低功耗运行上报
    SERVERS_PUBLISH_DATA_BATTERY_LOW_POWER_RUN_RELEASE, /// 电池仅低电低功耗运行解除上报

    /* Momcozy 数据埋点 Data Service EVT_001~EVT_019（svc=0x07） */
    SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_STARTUP = 600,       /// EVT_001 泵启动
    SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SHUTDOWN,            /// EVT_002 泵关机
    SERVERS_PUBLISH_DATA_DATA_BURY_BT_CONNECT_EVENT,         /// EVT_003 蓝牙连接
    SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_START,       /// EVT_004 泵奶会话开始
    SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_SESSION_END,         /// EVT_005 泵奶会话结束
    SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_MANUAL_ADJUST,       /// EVT_006 手动调节
    SERVERS_PUBLISH_DATA_DATA_BURY_PUMP_USER_EVENT,          /// EVT_007 用户操作
    SERVERS_PUBLISH_DATA_DATA_BURY_MILK_EJECTION,            /// EVT_008 喷乳反射
    SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_START,         /// EVT_009 充电开始
    SERVERS_PUBLISH_DATA_DATA_BURY_BAT_CHARGE_END,           /// EVT_010 充电结束
    SERVERS_PUBLISH_DATA_DATA_BURY_EXCEPTION,                /// EVT_011 异常
    SERVERS_PUBLISH_DATA_DATA_BURY_FIRMWARE_UPDATE,          /// EVT_012 固件升级
    SERVERS_PUBLISH_DATA_DATA_BURY_OVERFLOW,                   /// EVT_013 溢奶
    SERVERS_PUBLISH_DATA_DATA_BURY_MILK_VOLUME,              /// EVT_014 奶量采样
    SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_START,       /// EVT_015 加热会话开始
    SERVERS_PUBLISH_DATA_DATA_BURY_HEAT_SESSION_END,         /// EVT_016 加热会话结束
    SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_START,    /// EVT_017 按摩会话开始
    SERVERS_PUBLISH_DATA_DATA_BURY_MASSAGE_SESSION_END,      /// EVT_018 按摩会话结束
    SERVERS_PUBLISH_DATA_DATA_BURY_WEAR_DETECTION,           /// EVT_019 佩戴检测

    SERVERS_PUBLISH_DATA_ID_MAX
} servers_publish_data_id_e;

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
        thread_context_t servers_thd; /// 服务器线程
        struct protocol_manager_context *protocol_ctx; /// 协议管理器
        uint8_t encrypt_status;///payload 是否加密
    } priv;

    /// public:
    int (*dispatch_servers_event_wrapper)(struct servers_manager_context *ctx, servers_publish_data_id_e msg_id);
    struct __servers_manager_hook {
        int (*on_perpare_init_route)(struct servers_manager_context *ctx);
        int (*on_post_init_route)(struct servers_manager_context *ctx);
        int (*factory_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*ota_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*app_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
        int (*data_bury_server_data_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
         /// 广播路由
        int (*on_servers_manager_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        int (*servers_work_thread_route)(struct servers_manager_context *ctx, servers_publish_data_id_e msg_id);
    } *hook;
} servers_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 

