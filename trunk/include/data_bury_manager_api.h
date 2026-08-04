#ifndef __DATA_BURY_MANAGER_API_H__
#define __DATA_BURY_MANAGER_API_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h"
#include "broadcast_manager_api.h"
#include "servers_manager_api.h"

/*============================================================================
 * 业务枚举（协议 7.x / v1.0.2，Flash 存 packed 数值）
 *============================================================================*/
typedef enum {
    DATA_BURY_DEVICE_SIDE_LEFT = 0,
    DATA_BURY_DEVICE_SIDE_RIGHT,
    DATA_BURY_DEVICE_SIDE_INDEPENDENCE,
} data_bury_device_side_e;

#ifndef DATA_BURY_DEVICE_SIDE_DEFAULT
#define DATA_BURY_DEVICE_SIDE_DEFAULT    DATA_BURY_DEVICE_SIDE_INDEPENDENCE
#endif

typedef enum {
    DATA_BURY_PUMP_SIDE_LEFT = 0,
    DATA_BURY_PUMP_SIDE_RIGHT,
} data_bury_pump_side_e;

typedef enum {
    DATA_BURY_SHUTDOWN_USER_MANUAL = 0,
    DATA_BURY_SHUTDOWN_LOW_BATTERY,
    DATA_BURY_SHUTDOWN_AUTO_SLEEP,
    DATA_BURY_SHUTDOWN_ABNORMAL,
    DATA_BURY_SHUTDOWN_FW_UPDATE,
    DATA_BURY_SHUTDOWN_OVERHEAT,
} data_bury_shutdown_reason_e;

typedef enum {
    DATA_BURY_ERROR_BATTERY_LOW = 0x01,
    DATA_BURY_ERROR_BATTERY_UNDERVOLTAGE_SHUTDOWN = 0x02,
    DATA_BURY_ERROR_CHARGE_OVERVOLTAGE = 0x03,
    DATA_BURY_ERROR_CHARGE_TIMEOUT = 0x04,
    DATA_BURY_ERROR_BATTERY_TEMP_OUT_OF_RANGE = 0x05,
    DATA_BURY_ERROR_MOTOR_LOCKED_OR_OVERCURRENT = 0x11,
    DATA_BURY_ERROR_MOTOR_OPEN_CIRCUIT = 0x12,
    DATA_BURY_ERROR_NEGATIVE_PRESSURE_SENSOR = 0x21,
    DATA_BURY_ERROR_NEGATIVE_PRESSURE_OVER_LIMIT = 0x22,
    DATA_BURY_ERROR_NEGATIVE_PRESSURE_BUILD_FAILED = 0x23,
    DATA_BURY_ERROR_AIR_LEAKAGE = 0x24,
    DATA_BURY_ERROR_SYSTEM_OVERHEAT = 0x31,
    DATA_BURY_ERROR_AMBIENT_TEMP_TOO_LOW = 0x32,
    DATA_BURY_ERROR_STORAGE_FAILURE = 0x41,
    DATA_BURY_ERROR_COMMUNICATION = 0x42,
    DATA_BURY_ERROR_KEY_STUCK = 0x51,
    DATA_BURY_ERROR_BOTTLE_NOT_DETECTED = 0x52,
    DATA_BURY_ERROR_FLANGE_NOT_DETECTED = 0x53,
    DATA_BURY_ERROR_FLANGE_ABNORMAL = 0x54,
    DATA_BURY_ERROR_FLANGE_SHORT = 0x55,
    DATA_BURY_ERROR_SYSTEM_ERROR_REBOOT = 0x61,
} data_bury_error_code_e;

typedef enum {
    DATA_BURY_PUMP_MODE_STIMULATION = 0x00,
    DATA_BURY_PUMP_MODE_EXPRESSION = 0x01,
    DATA_BURY_PUMP_MODE_MIX = 0x02,
    DATA_BURY_PUMP_MODE_RHYTHM = 0x03,
    DATA_BURY_PUMP_MODE_MILK_BOOST = 0x04,
    DATA_BURY_PUMP_MODE_MILK_TRACK = 0x05,
    DATA_BURY_PUMP_MODE_MAX,
} data_bury_pump_mode_e;

typedef enum {
    DATA_BURY_END_REASON_MANUAL = 0,
    DATA_BURY_END_REASON_AUTO_COMPLETE,
    DATA_BURY_END_REASON_TIMEOUT,
    DATA_BURY_END_REASON_LOW_BATTERY,
    DATA_BURY_END_REASON_ERROR,
    DATA_BURY_END_REASON_OVERHEAT,
} data_bury_end_reason_e;

