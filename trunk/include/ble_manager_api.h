#ifndef __BLE_MANAGER_API_H__
#define __BLE_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "thread_api.h" // IWYU pragma: keep

#define BLE_MAC_ADDR_LEN               6
#define BLE_ADV_NAME_LEN               24

#define BLE_TASK_MSG_LEN1              128 /// 256
#define BLE_TASK_MSG_NUM               4

typedef enum {
    BLE_BROADCAST_CONNECTED = 100, /// 连接成功
    BLE_BROADCAST_DISCONNECT, /// 断开连接 
    BLE_BROADCAST_NOTIFY_ENABLED, /// BLE NOTIFY 使能
    BLE_BROADCAST_NOTIFY_DISABLED, /// BLE NOTIFY 禁用
    BLE_BROADCAST_ID_MAX,
} ble_broadcast_id_e;

typedef enum {
    BLE_DL_STATE_START_SUC,   /// 启动成功
    BLE_DL_STATE_START_FAIL,  /// 启动失败
    BLE_DL_STATE_INPROGRESS,  /// 下载中
    BLE_DL_STATE_COMPLETE,    /// 下载完成
    BLE_DL_STATE_FAIL,        /// 下载失败
    BLE_DL_STATE_FLOW_CTRL,   /// 流控
} ble_downlaod_state_e;

typedef enum {
    BLE_OPT_ADV_STOP,           /// 关闭广播 
    BLE_OPT_ADV_SLOW,           /// 开启慢广播 
    BLE_OPT_ADV_FAST,           /// 开启快广播 
    BLE_OPT_ADV_PARAM_UPDATE,   /// 更新广播参数 
} ble_opt_adv_id_e;

typedef enum {
    BLE_BIND_STATE_INIT = 0,        /// 绑定过程初始化 
    BLE_BIND_STATE_KEY_GENERATE,    /// 绑定过程密钥生成 
    BLE_BIND_STATE_COMPLETE         /// 绑定过程完成 
} ble_bind_process_state_e;

typedef enum {
    BLE_SRV_DATA_TYPE_RAW = 0,    /// 数据透传 
    BLE_SRV_DATA_TYPE_ENCRYPT,    /// 数据加密 
    BLE_SRV_DATA_TYPE_NO_ENCRYPT  /// 数据不加密 
} ble_srv_data_type_e;

typedef struct {
    uint8_t *p_data;
    uint16_t size;
    uint16_t protocol_version; /// 协议版本
    ble_srv_data_type_e data_type; /// 数据类型
} ble_service_data_t;

/// 描述ble消息ID.
typedef enum {
    BLE_MSG_ADV_START_OK = 0, /// 广播启动成功
    BLE_MSG_ADV_STOP_OK, /// 广播停止成功
    BLE_MSG_SET_ADV_UPDATE, /// 设置广播更新
    BLE_MSG_SET_ADV_START, /// 设置广播启动
    BLE_MSG_SET_ADV_STOP, /// 设置广播停止
    BLE_MSG_USER_BIND_SUCC, /// 用户绑定成功
    BLE_MSG_SUSPEND, /// 休眠
    BLE_MSG_RESUME, /// 唤醒
    BLE_MSG_PERIPH_DISCONN, /// 设备断开连接
    BLE_MSG_PERIPH_CONNECT_SUC, /// 设备连接成功
    BLE_MSG_TX_CCC_ENABLED, /// 设备通知使能
    BLE_MSG_PERIPH_DISCONN_SUC, /// 设备断开连接成功
    BLE_MSG_MASTER_SEND_DATA, /// 发送主设备数据
    BLE_MSG_MASTER_RECIVE_DATA, /// 主设备接收数据
    BLE_MSG_FAC_SEND_DATA, /// 发送工厂设备数据
    BLE_MSG_FAC_RECIVE_DATA, /// 工厂设备接收数据
    BLE_MSG_REMOT_SEND_DATA, /// 发送远程设备数据
    BLE_MSG_REMOT_RECIVE_DATA, /// 远程设备接收数据
    BLE_MSG_DATA_BURY_SEND_DATA, /// 发送数据埋点服务数据
    BLE_MSG_DATA_BURY_RECIVE_DATA, /// 数据埋点服务接收数据
} ble_msg_id_e;

/// 描述ble消息处理函数.
typedef void (*ble_msg_process_func)(void *context, void *data, int data_len);

/// 描述ble消息处理结构.
typedef struct {
    uint8_t msg_id;                               /// 消息ID
    const ble_msg_process_func ble_msg_process;   /// 消息处理接口.
} ble_msg_process_handle_t;

