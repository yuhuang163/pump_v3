#ifndef __PERSIST_STORAGE_H__
#define __PERSIST_STORAGE_H__

#include "stdint.h"
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_algo.h" // IWYU pragma: keep
#include "motor_policy.h" // IWYU pragma: keep

#define ROOT_WRITE_INFO_LEN                         64
#define ROOT_BREAST_PUMP_TIME_VALID                 30000   /// 30s
#define ROOT_BREAST_PUMP_DATA_BODY_NUM              70      /// 按天存储，每天最多 ROOT_BREAST_PUMP_DATA_BODY_NUM / 7
#define USER_BIND_UID_LEN                           8
#define USER_BIND_RANDOM_LEN                        6
#define USER_BIND_AES_KEY_LEN                       16

#define DEVICE_UNIQUE_DID_LEN                       10
#define DEVICE_PRODUCT_ID_LEN                       6
#define DEVICE_DEVICE_ID_LEN                        16
#define DEVICE_ENCTYPTION_KEY_LEN                   16
#define DEVICE_UNIQUE_CODE_LEN                      16
#define DEVICE_UNIQUE_SN_LEN                        40
#define DEVICE_FW_VERSION_LEN                       18

#define ROOT_BREAST_PUMP_STEP_MAX                   32
#define FACTORY_AGE_INFO_MAX                        3   /// 老化信息最大保存3条
#define ROOT_BREAST_PUMP_BURY_DATA_NUM              10    /// 埋点数据最大保存10条

#define LIGHT_SENSOR_CALIB_POINT_NUM                20

typedef enum  { 
    SYS_DAY_MONDAY = 0,
    SYS_DAY_TUESDAY = 1,
    SYS_DAY_WEDNESDAY = 2,
    SYS_DAY_THURSDAY = 3,
    SYS_DAY_FRIDAY = 4,
    SYS_DAY_SATURDAY = 5,
    SYS_DAY_SUNDAY = 6,
    SYS_DAY_MAX,
} sys_day_type_e;

typedef enum  { 
    SYS_MODE_STATE_NORMAL = 0, /// 正常模式
    SYS_MODE_STATE_NON_DELETABLE = 1, /// 不可删除模式
    SYS_MODE_STATE_NON_MOVE = 2, /// 不可移动模式
    SYS_MODE_STATE_NON_DELETABLE_MOVE = 3, /// 不可删除不可移动模式
    SYS_MODE_STATE_NON_DISPLAY = 4, /// 不可显示模式
    SYS_MODE_STATE_MAX,
} sys_mode_state_e;

/// 工厂部署标志位
typedef struct {
    uint8_t age_flag;          /// 老化标志//0-未老化 1-老化成功 2-老化失败
    uint32_t age_cycle;        /// 老化循环次数
    uint32_t age_time;         /// 老化时长，单位：秒
} factory_age_info_t;

/// 工厂部署标志位
typedef struct {
    uint8_t deploy_flag;       /// 工厂模式部署标志
    factory_age_info_t age_info[FACTORY_AGE_INFO_MAX];/// 老化信息
    uint8_t age_display;       /// 老化展示标志//0-不展示 1-展示
    uint8_t fsensor_cali_flag; /// fsensor校准标志//0-未校准 1-校准
} factory_flag_info_t;

/// 工厂烧录泵电机类型
#define FACTORY_MOTOR_TYPE_DEFAULT    0  /// 未烧录
#define FACTORY_MOTOR_TYPE_LONGYI    1  /// 龙一泵
#define FACTORY_MOTOR_TYPE_SHIGUANG  2  /// 时光泵

/// 工厂烧录电池厂商类型
#define FACTORY_BATTERY_TYPE_DEFAULT  0  /// 未烧录
#define FACTORY_BATTERY_TYPE_VENDOR1  1  /// 电池厂商1
#define FACTORY_BATTERY_TYPE_VENDOR2  2  /// 电池厂商2

typedef struct {
    uint8_t product_id[DEVICE_PRODUCT_ID_LEN];
    uint8_t device_id[DEVICE_DEVICE_ID_LEN];
    uint8_t enctyption_key[DEVICE_ENCTYPTION_KEY_LEN];
    uint8_t sn[DEVICE_UNIQUE_SN_LEN];
    uint8_t product_id_len;    // ProductId length
    uint8_t device_id_len;     // DeviceId length
    uint8_t enctyption_key_len;// enctyption_key length
    uint8_t sn_len;            // sn length
    uint8_t body_color;
    uint8_t region_id;   /// 区域ID, 中版，俄版，欧版，美版...
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    int32_t light_sensor_calib_tbl[LIGHT_SENSOR_CALIB_POINT_NUM]; /// 光感校准数据
    uint8_t motor_type;//设备泵电机类型
    uint8_t battery_type; //电池类型
    uint8_t rsv[30]; /// 保留
#endif
} __attribute__((packed)) factory_dev_info_t;