typedef enum {
    DATA_BURY_EVT_SOURCE_LOCAL = 0,
    DATA_BURY_EVT_SOURCE_REMOTE_APP,
} data_bury_evt_source_e;

typedef enum {
    DATA_BURY_ADJUST_SUCTION_LEVEL = 1,
    DATA_BURY_ADJUST_PUMP_MODE,
    DATA_BURY_ADJUST_FREQUENCY,
    DATA_BURY_ADJUST_HEAT_LEVEL,
    DATA_BURY_ADJUST_VIBRATION_LEVEL,
} data_bury_adjust_type_e;

/** EVT_006 trigger 入参，经 DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST 的 args->val 传入 */
typedef struct {
    uint8_t evt_source;
    uint8_t evt_type;
    uint8_t adjust_from;           /* 非吸力调节：模式/频率等单字节前后值 */
    uint8_t adjust_to;
    uint16_t adjust_from_desktop;  /* 吸力调节：DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP */
    uint16_t adjust_to_desktop;
} data_bury_trigger_manual_adjust_t;

/** EVT_007 trigger 入参，经 DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT 的 args->val 传入 */
typedef struct {
    uint8_t evt_source;
    uint8_t evt_type;
} data_bury_trigger_user_evt_t;

typedef enum {
    DATA_BURY_USER_EVT_NIGHT_LIGHT = 0,
    DATA_BURY_USER_EVT_TRAVEL_LOCK,
    DATA_BURY_USER_EVT_RESET,
    DATA_BURY_USER_EVT_START,
    DATA_BURY_USER_EVT_PAUSE,
} data_bury_user_evt_type_e;

typedef enum {
    DATA_BURY_WEAR_DETECTION_TYPE_IR = 1,
    DATA_BURY_WEAR_DETECTION_TYPE_PRESSURE,
    DATA_BURY_WEAR_DETECTION_TYPE_MIXED,
} data_bury_wear_detection_type_e;

/** EVT_019 trigger 入参，经 DATA_BURY_TRIGGER_EVT_WEAR_DETECTION 的 args->val 传入 */
typedef struct {
    uint8_t detection_type;
    uint32_t detection_data;
} data_bury_trigger_wear_detection_t;

/** EVT_011 trigger 入参，经 DATA_BURY_TRIGGER_EVT_EXCEPTION 的 args->val 传入 */
typedef struct {
    uint16_t error_code;
    uint32_t error_data;
} data_bury_trigger_exception_t;

#define DATA_BURY_BAT_CHARGE_END_REASON_FULL              0
#define DATA_BURY_BAT_CHARGE_END_REASON_UNPLUG            1
#define DATA_BURY_BAT_CHARGE_END_REASON_ADAPTER_MISMATCH  2

#define DATA_BURY_FIRMWARE_UPDATE_RESULT_SUCCESS  0x00u
#define DATA_BURY_FIRMWARE_UPDATE_RESULT_FAILURE  0x01u

/** EVT_003 connect_result：1=建链失败(HCI)，0=建链成功后会话结束(断链上报) */
#define DATA_BURY_BT_CONNECT_RESULT_FAIL          0x01u
#define DATA_BURY_BT_CONNECT_RESULT_SUCCESS       0x00u

/** EVT_003 disconnect_reason（协议业务码） */
#define DATA_BURY_BT_DISCONNECT_REASON_USER       0x00u
#define DATA_BURY_BT_DISCONNECT_REASON_TIMEOUT    0x01u
/** 映射后跳过上报 */
#define DATA_BURY_BT_DISCONNECT_REASON_SKIP       0xFFu

/** 有效泵奶 session 最短时长(秒)，与 pump_m5pro 一致：仅超过此时长才分配 pump_session_id 并落盘 start/end */
#define DATA_BURY_PUMP_SESSION_MIN_DURATION_SEC   120u

/**
 * DATA_BURY_TRIGGER_EVT_BLE_CONNECTED：
 *   arg1=建连耗时 setup_ms（广播窗口起点至建连完成，由应用层传入，上限 65535ms）
 *   arg2=RSSI(int8，低 8 位有效；0/0xFF 时埋点层自动采样)
 * DATA_BURY_TRIGGER_EVT_BLE_DISCONNECT：arg1=HCI 断开原因，arg2=保留
 * DATA_BURY_TRIGGER_EVT_BAT_CHARGE_START：arg1/arg2 保留
 * DATA_BURY_TRIGGER_EVT_BAT_CHARGE_END：arg1=结束原因（DATA_BURY_BAT_CHARGE_END_REASON_*），arg2 保留
 * DATA_BURY_TRIGGER_EVT_PUMP_MODE_SWITCH：arg1=adjust_from，arg2=adjust_to，val=uint16_t* duration_sec(秒)
 * DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST：val=data_bury_trigger_manual_adjust_t*
 * DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT：val=data_bury_trigger_user_evt_t*
 * DATA_BURY_TRIGGER_EVT_WEAR_DETECTION：val=data_bury_trigger_wear_detection_t*
 * DATA_BURY_TRIGGER_EVT_EXCEPTION：val=data_bury_trigger_exception_t*
 */

