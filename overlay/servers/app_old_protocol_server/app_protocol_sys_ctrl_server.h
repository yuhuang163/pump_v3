#ifndef __APP_PROTOCOL_SYS_CTRL_SERVER_H__
#define __APP_PROTOCOL_SYS_CTRL_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

typedef enum {
    APP_OLD_COMM_SYS_POWER_OFF = 0x01,
    APP_OLD_COMM_SYS_REBOOT = 0x02,
    APP_OLD_COMM_SYS_OTA = 0x03,
    APP_OLD_COMM_SYS_OFF_OTA_RECOVERY = 0x04,
} app_old_commd_sys_ctrl_e;

typedef struct {
    uint8_t sys_language_e;
    uint8_t screem_lock_time;
    uint8_t screem_lock_light_level;
    uint16_t motor_Duration;
    uint8_t adaptive_dimming_switch;
    uint8_t device_bind_status; /// app_old_comm_bind_status_e
} app_old_comm_sys_status_ctrl;

/// 协议绑定状态，需转译到本地 BLE_BIND_STATE_*
typedef enum {
    APP_OLD_COMM_BIND_STATUS_UNBIND = 0,    /// 未绑定/绑定失败
    APP_OLD_COMM_BIND_STATUS_BINDING,       /// 绑定中
    APP_OLD_COMM_BIND_STATUS_COMPLETE,      /// 绑定完成
} app_old_comm_bind_status_e;

#define APP_OLD_COMM_LIGHT_LEVEL_MAX  100  /// 协议亮度档位上限（0~100，与驱动百分比一致）

/// 灯光状态（0xC2 / 0xBE / 0xDE，CAL=4）
typedef struct __attribute__((packed)) {
    uint8_t night_light_switch;  /// 夜灯功能开关
    uint8_t night_light_gear;   /// 夜灯档位
    uint8_t adaptive_switch;     /// 自适应开关：0-关，1-开
    uint8_t screen_key_level;    /// 屏幕/按键亮度 0-100
} app_old_comm_light_status_t;

#define APP_OLD_COMM_LIGHT_STATUS_CAL  ((uint8_t)sizeof(app_old_comm_light_status_t))

void app_old_comm_handle_set_bind_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_heartbeat_req(void);
void app_old_comm_handle_set_rtc_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_sys_ctrl_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_encrypt_req(void);
void app_old_comm_handle_set_travel_lock_req(uint8_t cal, uint8_t *cab);
void app_old_comm_report_discon_status(void);
void app_old_comm_report_bind_status(uint8_t bind_status, uint8_t left_right);
void app_old_comm_handle_ctrl_dev_sys_status_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_pump_ble_rgb_set_ctrl_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_light_status_req(uint8_t cal, uint8_t *cab);
void app_old_comm_report_light_status(uint8_t arg1, void *val);
void app_old_comm_handle_dev_abnormal_report(void *cab);
void app_old_comm_handle_dev_abnormal_app_req(uint8_t cal, uint8_t *cab);
bool app_old_comm_check_if_fault_warning(void);
void app_old_comm_report_abnormal(uint8_t abnormal_type, uint8_t abnormal_state);

#ifdef __cplusplus
}
#endif

#endif 