/// 工厂老化标志
typedef struct {
    uint8_t complete;
    uint8_t display; 
} factory_aging_flag_t;   

typedef struct {
    uint8_t policy_type; /// 策略类型 motor_policy_type_e
    uint8_t mode_type;   /// 吸力模式类型 suction_mode_type_e
    uint8_t level;       /// 吸力等级 suction_level_e
    uint8_t freq;        /// 吸力速度 suction_speed_e
    uint8_t loop_num;    /// 循环次数
    uint8_t rsv;         /// 保留
    uint32_t duration;   /// 持续时间，单位s
} pump_mode_step_param_t;

typedef struct {
    union {
        uint32_t dat32;
        uint8_t dat[4];
    } id;            /// 参数ID
    uint8_t policy_num;     /// 策略总数量(包含子策略和定时策略)
    uint8_t time_policy_num;  /// 时间策略数量
    uint32_t total_time; /// 总时长,单位s
	pump_mode_step_param_t data[ROOT_BREAST_PUMP_STEP_MAX];
} pump_mode_steps_info_t;

/// 泵奶模式
typedef enum {   
#if (PUMP_RHYTHM_MODE_NUM == 3)
    BREAST_PUMP_STIMULATION_MODE,             /// 刺激模式
    BREAST_PUMP_LACTATION_MODE,               /// 吸乳模式
    BREAST_PUMP_MIX_MODE,                     /// 混合模式
    BREAST_PUMP_RELAX_MODE,                   /// 放松模式（Relax）
    BREAST_PUMP_MILK_BOOST_MODE,              /// 2+8奶量提升模式
    BREAST_PUMP_PP_MILK_TRACK_MODE,           /// PP追奶模式（Power Pumping模式）
    BREAST_PUMP_CUSTOM_MODE,                  /// 个性化模式
    BREAST_PUMP_CUSTOM_TRIAL_MODE,            /// 用户自定义模式(试用)
    BREAST_PUMP_REGULAR_PUMPING_MODE,         /// 常规韵律模式（Regular Pumping）
    BREAST_PUMP_LACTATION_START_MODE,         /// 开奶模式
    BREAST_PUMP_EMPTYING_BOOST_MODE,          /// 追加排空模式（Emptying Boost）
    BREAST_PUMP_MODE_MAX,                     /// 模式最大模式
    BREAST_PUMP_NORMAL_MODE_LOWER = BREAST_PUMP_STIMULATION_MODE, /// 常规模式下限模式
    #if (PUMP_MODE_NUM == 3)
    BREAST_PUMP_NORMAL_MODE_UPPER = BREAST_PUMP_MIX_MODE, /// 常规模式上限模式
    #else
    BREAST_PUMP_NORMAL_MODE_UPPER = BREAST_PUMP_LACTATION_MODE, /// 常规模式上限模式
    #endif
    BREAST_PUMP_EXPERT_MODE_LOWER = BREAST_PUMP_MILK_BOOST_MODE, /// 专家模式下限模式
    BREAST_PUMP_EXPERT_MODE_UPPER = BREAST_PUMP_CUSTOM_MODE, /// 专家模式上限模式
    BREAST_PUMP_NORMAL_MODE_DEFAULT = BREAST_PUMP_STIMULATION_MODE, /// 默认常规模式
    BREAST_PUMP_EXPERT_MODE_DEFAULT = BREAST_PUMP_MILK_BOOST_MODE, /// 默认韵律
    #if IS_SUPPORT_PUMP_RHYTHM_MODE_SELECT
    BREAST_PUMP_PUMP_MODE_DEFAULT = BREAST_PUMP_MILK_BOOST_MODE, /// 默认韵律模式
    #else
    BREAST_PUMP_PUMP_MODE_DEFAULT = BREAST_PUMP_STIMULATION_MODE, /// 默认常规模式
    #endif
#else
    BREAST_PUMP_STIMULATION_MODE,             /// 刺激模式
    BREAST_PUMP_LACTATION_MODE,               /// 吸乳模式
    BREAST_PUMP_MIX_MODE,                     /// 混合模式
    BREAST_PUMP_REGULAR_PUMPING_MODE,         /// 常规韵律模式（Regular Pumping）
    BREAST_PUMP_MILK_BOOST_MODE,              /// 2+8奶量提升模式
    BREAST_PUMP_PP_MILK_TRACK_MODE,           /// PP追奶模式（Power Pumping模式）
    BREAST_PUMP_LACTATION_START_MODE,         /// 开奶模式
    BREAST_PUMP_EMPTYING_BOOST_MODE,          /// 追加排空模式（Emptying Boost）
    BREAST_PUMP_CUSTOM_MODE,                  /// 个性化模式
    BREAST_PUMP_CUSTOM_TRIAL_MODE,            /// 用户自定义模式(试用)
    BREAST_PUMP_RELAX_MODE,                   /// 放松模式（Relax）
    BREAST_PUMP_MODE_MAX,                     /// 模式最大模式
    BREAST_PUMP_NORMAL_MODE_LOWER = BREAST_PUMP_STIMULATION_MODE, /// 常规模式下限模式
    BREAST_PUMP_NORMAL_MODE_UPPER = BREAST_PUMP_MIX_MODE, /// 常规模式上限模式
    BREAST_PUMP_EXPERT_MODE_LOWER = BREAST_PUMP_REGULAR_PUMPING_MODE, /// 专家模式下限模式
    BREAST_PUMP_EXPERT_MODE_UPPER = BREAST_PUMP_CUSTOM_MODE, /// 专家模式上限模式
    BREAST_PUMP_NORMAL_MODE_DEFAULT = BREAST_PUMP_STIMULATION_MODE, /// 默认常规模式
    BREAST_PUMP_EXPERT_MODE_DEFAULT = BREAST_PUMP_REGULAR_PUMPING_MODE, /// 默认韵律
    #if IS_SUPPORT_PUMP_RHYTHM_MODE_SELECT
    BREAST_PUMP_PUMP_MODE_DEFAULT = BREAST_PUMP_REGULAR_PUMPING_MODE, /// 默认韵律模式
    #else
    BREAST_PUMP_PUMP_MODE_DEFAULT = BREAST_PUMP_STIMULATION_MODE, /// 默认常规模式
    #endif
#endif
    BREAST_PUMP_MODE_INVALID = 0xFF,
    BREAST_PUMP_ALL_MODE_AND_PROGRAM = 0xFFFF, /// 所有模式和韵律
} breast_pump_mode_e;

