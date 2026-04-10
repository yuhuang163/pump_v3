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
    uint8_t device_bind_status;
} app_old_comm_sys_status_ctrl;

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
void app_old_comm_handle_dev_abnormal_report(void *cab);

#ifdef __cplusplus
}
#endif

#endif 
