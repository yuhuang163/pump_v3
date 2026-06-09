#ifndef __DATA_BURY_MANAGER_API_H__
#define __DATA_BURY_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_base.h"
#include "broadcast_manager_api.h"
#include "servers_manager_api.h"

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
    int (*data_bury_flush_pending)(void);
    int (*data_bury_sys_msg_put)(uint16_t event, uint32_t delay_ms, data_bury_msg_pool_payload_t *payload);
    void (*data_bury_sys_msg_remove)(uint16_t event);
    uint32_t (*data_bury_get_use_session_id)(void);
    uint32_t (*data_bury_get_evt_timestamp)(void);
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
