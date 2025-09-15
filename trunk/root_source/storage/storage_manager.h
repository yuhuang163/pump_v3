#ifndef __STORAGE_MANAGER_H__
#define __STORAGE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#if 0
#define LOG_STORE_WRAP(format, args...) LOG_STORE("[tag=%s, store_id=%d]" format, tag, id, ##args)
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

    PERSIST_KEY_UTC_TIMESTEMP = 150,             /// 系统时间戳

    PERSIST_KEY_BREATH_PUMP_INFO = 180,

    PERSIST_KEY_BREATH_PUMP_HEAD_DATA = 200,          /// 刷牙数据头
    PERSIST_KEY_BREATH_PUMP_BODY_DATA_BEGIN,
    PERSIST_KEY_BREATH_PUMP_BODY_DATA_END = PERSIST_KEY_BREATH_PUMP_BODY_DATA_BEGIN + ROOT_BREATH_PUMP_BODY_DATA_NUM,
    PERSIST_KEY_USER_ID,
    PERSIST_KEY_MAX,
} persist_key_id_enum_t;



/// 描述用户绑定信息
typedef struct {
    uint32_t magic;
    user_bind_info_t bind_info;
    uint16_t crc;
} user_bind_persist_data_t;

/// 描述语音包信息
typedef struct {
    uint32_t magic;
    audio_packet_info_t audio_packet;
    uint16_t crc;
} audio_packet_persist_data_t;

/// 描述主题包信息
typedef struct {
    uint32_t magic;
    theme_packet_info_t theme_packet;
    uint16_t crc;
} theme_packet_persist_data_t;

/// 描述刷牙信息
typedef struct {
    uint32_t magic;
    breath_pump_info_t pump_info;
    uint16_t crc;
} breath_pump_info_persist_data_t;

/// 描述系统配置
typedef struct {
    uint32_t magic;
    sys_config_info_t config;
    uint16_t crc;
} sys_config_persist_data_t;

/// 描述系统状态
typedef struct {
    uint32_t magic;
    sys_state_info_t state;
    uint16_t crc;
} sys_state_persist_data_t;

/// 描述工厂校准数据
typedef struct {
    uint32_t magic;
    imu_cali_data_t imu_cali;
    uint16_t crc;
} imu_cali_persist_data_t;

/// 描述工厂标志信息
typedef struct {
    uint32_t magic;
    factory_flag_info_t flag_info;
    uint16_t crc;
} factory_flag_persist_data_t;

/// 描述工厂设备信息
typedef struct {
    uint32_t magic;
    factory_dev_info_t dev_info;
    uint16_t crc;
} factory_dev_info_persist_data_t;

/// 描述泵奶数据头
typedef struct {
    uint32_t magic;
    breath_pump_data_head_t pump_data_head;
    uint16_t crc;
} breath_pump_head_persist_data_t;

/// 描述泵奶数据体
typedef struct {
    uint32_t magic;
    breath_pump_data_body_t pump_data_body;
    uint16_t crc;
} breath_pump_body_persist_data_t;

/// 描述系统时间戳
typedef struct {
    uint32_t magic;
    uint32_t utc_timestamp;
    uint16_t crc;
} utc_timestamp_persist_data_t;

#ifdef __cplusplus
}
#endif

#endif 