/// 泵奶模式类型
typedef enum {
    PUMP_MODE_STIMULATION = 0, /// 刺激模式
    PUMP_MODE_LACTATION = 1, /// 吸乳模式
    PUMP_MODE_MIX = 2, /// 混合模式
    PUMP_MODE_IDLE = 3, /// 空闲模式
    PUMP_MODE_UNKNOWN = 0xFF, /// 未知模式
} pump_mode_type_e;

/// 泵奶时长
typedef enum {
    BREAST_PUMPING_TIME_ERROR = 0,
    BREAST_PUMPING_ONE_MIN = 60,
    BREAST_PUMPING_ONE_MIN_HALF = 90,
    BREAST_PUMPING_TWO_MIN = 120,
    BREAST_PUMPING_TWO_MIN_HALF = 150,
    BREAST_PUMPING_THREE_MIN = 180,
    BREAST_PUMPING_THREE_MIN_HALF = 210,
    BREAST_PUMPING_FOUR_MIN = 240,
    BREAST_PUMPING_FOUR_MIN_HALF = 270,
    BREAST_PUMPING_FIVE_MIN = 300,
    BREAST_PUMPING_DEFAULT_TIME = BREAST_PUMPING_TWO_MIN,
} sys_breast_pumping_time_e;

/// 泵奶力度
typedef enum {
    BREAST_PUMP_LEVEL_0 = 0,
    BREAST_PUMP_LEVEL_1,
    BREAST_PUMP_LEVEL_2,
    BREAST_PUMP_LEVEL_3,
    BREAST_PUMP_LEVEL_4,
    BREAST_PUMP_LEVEL_5,
    BREAST_PUMP_LEVEL_6,
    BREAST_PUMP_LEVEL_7,
    BREAST_PUMP_LEVEL_8,
    BREAST_PUMP_LEVEL_9,
    BREAST_PUMP_LEVEL_10,
    BREAST_PUMP_LEVEL_11,
    BREAST_PUMP_LEVEL_12,
    BREAST_PUMP_LEVEL_13,
    BREAST_PUMP_LEVEL_14,
    BREAST_PUMP_LEVEL_15,
    BREAST_PUMP_LEVEL_MAX,
    BREAST_PUMP_LEVEL_DEFAULT = BREAST_PUMP_LEVEL_1,
} breast_pump_level_e;