/// 描述ble服务接收结构.
typedef struct {
    uint8_t *buf;
} ble_service_recv_t;

/// 描述ble事件连接结构.
typedef struct {
    int connect_state;        /// 0:未连接 1:已连接 2:鉴权完成
    int ble_popup_forbid;     /// 0:允许弹窗 1:禁止弹窗
    int16_t connect_id;
} ble_event_connect_t;

/// 描述ble发送数据结构.
typedef uint16_t (*ble_send_data)(uint8_t *p_data, uint16_t ulLen);
/// 描述ble事件回调函数
typedef void (*on_ble_evt_data_cb_fptr)(void *user_data, uint8_t *data, uint16_t data_len);
/// 描述ble服务接收发送接口结构.
typedef struct {
    ble_service_recv_t recv_state;
    uint16_t (*send_data)(uint8_t *p_data, uint16_t ulLen);
    uint16_t (*recv_data)(void *context, int srv_id, uint8_t *p_data, uint16_t ulLen);
    // uint16_t (*packaging_data)(void *context, uint8_t *p_data, uint16_t ulLen);
} ble_service_recv_send_if_t;

/// 依赖模块
struct rt_device_node;
struct broadcast_obj;

/// 描述ble管理上下文对象
typedef struct ble_manager_context {
/// private
    struct __ble_manager_private {
        thread_context_t ble_work_thd;
        struct broadcast_obj *bcreceiver;
        os_mutex_id m_lock;
        list_head_t client_list;
        struct rt_device_node *ble_dev;
        uint8_t mac_addr[BLE_MAC_ADDR_LEN];
        uint8_t adv_speed_flag;     /// 广播速度标记，快或慢 
        uint8_t ble_adv_state;       /// 广播是否开启 
        uint8_t ble_bind_state;  /// 绑定过程状态，@ref ble_bind_process_state_e 
        uint8_t ble_connect_state;
        uint8_t fac_enter_sleep;
        uint16_t connect_idx;
        /** 广播窗口起点 tick(get_timestamp)，建连时用于计算 setup_ms */
        uint32_t connect_timestamp_use;
        /** 各服务notify使能状态数组，索引为 ble_service_id_e */
        uint8_t notify_enabled[BLE_SRV_ID_MAX];
    } priv;
/// public
    int (*get_mac_addr)(uint8_t *addr);
    int (*set_mac_addr)(uint8_t *addr);
    int (*ble_is_connected)(void);
    int (*disconnect_bt_by_user)(void);
    int (*ble_bind_state_get)(void);
    int (*ble_bind_state_set)(uint8_t state);
    int (*bt_adv_stop_by_user)(void);
    int (*bt_adv_start_by_user)(void);
    int (*get_adv_speed)(void);
    int (*set_adv_speed)(int flag);
    int (*get_adv_state)(void);
    int (*set_adv_state)(int state);
    void (*add_ble_evt_lisenser)(void *user_data, int msg_channel, on_ble_evt_data_cb_fptr cb);
    void (*remove_ble_evt_lisenser)(on_ble_evt_data_cb_fptr cb);
    void (*dispatch_ble_client)(struct ble_manager_context *obj, int msg_channel, uint8_t *data, uint16_t size);
    void (*on_connect_status_change)(struct ble_manager_context *ctx, int new_status, int popup);
    int (*dispatch_ble_event_wrapper)(struct ble_manager_context *ctx, uint8_t _msg_id, uint8_t *_data, int _data_len);
    int (*notify_enabled_check)(uint8_t srv_id);
    uint8_t (*notify_enabled_get)(uint8_t srv_id);
    /// 钩子函数指针.
    struct __ble_manager_hook {
        int (*on_post_init_route)(struct ble_manager_context *ctx);
        int (*on_connect_change_route)(struct ble_manager_context *ctx, int new_status);
        int (*packet_get_breast_pump_report_route)(void *context, void *report, void *breast_pump_data);
        int (*protocol_encode_process_route)(void *context, int cmd, void *data);
        int (*decode_msg_post_req)(const void *args);
        int (*msg_decode_process)(void *context, uint8_t *p_data, uint16_t length);
    } *hook;
} ble_manager_context_t;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
int ble_data_bury_srv_recv_data(void *context, ble_service_data_t *srv_data);
int ble_data_bury_srv_send_data(void *context, ble_service_data_t *srv_data);

#ifdef __cplusplus
}
#endif

#endif
