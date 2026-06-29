#ifndef __STORAGE_MANAGER_H__
#define __STORAGE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/* Momcozy 埋点 Flash：8 槽 × 每槽 1 块 2048B（与 PSRAM 环对齐，须与 data_bury_persist_defs.h 一致） */
#ifndef PERSIST_DATA_BURY_FLASH_SLOT_NUM
#define PERSIST_DATA_BURY_FLASH_SLOT_NUM  DATA_BURY_FLASH_SLOT_NUM
#endif
#ifndef PERSIST_DATA_BURY_RING_CHUNK_PER_SLOT
#define PERSIST_DATA_BURY_RING_CHUNK_PER_SLOT  1u
#endif
#ifndef PERSIST_DATA_BURY_RING_CHUNK_NUM
#define PERSIST_DATA_BURY_RING_CHUNK_NUM  (PERSIST_DATA_BURY_FLASH_SLOT_NUM * PERSIST_DATA_BURY_RING_CHUNK_PER_SLOT)
#endif

#if 1
#define LOG_STORE_WRAP(...) LOG_STORE(__VA_ARGS__)
#else
#define LOG_STORE_WRAP(format, args...) do { } while (0)
#endif

typedef enum {
    PERSIST_KEY_OLD_BIND_INFO = 1,
    PERSIST_KEY_OLD_CONFIG_INFO,
    PERSIST_KEY_OLD_USER_DATA,

    PERSIST_KEY_BEGIN = 100,
    PERSIST_KEY_OTA_RECOVERY_TRY_COUNT,
    PERSIST_KEY_OTA_PRESSURE_TEST,
    PERSIST_KEY_ALGO_CALIB_PARAM,
    PERSIST_KEY_DEPOLY_FLAG,
    PERSIST_KEY_LOG_FILTER,
    PERSIST_KEY_POWER_ON_COMPLETE,

    PERSIST_KEY_FACTORY_DATA_HEAD = 120,
    PERSIST_KEY_FACTORY_FLAG,
    PERSIST_KEY_IMU_CALI_DATA,
    PERSIST_KEY_FACTORY_DEV_INFO,
    PERSIST_KEY_FACTORY_DID_CODE,
    PERSIST_KEY_FACTORY_SN_CODE,
    PERSIST_KEY_FACTORY_BODY_COLOR,
    PERSIST_KEY_SYS_INFO_DATA_HEAD = 140,
    PERSIST_KEY_SYS_CONFIG_DATA,
    PERSIST_KEY_SYS_STATE_DATA,
    PERSIST_KEY_SYS_BIND_INFO,
    PERSIST_KEY_AUDIO_PACKET_DATA,
    PERSIST_KEY_THEME_PACKET_DATA,
    PERSIST_KEY_BATTERY_HEALTH_COEFF,
    PERSIST_KEY_IMG_RES_OTA_INFO,

    PERSIST_KEY_UTC_TIMESTEMP = 150,             /// 系统时间戳

    PERSIST_KEY_BREAST_PUMP_INFO = 180,
    PERSIST_KEY_BREAST_PUMP_MODE_STEPS_INFO,
    PERSIST_KEY_BREAST_PUMP_BURY_INFO,

    PERSIST_KEY_BREAST_PUMP_DATA_HEAD = 200,          /// 泵奶数据头
    PERSIST_KEY_BREAST_PUMP_DATA_BODY_BEGIN,
    PERSIST_KEY_BREAST_PUMP_DATA_BODY_END = PERSIST_KEY_BREAST_PUMP_DATA_BODY_BEGIN + ROOT_BREAST_PUMP_DATA_BODY_NUM,
    PERSIST_KEY_USER_ID,
    PERSIST_KEY_DATA_BURY_FLASH_BANK,
    PERSIST_KEY_DATA_BURY_USE_SESSION_BOOT_SEQ,
    PERSIST_KEY_DATA_BURY_RING_META_SLOT_BEGIN,
    PERSIST_KEY_DATA_BURY_RING_META_SLOT_END = PERSIST_KEY_DATA_BURY_RING_META_SLOT_BEGIN + PERSIST_DATA_BURY_FLASH_SLOT_NUM,
    PERSIST_KEY_DATA_BURY_RING_CHUNK_BEGIN,
    PERSIST_KEY_DATA_BURY_RING_CHUNK_END = PERSIST_KEY_DATA_BURY_RING_CHUNK_BEGIN + PERSIST_DATA_BURY_RING_CHUNK_NUM,
    PERSIST_KEY_MAX,
} persist_key_id_enum_t;



/// 描述用户绑定信息
typedef struct {
    uint32_t magic;
    user_bind_info_t bind_info;
    uint16_t crc;
} user_bind_storage_t;

/// 描述泵奶信息
typedef struct {
    uint32_t magic;
    sys_pump_info_t pump_info;
    uint16_t crc;
} breast_pump_info_storage_t;

/// 描述泵奶模式步骤信息
typedef struct {
    uint32_t magic;
    sys_pump_mode_info_t mode_info;
    uint16_t crc;
} breast_pump_mode_steps_info_storage_t;

/// 描述泵奶埋点信息
typedef struct {
    uint32_t magic;
    sys_buried_info_list_t buried_info_list;
    uint16_t crc;
} breast_pump_buried_info_storage_t;

/// 描述系统配置
typedef struct {
    uint32_t magic;
    sys_config_info_t config;
    uint16_t crc;
} sys_config_storage_t;

/// 描述系统状态
typedef struct {
    uint32_t magic;
    sys_state_info_t state;
    uint16_t crc;
} sys_state_storage_t;

/// 描述工厂校准数据
typedef struct {
    uint32_t magic;
    imu_cali_data_t imu_cali;
    uint16_t crc;
} imu_cali_storage_t;

/// 描述工厂标志信息
typedef struct {
    uint32_t magic;
    factory_flag_info_t flag_info;
    uint16_t crc;
} factory_flag_storage_t;

/// 描述工厂设备信息
typedef struct {
    uint32_t magic;
    factory_dev_info_t dev_info;
    uint16_t crc;
} __attribute__((packed)) factory_dev_info_storage_t;

/// 描述泵奶数据头
typedef struct {
    uint32_t magic;
    breast_pump_data_head_t pump_data_head;
    uint16_t crc;
} breast_pump_data_head_storage_t;

/// 描述泵奶数据体
typedef struct {
    uint32_t magic;
    pump_result_info_t pump_data_body;
    uint16_t crc;
} breast_pump_body_storage_t;

/// 描述系统时间戳
typedef struct {
    uint32_t magic;
    uint32_t utc_timestamp;
    uint16_t crc;
} utc_timestamp_storage_t;

/// 描述图片资源OTA状态信息
typedef struct {
    uint32_t magic;
    img_res_ota_info_t res_ota_info;
    uint16_t crc;
} img_res_ota_storage_t;

#ifdef __cplusplus
}
#endif

#endif 