/// 泵奶速度，与 suction_speed_e 保持一致
/// 泵奶速度，与 suction_speed_e 保持一致
typedef enum {
    BREAST_PUMP_SPEED_LOW = 0,
    BREAST_PUMP_SPEED_MID = 1,
    BREAST_PUMP_SPEED_HIGH = 2,
#if IS_USE_SPECTRA_SUCTION_CURVE
    BREAST_PUMP_SPEED_HIGH_2 = 3,
    BREAST_PUMP_SPEED_HIGH_3 = 4,
#endif
    BREAST_PUMP_SPEED_MAX,
    BREAST_PUMP_SPEED_INVALID = 0xFF,
    BREAST_PUMP_SPEED_DEFAULT = BREAST_PUMP_SPEED_MID,
} breast_pump_speed_e;

/// 泵奶控制类型
typedef enum {
    BREAST_PUMP_CONTROL_TYPE_LEFT = 0,
    BREAST_PUMP_CONTROL_TYPE_RIGHT = 1,
    BREAST_PUMP_CONTROL_TYPE_BILATERAL = 2,
    BREAST_PUMP_CONTROL_TYPE_MAX,
} breast_pump_control_type_e;

/// 泵奶策略类型 对应motor_policy_type_e
typedef enum {
    BREAST_PUMP_TIME_POLICY = 0, /// 时间策略
    BREAST_PUMP_TIME_SUB_POLICY = 1, /// 时间子策略
} breast_pump_policy_type_e;

typedef struct {
    uint8_t level[BREAST_PUMP_NUM];
    uint8_t speed;
    uint8_t policy_type;               /// 策略类型 breast_pump_policy_type_e
} pump_setting_t;

/// 描述泵奶设置
typedef struct {
    pump_setting_t info[ROOT_BREAST_PUMP_STEP_MAX];              /// 保留
} pump_setting_wrap_t;

/// 描述泵奶信息
typedef struct {
    uint8_t rsv[20];                      /// 保留
    uint8_t custom_activate;              /// 激活个性化韵律a
    uint8_t last_expert_mode;             /// 上一次使用的专家韵律模式
    uint8_t last_regular_mode;            /// 上一次使用的常规模式
    uint8_t last_pump_mode;               /// 上一次使用的泵奶模式 breast_pump_mode_e
    uint8_t control_type;                 /// 泵奶控制类型 breast_pump_control_type_e
    bool new_step_info_flag[BREAST_PUMP_MODE_MAX];   /// 泵奶模式新步数信息标志 true-有新步数信息 false-无新步数信息, 使用后重置
    pump_setting_wrap_t setting_wrap[BREAST_PUMP_MODE_MAX];     /// 泵奶设置
} sys_pump_info_t;

/// 描述泵奶模式信息
/// 协议 C4/C5 名称最长 20 字节 ASCII（无结束符），本地多留 1 字节存放 '\0'
#define PUMP_MODE_NAME_LEN                 21
typedef struct {
    pump_mode_steps_info_t steps_info[BREAST_PUMP_MODE_MAX];
    char mode_name[BREAST_PUMP_MODE_MAX][PUMP_MODE_NAME_LEN];   /// 模式名称
} sys_pump_mode_info_t;

/// 用户绑定信息
typedef struct {
    uint8_t bind_status;                    /// 绑定状态 0-未绑定 1-已绑定
    uint8_t user_id[USER_BIND_UID_LEN];     /// root id, eg. TF123456;
    uint8_t random[USER_BIND_RANDOM_LEN];   /// 随机数，绑定时由App 下发，用于计算aes key;
    uint8_t aes_key[USER_BIND_AES_KEY_LEN]; /// 对称密钥，数据加解密
} user_bind_info_t;

typedef enum {
    SYS_UI_STYLE_NONE = 0, // 无UI风格
    SYS_UI_STYLE_DEFAULT = 1, // 默认UI风格
    SYS_UI_STYLE_CUSTOM = 2, // 自定义UI风格
} sys_ui_style_e;

