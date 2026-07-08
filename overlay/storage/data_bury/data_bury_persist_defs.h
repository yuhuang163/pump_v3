#ifndef __DATA_BURY_PERSIST_DEFS_H__
#define __DATA_BURY_PERSIST_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_data_bury_protocol_defs.h"

/* Momcozy 数据埋点：PSRAM 环临时缓冲 + Flash 关机快照（8 槽 × 2048B ≈ 16KB） */
#define DATA_BURY_RING_DATA_SIZE           2048u
/* Flash 槽数据块与 PSRAM 环等大，每槽 1 块（不再拆 512B 分片） */
#define DATA_BURY_RING_CHUNK_SIZE          DATA_BURY_RING_DATA_SIZE
#define DATA_BURY_RING_CHUNK_NUM           1u
#define DATA_BURY_FLASH_SLOT_NUM           16u

/* 单条环记录 payload 上限（v1.0.3 EVT_005: 25 + 58*6 = 373） */
#define DATA_BURY_EVENT_PAYLOAD_MAX        374u
#define DATA_BURY_PUMP_SESSION_MAX_MODES    58u

/*
 * Flash/环内 packed 事件结构布局版本。
 * 修改 data_bury_evt_*_t 字段或变长布局时递增，开机与 Flash 中版本不一致则清空埋点队列。
 */
#define DATA_BURY_STORAGE_VERSION           1u

#define DATA_BURY_SVC_DATA                 APP_DATA_BURY_PROTOCOL_SVC_DATA
/*============================================================================
 * Data Service 事件载荷（协议 7.1~7.19 / v1.0.2，packed 小端写入 Flash 环）
 * device_side 由应用层填写
 *============================================================================*/

/* 协议公共头：device_side + use_session_id + evt_timestamp */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
} data_bury_evt_common_hdr_t;

#define DATA_BURY_EVT_COMMON_HDR_LEN            ((uint16_t)sizeof(data_bury_evt_common_hdr_t))
#define DATA_BURY_EVT_PUMP_SESSION_ID_OFF       9u

#define DATA_BURY_EVT_USE_SESSION_PLACEHOLDER   0xFFFFFFFFu
#define DATA_BURY_EVT_TIMESTAMP_PLACEHOLDER     0xFFFFFFFFu
#define DATA_BURY_EVT_PUMP_SESSION_ID_PLACEHOLDER 0xFFFFu

/* CMD:0x01 EVT_001 pump_startup */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t firmware_version;
    uint8_t  battery_level_startup;
} data_bury_evt_pump_startup_t;

/* CMD:0x02 EVT_002 pump_shutdown */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint8_t  battery_level_shutdown;
    uint8_t  shutdown_reason;
} data_bury_evt_pump_shutdown_t;

/* CMD:0x03 EVT_003 bt_connect */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint8_t  connect_result;
    uint16_t connection_setup_duration;
    int8_t   bt_signal_strength_rssi;
    uint32_t disconnect_timestamp;
    uint8_t  disconnect_reason;
} data_bury_evt_bt_connect_t;

/* CMD:0x04 EVT_004 pump_session_start */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint8_t  initial_mode_type;
    uint8_t  initial_suction_level;  /* 本工程不用，固定填 0；挡位见 initial_suction_level_desktop */
    uint32_t initial_attitude_pitch_roll;
    uint16_t initial_volume_ml;
    uint8_t  initial_battery_level;
    uint16_t initial_suction_level_desktop; /* v1.0.3 TLV 0x09：低字节左、高字节右 */
} data_bury_evt_pump_session_start_t;

/* CMD:0x05 EVT_005 pump_session_end（变长: program_duration[used_mode_count]，TLV 0x0D） */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint8_t  final_mode_type;
    uint8_t  final_suction_level;  /* 本工程不用，固定填 0；挡位见 final_suction_level_desktop(TLV 0x0E) */
    uint32_t final_attitude_pitch_roll;
    uint16_t final_volume_ml;
    uint8_t  final_battery_level;
    uint8_t  end_reason;
    uint8_t  milk_drop_count;
    uint8_t  used_mode_count;
    uint16_t final_suction_level_desktop; /* v1.0.3 TLV 0x0E：低字节左、高字节右 */
} data_bury_evt_pump_session_end_fixed_t;

typedef struct __attribute__((packed)) {
    uint32_t mode_or_program_id;
    uint16_t duration_sec;
} data_bury_evt_program_duration_t;

#define DATA_BURY_EVT_PUMP_SESSION_END_FIXED_LEN \
    ((uint16_t)sizeof(data_bury_evt_pump_session_end_fixed_t))
#define DATA_BURY_EVT_PUMP_SESSION_END_PROGRAM_OFF \
    DATA_BURY_EVT_PUMP_SESSION_END_FIXED_LEN
#define DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(_used) \
    ((uint16_t)(DATA_BURY_EVT_PUMP_SESSION_END_PROGRAM_OFF + ((uint16_t)(_used) * 6u)))
#define DATA_BURY_EVT_PUMP_SESSION_END_MAX_LEN \
    DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(DATA_BURY_PUMP_SESSION_MAX_MODES)

/* CMD:0x06 EVT_006 pump_manual_adjust */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint8_t  evt_source;
    uint8_t  evt_type;
    uint8_t  adjust_from;          /* 本工程吸力调节不用，固定 0；见 adjust_from_desktop */
    uint8_t  adjust_to;
    uint16_t adjust_from_desktop; /* v1.0.3 TLV 0x09：低字节左、高字节右 */
    uint16_t adjust_to_desktop;   /* v1.0.3 TLV 0x0A：低字节左、高字节右 */
} data_bury_evt_pump_manual_adjust_t;

