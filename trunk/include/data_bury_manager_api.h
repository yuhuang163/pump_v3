#ifndef __DATA_BURY_MANAGER_API_H__
#define __DATA_BURY_MANAGER_API_H__

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

/** 韵律模式（initial_mode_type=0x03 时附带 program TLV） */
#define DATA_BURY_PUMP_MODE_RHYTHM              0x03u

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

typedef enum {
    DATA_BURY_USER_EVT_NIGHT_LIGHT = 0,
    DATA_BURY_USER_EVT_TRAVEL_LOCK,
    DATA_BURY_USER_EVT_RESET,
    DATA_BURY_USER_EVT_START,
    DATA_BURY_USER_EVT_PAUSE,
} data_bury_user_evt_type_e;

#define DATA_BURY_BAT_CHARGE_END_REASON_FULL    0
#define DATA_BURY_BAT_CHARGE_END_REASON_UNPLUG  1

#define DATA_BURY_FIRMWARE_UPDATE_RESULT_SUCCESS  0x00u
#define DATA_BURY_FIRMWARE_UPDATE_RESULT_FAILURE  0x01u

typedef enum {
    DATA_BURY_TRIGGER_EVT_NONE = 0,
    DATA_BURY_TRIGGER_EVT_BLE_CONNECTED,
    DATA_BURY_TRIGGER_EVT_BLE_DISCONNECT,
    DATA_BURY_TRIGGER_EVT_PUMP_SESSION_START,
    DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END,
    DATA_BURY_TRIGGER_EVT_POWER_ON,
    DATA_BURY_TRIGGER_EVT_POWER_OFF,
    DATA_BURY_TRIGGER_EVT_OTA_START,
    DATA_BURY_TRIGGER_EVT_BAT_CHARGE_STATUS,
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
    uint16_t (*data_bury_alloc_pump_session_id)(void);
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
