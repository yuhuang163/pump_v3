#ifndef __PERSIST_STORAGE_H__
#define __PERSIST_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "root_algo.h" // IWYU pragma: keep

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

#define IS_PUMP_STEPS_USE_SAME_SETTING              1 /// 是否使用相同的设置，如果使用相同的设置，则所有步骤都使用相同的设置
#define ROOT_BREAST_PUMP_STEP_MAX                   32
#define FACTORY_AGE_INFO_MAX                        3   /// 老化信息最大保存3条
#define ROOT_BREAST_PUMP_BURY_DATA_NUM              10    /// 埋点数据最大保存10条

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
    uint16_t duration;   /// 持续时间，单位s
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
    BREAST_PUMP_STIMULATION_MODE,      /// 刺激模式
    BREAST_PUMP_LACTATION_MODE,        /// 吸乳模式
    BREAST_PUMP_MIX_MODE,              /// 混合模式
    BREAST_PUMP_REGULAR_PUMPING_MODE,         /// 常规韵律模式（Regular Pumping）
    BREAST_PUMP_MILK_BOOST_MODE,              /// 2+8奶量提升模式
    BREAST_PUMP_PP_MILK_TRACK_MODE,           /// PP追奶模式（Power Pumping模式）
    BREAST_PUMP_LACTATION_START_MODE,         /// 开奶模式
    BREAST_PUMP_EMPTYING_BOOST_MODE,          /// 追加排空模式（Emptying Boost）
    BREAST_PUMP_CUSTOM_MODE,                  /// 个性化模式
    BREAST_PUMP_CUSTOM_TRIAL_MODE,            /// 用户自定义模式(试用)
    BREAST_PUMP_RELAX_MODE,                   /// 放松模式（Relax）
    BREAST_PUMP_MODE_MAX,
    BREAST_PUMP_NORMAL_MODE_MAX = BREAST_PUMP_MIX_MODE - BREAST_PUMP_STIMULATION_MODE + 1,
    BREAST_PUMP_NORMAL_MODE_DEFAULT = BREAST_PUMP_STIMULATION_MODE, /// 默认常规模式
    BREAST_PUMP_PUMP_MODE_DEFAULT = BREAST_PUMP_REGULAR_PUMPING_MODE, /// 默认泵奶模式

    BREAST_PUMP_MODE_INVALID = 0xFF,
} breast_pump_mode_e;

/// 泵奶模式类型
typedef enum {
    PUMP_MODE_STIMULATION = 0, /// 刺激模式
    PUMP_MODE_LACTATION = 1, /// 吸乳模式
    PUMP_MODE_MIX = 2, /// 混合模式
    PUMP_MODE_IDLE = 3, /// 空闲模式
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

/// 泵奶控制类型
typedef enum {
    BREAST_PUMP_SPEED_LOW = 0,
    BREAST_PUMP_SPEED_MID = 1,
    BREAST_PUMP_SPEED_HIGH = 2,
    BREAST_PUMP_SPEED_MAX,
} breast_pump_speed_e;

/// 泵奶控制类型
typedef enum {
    BREAST_PUMP_CONTROL_TYPE_LEFT = 0,
    BREAST_PUMP_CONTROL_TYPE_RIGHT = 1,
    BREAST_PUMP_CONTROL_TYPE_BILATERAL = 2,
    BREAST_PUMP_CONTROL_TYPE_MAX,
} breast_pump_control_type_e;

typedef struct {
    uint8_t level[BREAST_PUMP_NUM];
    uint8_t speed;
    uint8_t rsv;               /// 保留
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
    uint8_t mode_update_flag[BREAST_PUMP_MODE_MAX];   /// 泵奶模式更新标志 0-未更新 1-已更新, 使用后重置
    pump_setting_wrap_t setting_wrap[BREAST_PUMP_MODE_MAX];     /// 泵奶设置
} sys_pump_info_t;

/// 描述泵奶模式信息
#define PUMP_MODE_NAME_LEN                 20
typedef struct {
    pump_mode_steps_info_t steps_info[BREAST_PUMP_MODE_MAX];
    char mode_name[BREAST_PUMP_MODE_MAX][PUMP_MODE_NAME_LEN];   /// 模式名称
} sys_pump_mode_info_t;

typedef enum {
    THEME_PAKCET_ORIGINAL, 
    THEME_PACKET_OFFICIAL,
    THEME_PACKET_PUSH,   
    THEME_PACKET_GIFT,
    THEME_PACKET_CUSTOME,
    THEME_PACKET_MAX,
} theme_type_e;

typedef struct {
    uint8_t file_id;
    // char filename[ROOT_FILENAME_LEN];
} theme_item_info_t;

typedef struct {
    uint8_t cur_theme_type;  
    uint8_t last_theme_type;
    theme_item_info_t theme_info[THEME_PACKET_MAX];
} theme_packet_info_t;

/// 用户绑定信息
typedef struct {
    uint8_t bind_status;                    /// 绑定状态 0-未绑定 1-已绑定
    uint8_t user_id[USER_BIND_UID_LEN];     /// root id, eg. TF123456;
    uint8_t random[USER_BIND_RANDOM_LEN];   /// 随机数，绑定时由App 下发，用于计算aes key;
    uint8_t aes_key[USER_BIND_AES_KEY_LEN]; /// 对称密钥，数据加解密
} user_bind_info_t;

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
    uint8_t night_light_adaptive_switch; /// 夜灯自适应开关
    uint32_t log_filter;          /// 日志过滤
    uint32_t sync_sys_time;       /// 保存第一次同步系统时间时 本地经历的最终时间点 用于计算泵奶记录的差值.
    uint8_t rsv[32];                   /// 保留
} sys_state_info_t;

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
    uint8_t head;  
    uint8_t tail;
    uint8_t resv;
    uint16_t crc[ROOT_BREAST_PUMP_DATA_BODY_NUM + 1];  
} breast_pump_data_head_t;

/// 描述系统信息
typedef struct sys_persist_info {
    sys_config_info_t config;
    sys_state_info_t state;
    sys_pump_info_t pump_info;
    sys_pump_mode_info_t mode_info;
    user_bind_info_t bind_info;
    sys_buried_info_list_t buried_info_list;
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

/// 工厂校准数据结构, 恢复出厂不擦除.
typedef struct factory_data_store {
    factory_flag_info_t flag_info;
    factory_dev_info_t dev_info;
    #if IS_SUPPORT_DEVICE_IMU
    imu_cali_data_t imu;
    #endif
} factory_data_store_t;

#ifdef __cplusplus
}
#endif

#endif 