typedef enum {
    DATA_BURY_TRIGGER_EVT_NONE = 0,
    DATA_BURY_TRIGGER_EVT_BLE_CONNECTED,
    DATA_BURY_TRIGGER_EVT_BLE_DISCONNECT,
    DATA_BURY_TRIGGER_EVT_PUMP_SESSION_START,
    DATA_BURY_TRIGGER_EVT_PUMP_MODE_SWITCH,
    DATA_BURY_TRIGGER_EVT_PUMP_MANUAL_ADJUST,
    DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT,
    DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END,
    DATA_BURY_TRIGGER_EVT_POWER_ON,
    DATA_BURY_TRIGGER_EVT_POWER_OFF,
    DATA_BURY_TRIGGER_EVT_OTA_START,
    DATA_BURY_TRIGGER_EVT_OTA_SUCCESS,
    DATA_BURY_TRIGGER_EVT_OTA_FAIL,
    DATA_BURY_TRIGGER_EVT_BAT_CHARGE_START,
    DATA_BURY_TRIGGER_EVT_BAT_CHARGE_END,
    DATA_BURY_TRIGGER_EVT_WEAR_DETECTION,
    DATA_BURY_TRIGGER_EVT_EXCEPTION,
} data_bury_trigger_event_e;

typedef struct {
    int arg1;
    int arg2;
    void *val;
} data_bury_trigger_args_t;

struct data_bury_manager_context;
typedef int (*data_bury_event_process_func)(struct data_bury_manager_context *ctx,
                                            const data_bury_trigger_args_t *args);

typedef struct {
    uint16_t event_id;
    data_bury_event_process_func event_process;
} data_bury_event_process_handle_t;

#define DATA_BURY_EVENT_TRIGGER_ARGS(_evt, _args) \
    do { \
        struct data_bury_manager_context *_data_bury_ctx_ = \
            (struct data_bury_manager_context *)find_module(ID_MODULE_DATA_BURIAL); \
        if (_data_bury_ctx_ && _data_bury_ctx_->data_bury_event_trigger) { \
            (void)_data_bury_ctx_->data_bury_event_trigger((_evt), (_args)); \
        } \
    } while (0)

#define DATA_BURY_EVENT_TRIGGER(_evt, _a1, _a2) \
    do { \
        data_bury_trigger_args_t _bury_trig_args_ = { \
            .arg1 = (_a1), \
            .arg2 = (_a2), \
            .val = NULL, \
        }; \
        DATA_BURY_EVENT_TRIGGER_ARGS((_evt), &_bury_trig_args_); \
    } while (0)

#define DATA_BURY_EVENT_TRIGGER_VAL(_evt, _a1, _a2, _val) \
    do { \
        data_bury_trigger_args_t _bury_trig_args_ = { \
            .arg1 = (_a1), \
            .arg2 = (_a2), \
            .val = (_val), \
        }; \
        DATA_BURY_EVENT_TRIGGER_ARGS((_evt), &_bury_trig_args_); \
    } while (0)

/** EVT_011 异常触发：error_code 为协议 uint16 有效码，error_data 无附加数据时填 0 */
#define DATA_BURY_EVENT_TRIGGER_EXCEPTION(_code, _data) \
    do { \
        data_bury_trigger_exception_t _bury_exc_ = { \
            .error_code = (uint16_t)(_code), \
            .error_data = (uint32_t)(_data), \
        }; \
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_EXCEPTION, 0, 0, &_bury_exc_); \
    } while (0)

/** 大主机双奶碗吸力档位：低字节左、高字节右（各 0~255，工程侧 0~15） */
#define DATA_BURY_PACK_SUCTION_LEVEL_DESKTOP(_left, _right) \
    ((uint16_t)(((uint8_t)(_left) & 0xFFu) | (((uint16_t)((uint8_t)(_right) & 0xFFu)) << 8)))

/** 佩戴检测数据打包：低 8 位左、次 8 位右（wear_status 0/1） */
#define DATA_BURY_PACK_WEAR_DETECTION_DATA(_left, _right) \
    ((uint32_t)(((uint32_t)(uint8_t)(_left)) | (((uint32_t)(uint8_t)(_right)) << 8)))