/// 描述系统配置
typedef struct {
    int8_t  time_zone;             /// 时区
    uint8_t ap_mode_onoff;         /// 飞航模式开关.
    uint8_t slient_upgrade_onoff;  /// 静默升级开关
    uint8_t sys_ui_style;          /// 系统UI风格
    uint8_t log_switch;            /// 系统UART日志开关
    uint8_t language;              /// 系统语言类型
    uint8_t rsv[26];               /// 保留
} sys_config_info_t;

/// 描述系统配置
typedef struct {
    uint8_t guide_deployed;       /// 向导部署标志
    uint8_t battery_health;       /// 电池健康
    uint8_t airplane_mode_state;  /// 飞行模式状态
    uint8_t night_light_adaptive_switch; /// 夜灯自适应开关 @deprecated
    uint32_t log_filter;          /// 日志过滤
    uint32_t sync_sys_time;       /// 保存第一次同步系统时间时 本地经历的最终时间点 用于计算泵奶记录的差值.
    uint8_t battery_percent;      /// 电池电量
    uint8_t global_adaptive_switch;             /// App 自动调光总开关（Flash 唯一 ALS 持久化）
    uint8_t night_light_gear;                   /// 夜灯档位
    uint8_t back_light_level;                   /// 全局屏/键背光亮度 20~100
    uint8_t night_light_startup_on;             /// 夜灯开机自动开：0-关 1-开
    uint8_t seal_leak_enabled;                  /// 漏气检测功能开关
    uint8_t rsv[26];              /// 保留
} sys_state_info_t;

typedef struct {
    uint32_t img_id;
    uint32_t total_size;
    uint32_t version;
    uint32_t crc32;
    uint32_t recv_size;
} img_res_ota_info_t;

/// 描述埋点数据
typedef struct sys_buried_info {
    union {
        u32 dat32;
        u8 dat[4];
    } rtc_time; /// 系统时间
    uint8_t sys_event;
    uint8_t pump_state;
    uint8_t pump_mode;
    uint8_t pump_level;
    uint8_t battery;
    uint8_t milk;
    uint8_t shock_swicth;
    uint8_t heat_swicth;
    uint8_t angle;
} sys_buried_info_t;

typedef struct {
    sys_buried_info_t buried_info[ROOT_BREAST_PUMP_BURY_DATA_NUM];
    uint16_t buried_index; /// 埋点索引
} sys_buried_info_list_t;

/// 泵奶数据头
typedef struct {
    uint8_t num;        /// 存储数量.
    uint8_t head;       /// 头索引
    uint8_t tail;       /// 尾索引
    uint8_t resv;
    uint16_t crc[ROOT_BREAST_PUMP_DATA_BODY_NUM + 1];  
} breast_pump_data_head_t;