/* CMD:0x07 EVT_007 pump_user_evt */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint8_t  evt_source;
    uint8_t  evt_type;
} data_bury_evt_pump_user_evt_t;

/* CMD:0x08 EVT_008 milk_ejection */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint16_t detect_times_no;
    uint16_t start_mls;
    uint32_t start_timestamp;
    uint16_t end_mls;
    uint32_t end_timestamp;
} data_bury_evt_milk_ejection_t;

/* CMD:0x09 EVT_009 bat_charge_start */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint8_t  start_battery_percent;
    uint16_t start_battery_voltage;
    int16_t  start_battery_current;
    int8_t   start_battery_temp;
} data_bury_evt_bat_charge_start_t;

/* CMD:0x0A EVT_010 bat_charge_end */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint8_t  end_battery_percent;
    uint16_t end_battery_voltage;
    int16_t  end_battery_current;
    int8_t   end_battery_temp;
    int8_t   end_reason;
} data_bury_evt_bat_charge_end_t;

/* CMD:0x0B EVT_011 exception */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t error_code;
    uint32_t error_data;
} data_bury_evt_exception_t;

/* CMD:0x0C EVT_012 firmware_update */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t update_from;
    uint8_t  update_result;
    uint16_t update_err;
} data_bury_evt_firmware_update_t;

/* CMD:0x0D EVT_013 overflow */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint16_t milk_volume_ml;
    uint32_t real_time_attitude_pitch_roll;
} data_bury_evt_overflow_t;

/* CMD:0x0E EVT_014 milk_volume（变长: volume_ml[sampling_count]） */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t pump_session_id;
    uint16_t sampling_count;
} data_bury_evt_milk_volume_fixed_t;

typedef struct __attribute__((packed)) {
    data_bury_evt_milk_volume_fixed_t fixed;
    uint16_t volume_ml[];
} data_bury_evt_milk_volume_t;

#define DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN \
    ((uint16_t)sizeof(data_bury_evt_milk_volume_fixed_t))
#define DATA_BURY_EVT_MILK_VOLUME_MAX_LEN \
    (DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN + (DATA_BURY_PUMP_SESSION_MAX_MODES * 2u))

/* CMD:0x0F EVT_015 heat_session_start */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t heat_session_id;
    uint8_t  target_level;
    uint8_t  initial_temp;
} data_bury_evt_heat_session_start_t;

/* CMD:0x10 EVT_016 heat_session_end */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t heat_session_id;
    uint8_t  top_temp;
    uint8_t  final_temp;
} data_bury_evt_heat_session_end_t;

/* CMD:0x11 EVT_017 massage_session_start */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t massage_session_id;
    uint8_t  initial_level;
} data_bury_evt_massage_session_start_t;

/* CMD:0x12 EVT_018 massage_session_end */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint16_t massage_session_id;
    uint8_t  final_level;
} data_bury_evt_massage_session_end_t;

/* CMD:0x13 EVT_019 wear_detection */
typedef struct __attribute__((packed)) {
    uint8_t  device_side;
    uint32_t use_session_id;
    uint32_t evt_timestamp;
    uint8_t  detection_type;
    uint32_t detection_data;
} data_bury_evt_wear_detection_t;

/*============================================================================
 * 环记录 / Flash 分片存储
 *============================================================================*/
typedef struct __attribute__((packed)) {
    uint8_t  svc_id;
    uint8_t  cmd_id;
    uint16_t payload_len;
} data_bury_rec_hdr_t;

typedef struct {
    uint16_t head;
    uint16_t tail;
} data_bury_ring_meta_t;

/** put_store_ram_val(STORE_ID_DATA_BURY_RING_APPEND): 追加一条埋点事件 */
typedef struct {
    uint8_t       svc_id;
    uint8_t       cmd_id;
    uint16_t      payload_len;
    const void   *payload;
} data_bury_ring_append_args_t;

/** get_store_ram_val(STORE_ID_DATA_BURY_RING_PEEK_OLDEST): 读最旧一条，不 pop */
typedef struct {
    data_bury_rec_hdr_t *hdr;
    uint8_t             *payload;
    uint16_t             payload_buf_len;
} data_bury_ring_peek_args_t;

typedef struct {
    uint8_t count;      /* 0~DATA_BURY_FLASH_SLOT_NUM 有效快照数 */
    uint8_t read_slot;  /* 最旧待上报快照的物理槽号 */
    uint8_t write_slot; /* 下次关机写入的物理槽号 */
    uint8_t reserved;
} data_bury_flash_bank_t;

typedef struct {
    data_bury_ring_meta_t meta;
    uint16_t event_count;
} data_bury_slot_meta_t;

/** 埋点控制信息：bank / boot_seq / 版本 / 各槽 meta，开机一次读入 RAM，按需整包写回 */
typedef struct {
    data_bury_flash_bank_t bank;
    uint16_t boot_seq;
    uint16_t storage_version;
    data_bury_slot_meta_t slots[DATA_BURY_FLASH_SLOT_NUM];
} data_bury_ctrl_info_t;

typedef struct {
    uint32_t magic;
    data_bury_ctrl_info_t info;
    uint16_t crc;
} data_bury_ctrl_storage_t;

typedef struct {
    uint32_t magic;
    uint8_t  chunk[DATA_BURY_RING_CHUNK_SIZE];
    uint16_t crc;
} data_bury_ring_chunk_storage_t;

typedef struct {
    uint16_t boot_seq;
} data_bury_info_t;

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_PERSIST_DEFS_H__ */