/** data_bury_publish(servers_publish_data_args_t)：arg1=payload 长度(0=按 id 推导)，arg2=延时 ms(0=立即)，val=payload；delay>0 时内部会拷贝 payload */
#define DATA_BURY_PUBLISH_ARGS(_len, _delay_ms, _val) \
    (servers_publish_data_args_t){ .arg1 = (_len), .arg2 = (_delay_ms), .val = (_val) }

#ifndef DATA_BURY_MSG_POOL_SIZE
#define DATA_BURY_MSG_POOL_SIZE                     32u
#endif

#ifndef DATA_BURY_MSG_WARN_NUM
#define DATA_BURY_MSG_WARN_NUM                      20u
#endif

#ifndef DATA_BURY_MSG_LOOP_PERIOD_MS
#define DATA_BURY_MSG_LOOP_PERIOD_MS                50u
#endif

typedef struct {
    servers_publish_data_id_e publish_id;
    int arg1;
    void *val;
    uint8_t val_in_psram;
} data_bury_msg_pool_payload_t;

typedef struct {
    uint8_t  is_use;
    uint16_t event;
    uint32_t target_ticks;
    union {
        data_bury_msg_pool_payload_t publish;
    } body;
} data_bury_msg_pool_entry_t;

typedef enum {
    DATA_BURY_EVT_NONE = 0,
    DATA_BURY_EVT_MSG_POOL_LOOP,
    DATA_BURY_EVT_FLUSH_PENDING,
    DATA_BURY_EVT_SESSION_END_COMMIT,
    DATA_BURY_EVT_PUBLISH,
} data_bury_event_type_e;

typedef struct {
    uint16_t event_type;
    servers_publish_data_id_e publish_id;
    int arg1;
    void *val;
    uint8_t val_in_psram;
} data_bury_entry_extra_t;

typedef struct data_bury_manager_context {
    struct __data_bury_private {
        thread_context_t bury_thd;
        os_mutex_id m_lock;
        os_timer_id bury_timer;
        struct broadcast_obj *bcd;
        uint8_t enabled;
        const data_bury_event_process_handle_t *event_process_list;
        uint16_t event_process_list_len;
        uint8_t msg_inited;
        uint8_t msg_cnt;
        uint8_t use_session_boot_inited;
        uint8_t pump_session_active;
        uint16_t use_session_boot_seq;
        uint16_t pump_session_counter;
        uint16_t current_pump_session_id;
        data_bury_msg_pool_entry_t msg_pool[DATA_BURY_MSG_POOL_SIZE];
        struct servers_manager_context *servers_ctx;
        struct storage_manager_context *storage_ctx;
        struct ble_manager_context *ble_ctx;
    } priv;

    uint8_t (*data_bury_is_enable)(void);
    uint8_t (*data_bury_set_enable)(uint8_t enable);
    int (*data_bury_publish)(servers_publish_data_id_e id, servers_publish_data_args_t *args);
    int (*data_bury_event_trigger)(data_bury_trigger_event_e event_id, const data_bury_trigger_args_t *args);
    int (*data_bury_flush_pending)(void);
    int (*data_bury_sys_msg_put)(uint16_t event, uint32_t delay_ms, data_bury_msg_pool_payload_t *payload);
    void (*data_bury_sys_msg_remove)(uint16_t event);
    uint32_t (*data_bury_get_use_session_id)(void);
    uint32_t (*data_bury_get_evt_timestamp)(void);
    uint8_t (*data_bury_get_device_side)(void);
    uint8_t (*data_bury_get_pump_side)(void);
    uint16_t (*data_bury_get_current_pump_session_id)(void);
    uint16_t (*data_bury_alloc_pump_session_id_on_commit)(void);
    void (*data_bury_reset_pump_session_state)(void);
    void (*data_bury_pump_session_begin)(void);
    void (*data_bury_pump_session_end)(void);
    void (*data_bury_fill_evt_common_hdr)(void *payload, uint16_t payload_len, servers_publish_data_id_e id);
    void (*data_bury_fill_pump_session_id)(void *payload, uint16_t payload_len, uint16_t pump_session_id);

    struct data_bury_manager_hook {
        int (*on_prepare_init_route)(struct data_bury_manager_context *ctx);
        int (*on_post_init_route)(struct data_bury_manager_context *ctx);
        int (*on_data_bury_br_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                              broadcast_priv_data_t *pdata);
        int (*data_bury_thread_route)(struct data_bury_manager_context *ctx, void *msg);
    } *hook;
} data_bury_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_MANAGER_API_H__ */