/** 系统重启原因（数字编号，用于埋点 error_data；前段对应 PMU，后段对应 SW bootmode） */
typedef enum {
    /* PMU / 硬件启动原因 */
    CORE_BOOT_CAUSE_NULL                 = 0,
    CORE_BOOT_CAUSE_POWER_KEY            = 1,
    CORE_BOOT_CAUSE_DIG_WDT              = 2,
    CORE_BOOT_CAUSE_DIG_REBOOT           = 3,
    CORE_BOOT_CAUSE_AC_IN                = 4,
    CORE_BOOT_CAUSE_AC_OUT               = 5,
    CORE_BOOT_CAUSE_RTC                  = 6,
    CORE_BOOT_CAUSE_PMU_WDT              = 7,
    CORE_BOOT_CAUSE_RESET_KEY            = 8,
    CORE_BOOT_CAUSE_LONG_PRESS           = 9,
    CORE_BOOT_CAUSE_VCHG_COMMAND         = 10,
    CORE_BOOT_CAUSE_VCORE_LOW            = 11,
    CORE_BOOT_CAUSE_VBAT_OVP             = 12,
    /* 软件 bootmode（对应 HAL_SW_BOOTMODE_*） */
    SOFT_BOOT_ROM_RESERVED_4             = 13,
    SOFT_BOOT_ROM_RESERVED_5             = 14,
    SOFT_BOOT_ROM_RESERVED_6             = 15,
    SOFT_BOOT_FORCE_USB_DLD              = 16,
    SOFT_BOOT_FORCE_UART_DLD             = 17,
    SOFT_BOOT_DLD_TRANS_UART             = 18,
    SOFT_BOOT_SKIP_FLASH_BOOT            = 19,
    SOFT_BOOT_CHIP_TEST                  = 20,
    SOFT_BOOT_FACTORY                    = 21,
    SOFT_BOOT_CALIB                      = 22,
    SOFT_BOOT_RAM_BOOT                   = 23,
    SOFT_BOOT_FLASH_BOOT                 = 24,
    SOFT_BOOT_REBOOT                     = 25,
    SOFT_BOOT_ROM_RESERVED_17            = 26,
    SOFT_BOOT_FORCE_USB_PLUG_IN          = 27,
    SOFT_BOOT_POWER_DOWN_WAKEUP          = 28,
    SOFT_BOOT_TEST_MODE                  = 29,
    SOFT_BOOT_TEST_SIGNALINGMODE         = 30,
    SOFT_BOOT_TEST_NOSIGNALINGMODE       = 31,
    SOFT_BOOT_ENTER_HIDE_BOOT            = 32,
    SOFT_BOOT_CUSTOM_OP1_AFTER_REBOOT    = 33,
    SOFT_BOOT_REBOOT_FROM_CRASH          = 34,
    SOFT_BOOT_SINGLE_LINE_DOWNLOAD       = 35,
    SOFT_BOOT_CUSTOM_OP2_AFTER_REBOOT    = 36,
    SOFT_BOOT_CDC_OR_TEST_NORMAL         = 37, /* HAL: CDC_COMM / TEST_NORMAL_MODE */
    SOFT_BOOT_A7_OR_BT_ON                = 38, /* HAL: A7_REBOOT / REBOOT_BT_ON */
    SOFT_BOOT_WARM_OR_ANC_ON             = 39, /* HAL: PMU_WARM_BOOT / REBOOT_ANC_ON */
} sys_reboot_reason_e;
/// 描述系统信息
typedef struct sys_persist_info {
    sys_config_info_t config;
    sys_state_info_t state;
    sys_pump_info_t pump_info;
    sys_pump_mode_info_t mode_info;
    user_bind_info_t bind_info;
    sys_buried_info_list_t buried_info_list;
    uint8_t reboot_reason; /// 系统重启原因 sys_reboot_reason_e
} sys_persist_info_t;

/// 描述板子信息
typedef struct sys_board_config {
    const char *git_hash;
    const char *device_name;
    const char *buildtime;
    uint16_t firmware_type;
    uint16_t firmware_verno;
    uint16_t res_major_verno;
    uint16_t res_minor_verno;
    uint16_t product_id;
} sys_board_config_t;

/// 描述板子信息
typedef struct {
    char device_name[32];
    uint16_t sys_connect_type;
    uint16_t sys_local_protocol_id;
    uint16_t sys_peer_app_protocol_id;
    uint16_t sys_product_id;
    uint16_t ui_res_id;
    uint16_t hw_version;
    uint16_t sw_version;
    uint16_t soc_version;
    uint16_t sw_version_type;
    uint8_t dfu_type;  /// 0:dfu  1:fast dfu
} sec_sys_ver_t;

typedef struct {
#if IS_USE_SPECTRA_SUCTION_CURVE
    suction_parameters_t short_suction_table[BREAST_PUMP_LEVEL_MAX][SHORT_SUCTION_SPEED_MAX];
    suction_parameters_t long_suction_table[BREAST_PUMP_LEVEL_MAX][PUMP_LONG_SUCTION_SPEED_MAX];
    uint16_t long_pump_freq_table[PUMP_LONG_SUCTION_SPEED_MAX];
    uint16_t short_suction_freq_table[SHORT_SUCTION_SPEED_MAX];
#else
    suction_parameters_t short_suction_table[SUCTION_LEVEL_MAX];
    suction_parameters_t long_suction_table[SUCTION_LEVEL_MAX];
    uint16_t long_pump_freq_table[BREAST_PUMP_LEVEL_MAX][SUCTION_SPEED_MAX];
#endif
} factory_suction_para_t;

/// 工厂校准数据结构, 恢复出厂不擦除.
typedef struct factory_data_store {
    factory_flag_info_t flag_info;
    factory_dev_info_t dev_info;
    img_res_ota_info_t res_ota_info;
    #if IS_SUPPORT_DEVICE_IMU
    imu_cali_data_t imu;
    #endif
    factory_suction_para_t factory_suction_para;
} factory_data_store_t;

#ifdef __cplusplus
}
#endif

#endif 

