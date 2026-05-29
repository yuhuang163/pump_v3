#ifndef __APP_PROTOCOL_PUMP_CTRL_SERVER_H__
#define __APP_PROTOCOL_PUMP_CTRL_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "persist_storage.h" // IWYU pragma: keep

typedef enum {
    APP_OLD_COMM_SYS_FINISH      = 0x00,
    APP_OLD_COMM_SYS_RUN       = 0x01,
    APP_OLD_COMM_SYS_CHANRGE   = 0x02,
    APP_OLD_COMM_SYS_OFF       = 0x03,
    APP_OLD_COMM_SYS_RESET     = 0x05,
    APP_OLD_COMM_SYS_PAUSE     =  0x06,
    APP_OLD_COMM_PUMP_FINISH_IN_CHARGE = 0x07,
    APP_OLD_COMM_PUMP_RUN_IN_CHARGE = 0x08,
    APP_OLD_COMM_PUMP_PAUSE_IN_CHARGE = 0x09,
} app_old_commd_sys_status_e;

typedef enum {
    APP_OLD_COMM_LEFT_CT       = 0x00,
    APP_OLD_COMM_RIGHT_CT      = 0x01,
    APP_OLD_COMM_ALL_CT        = 0x02,
    APP_OLD_COMM_INVALID_CT    = 0x03,
} app_old_commd_l_r_ctrl_e;

/// 旧协议泵奶模式（与 CTRL_DEV / 状态上报 cab[1] 一致，0x01~0x0B）
typedef enum {
    APP_OLD_COMM_MODE_MASSAGE = 0x01,           /// 按摩（刺激）
    APP_OLD_COMM_MODE_SUCTION = 0x02,           /// 吸乳
    APP_OLD_COMM_MODE_MIXED = 0x03,             /// 混合
    APP_OLD_COMM_MODE_REGULAR_PUMPING = 0x04,   /// 常规韵律
    APP_OLD_COMM_MODE_MILK_BOOST = 0x05,        /// 2+8 奶量提升
    APP_OLD_COMM_MODE_PP_MILK_TRACK = 0x06,     /// PP 追奶
    APP_OLD_COMM_MODE_LACTATION_START = 0x07,   /// 开奶
    APP_OLD_COMM_MODE_EMPTYING_BOOST = 0x08,    /// 追加排空
    APP_OLD_COMM_MODE_CUSTOM = 0x09,            /// 个性化
    APP_OLD_COMM_MODE_CUSTOM_TRIAL = 0x0A,      /// 自定义试用
    APP_OLD_COMM_MODE_RELAX = 0x0B,             /// 放松
} app_old_comm_pump_mode;

typedef enum {
    APP_OLD_COMM_STEP_MODE_SHORT = 0x01, /// 短吸（按摩）
    APP_OLD_COMM_STEP_MODE_LONG = 0x02,  /// 长吸（吸乳）
    APP_OLD_COMM_STEP_MODE_MIX1 = 0x03,  /// 混合1
    APP_OLD_COMM_STEP_MODE_MIX2 = 0x04,  /// 混合2
    APP_OLD_COMM_STEP_MODE_RELAX = 0x05, /// 放松步骤
} app_old_comm_custom_pump_step_mode;

typedef enum {
    APP_OLD_COMM_TASTE_MODE_STIMULATION = 0x01, // 刺激模式
    APP_OLD_COMM_TASTE_MODE_SUCTION = 0x02, // 吸奶模式
    APP_OLD_COMM_TASTE_MODE_MIX1 = 0x03, // 混合模式
    APP_OLD_COMM_TASTE_MODE_MIX2 = 0x04, // 混合模式
} app_old_comm_taste_mode;

typedef enum {
    APP_OLD_COMM_TASTE_STATUS_STOP      = 0x00,
    APP_OLD_COMM_TASTE_STATUS_RUN       = 0x01,
    APP_OLD_COMM_TASTE_STATUS_EXIT      = 0x02,
} app_old_comm_taste_status_ctrl_e;

typedef enum {
    APP_OLD_COMM_PUMP_CTRL_STOP      = 0x00,
    APP_OLD_COMM_PUMP_CTRL_RUN       = 0x01,
} app_old_comm_pump_ctrl_e;

typedef struct {
    uint8_t pump_ctrl_status;
    uint8_t pump_mode;
    uint8_t normal_pump_level;
    uint8_t user_define_pump_step_mode;
    uint8_t user_define_pump_level;
    uint8_t user_define_pump_speed;
    uint8_t pump_mode_lr_or_all;
    uint8_t normal_pump_speed;
} app_old_comm_pump_ctrl;

/// BLE记录数据结构定义
/// 日期结构（年月日）
typedef struct __attribute__((packed)) {
    uint8_t year;    /// 年份（2000+year，如2024年存储为24）
    uint8_t month;   /// 月份（1-12）
    uint8_t day;     /// 日期（1-31）
} app_old_comm_record_date_t;

/// 时钟时间结构（时分秒）
typedef struct __attribute__((packed)) {
    uint8_t hour;    /// 小时（0-23）
    uint8_t minute;  /// 分钟（0-59）
    uint8_t second;  /// 秒（0-59）
} app_old_comm_record_time_t;

/// 泵奶工作记录结构（用于BLE通信）
typedef struct __attribute__((packed)) {
    app_old_comm_record_time_t clock;  /// 时钟时间（时分秒）
    uint8_t mod;          /// 泵奶模式
    uint8_t lvl;          /// 泵奶级别
    uint16_t time;        /// 泵奶时长（秒）
    uint8_t left;         /// 左侧奶量（ML）
    uint8_t right;        /// 右侧奶量（ML）
} app_old_comm_pump_record_day_t;

/// BLE泵奶记录结构（日期 + 工作记录）
typedef struct __attribute__((packed)) {
    app_old_comm_record_date_t date;   /// 日期信息（年月日）
    app_old_comm_pump_record_day_t work; /// 工作记录信息
} app_old_comm_pump_ble_record_t;

//pump_ctrl
void app_old_comm_handle_ctrl_dev_req(uint8_t cal, uint8_t *cab);
void app_old_comm_report_device_status(app_old_commd_l_r_ctrl_e lr_ct, servers_old_protocol_publish_data_id_e id, void *val);
/// Notify CID=0xED：上报自定义模式 ID（CAL=4；0 表示 ID 为空），载荷小端序
void app_old_comm_report_custom_mode_id(app_old_commd_l_r_ctrl_e lr_ct);
void app_old_comm_handle_get_status_req(app_old_commd_l_r_ctrl_e lr_ct);
void app_old_comm_handle_ctrl_dev_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_user_define_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_dev_info_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_trail_user_define_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_ctrl_taste_dev_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_ctrl_reset_dev_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_taste_dev_status_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_report_taste_dev_status(servers_old_protocol_publish_data_id_e id, void *val);
void app_old_comm_handle_get_record_num_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_record_data_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_clear_trail_custom_mode_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_bilateral_pump_param_req(uint8_t cal, uint8_t *cab);
#ifdef __cplusplus
}
#endif

#endif 