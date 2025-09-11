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

#define BLE_MAC_ADDR_LEN    6
#define BLE_ADV_NAME_LEN    24

#define BLE_TASK_MSG_LEN1   128 /// 256
#define BLE_TASK_MSG_NUM    4

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
    BLE_MSG_ID_ADV_START_OK = 0,
    BLE_MSG_ID_ADV_STOP_OK,
    BLE_MSG_ID_SET_ADV_UPDATE,
    BLE_MSG_ID_SET_ADV_START,
    BLE_MSG_ID_SET_ADV_STOP,
    BLE_MSG_ID_USER_BIND_SUCC,
    BLE_MSG_ID_SUSPEND,
    BLE_MSG_ID_RESUME,
    BLE_MSG_ID_PERIPH_DISCONN,
    BLE_MSG_ID_PERIPH_CONNECT_SUC,
    BLE_MSG_ID_PERIPH_DISCONN_SUC,
    BLE_MSG_ID_MASTER_RECIVE_DATA,
    BLE_MSG_ID_ROTAS_RECV_CMD,
    BLE_MSG_ID_ROTAS_RESULT,
    BLE_MSG_ID_ROTAS_FILE_STATUE,
    BLE_MSG_ID_ROTAS_FLOW_CTRL,
    BLE_MSG_ID_ROTAS_PROGRESS,
    BLE_MSG_ID_ROTAS_FILE_CHECK,
    BLE_MSG_ID_SEND_BREATH_PUMP_REALTIME_INFO,
    BLE_MSG_ID_SEND_ALGO_REMIND_DATA,
    BLE_MSG_ID_SEND_DEVICE_REMIND_DATA,
    BLE_MSG_ID_AIRPLANE_MODE_CHANGE,
    BLE_MSG_ID_VOICE_STATE_CHANGE,
    BLE_MSG_ID_BREATH_PUMP_STATE_CHANGE,
    BLE_MSG_ID_BREATH_PUMP_MODE_CHANGE,

    BLE_MSG_ID_NOTIY_APP_SETTINGS,  /// 提醒手柄收到了APP下发任意设置

    BLE_MSG_ID_FAC_RECIVE_DATA,
    BLE_MSG_ID_FAC_UPLOAD_PRESS_SENSOR,
    BLE_MSG_ID_FAC_UPLOAD_NINE_ALEX,
    BLE_MSG_ID_FAC_UPLOAD_FLOAT_NINE_ALEX,
    BLE_MSG_ID_FAC_UPLOAD_BUTTON_STATE,
} ble_msg_id_e;

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

/// 描述ble服务接收发送接口结构.
typedef struct {
    ble_service_recv_t recv_state;
    uint16_t (*send_data)(uint8_t *p_data, uint16_t ulLen);
    uint16_t (*recv_data)(void *context, int srv_id, uint8_t *p_data, uint16_t ulLen);
    // uint16_t (*packaging_data)(void *context, uint8_t *p_data, uint16_t ulLen);
} ble_service_recv_send_if_t;

/// 描述ble操作抽象接口
typedef struct {
    int (*init)(void);
    // int (*opt_adv)(ble_opt_adv_id_e opt);
    // int (*ble_suspend)(void);
    // int (*ble_resume)(void);
    // int (*bt_disconnect)(uint16_t connect_id);
    // int (*bt_is_connect)(void);
    ble_service_recv_send_if_t recv_send_if[BLE_SRV_ID_MAX];
} ble_opt_interface_t;


typedef enum {
    DECODE_ACTION_REQ = 0,
    DECODE_ACTION_RSP = 1,
} decode_action_e;

typedef enum {
    DECODE_CHANNEL_NORMAL = 0,
    DECODE_CHANNEL_FACTORY = 1,
} decode_channel_e;

/// 描述decode数据接口封装.
typedef struct decode_msg_args {
    void *ctx;
    int cmd_id;
    int channel;    /// 0: normal, 1: factory.
    decode_action_e action;
    union {
        void *req;
        void *rsp;
        void *user_data;
    } data;
    int args0;
} decode_msg_args_t;

/// 依赖模块
struct rt_device_node;
struct broadcast_obj;

/// 描述ble管理上下文对象
typedef struct ble_manager_context {
/// private
    struct __airplane_private {
        thread_context_t ble_work_thd;
        struct broadcast_obj *bcreceiver;
        ble_opt_interface_t ble_opt_if;
        // ble_msg_mem_poll_t mem_pool;
        // sensor_collect_interface_t collect_if;  /// 算法采测接口
        struct rt_device_node *ble_dev;
        uint8_t mac_addr[BLE_MAC_ADDR_LEN];
        uint8_t adv_speed_flag;     /// 广播速度标记，快或慢 
        uint8_t bt_adv_state;       /// 广播是否开启 
        uint8_t bt_bind_process_state;  /// 绑定过程状态，@ref ble_bind_process_state_e 
        uint8_t bt_connect_state;
        uint16_t connect_idx;
    } priv;
/// public
    int (*get_mac_addr)(uint8_t *addr);
    int (*set_mac_addr)(uint8_t *addr);

    int (*is_bt_connected)(void);
    int (*disconnect_bt_by_user)(void);

    int (*get_bind_process_state)(void);
    int (*set_bind_process_state)(uint8_t state);

    int (*bt_adv_stop_by_user)(void);
    int (*bt_adv_start_by_user)(void);
    int (*get_adv_speed)(void);
    int (*set_adv_speed)(int flag);
    int (*get_adv_state)(void);
    int (*set_adv_state)(int state);

    int (*upload_breath_pump_overpress)(int state, int count, int side);
    int (*upload_voice_sw)(uint8_t state);
    int (*upload_breath_pump_life)(void);

    /// 蓝牙连接/断开状态改变回调.
    void (*on_connect_status_change)(struct ble_manager_context *thiz, int new_status, int popup);

    /// 钩子函数指针.
    struct __ble_manager_hook {
        int (*on_connect_change_route)(struct ble_manager_context *ctx, int new_status);
        int (*decode_msg_post_req)(const decode_msg_args_t *args);
        int (*packet_get_breath_pump_report_route)(void *context, void *report, void *breath_pump_data);
        int (*protocol_encode_process_route)(void *context, int cmd, void *data);
        int (*msg_decode_process)(void *context, uint8_t *p_data, uint16_t length);
    } *hook;
} ble_manager_context_t;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */

#ifdef __cplusplus
}
#endif

#endif
