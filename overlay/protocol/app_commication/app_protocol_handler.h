#ifndef __APP_PROTOCOL_HANDLER_H__
#define __APP_PROTOCOL_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_parse.h"
// Device status definitions
typedef enum {
    APP_OLD_COMM_DEV_STOP        = 0x00,  // Stopped
    APP_OLD_COMM_DEV_RUNNING     = 0x01,  // Running
    APP_OLD_COMM_DEV_CHARGING    = 0x02,  // Charging
    APP_OLD_COMM_DEV_POWER_OFF   = 0x03,  // Power off
    APP_OLD_COMM_DEV_CUSTOM_END  = 0x05   // Custom task ended
} AppOldCommDevStatus;

// Breast pumping mode definitions
typedef enum {
    APP_OLD_COMM_MODE_MASSAGE    = 0x01,  // Massage
    APP_OLD_COMM_MODE_SUCTION    = 0x02,  // Suction
    APP_OLD_COMM_MODE_MIXED      = 0x03,  // Mixed
    APP_OLD_COMM_MODE_CUSTOM     = 0x04,  // Custom
    APP_OLD_COMM_MODE_SMART      = 0x05   // Smart suction
} AppOldCommMilkMode;

// Device information structure
typedef struct {
    uint8_t product_type;    // Product model (0x01=M9, 0x02=G1, 0x03=AIR1)
    uint8_t hardware_platform;// Hardware platform (0x01=JL)
    uint8_t hardware_ver;    // Hardware version
    uint8_t software_ver;    // Software version
    uint32_t custom_mode_id; // Custom mode ID
    uint8_t milk_full_ver;   // Milk full detection parameter version
    uint8_t milk_full_switch;// Milk full detection switch (0=off, 1=on)
    char sn[36];             // Device SN code (max 35 bytes)
    uint8_t encrypt_status;  // Encryption status (0=none, 1=enabled)
    uint8_t encrypt_key;     // Encryption key
} AppOldCommDeviceInfo;

// Device status structure
typedef struct {
    uint8_t battery;         // Battery level (0-100%)
    AppOldCommDevStatus status; // Device status
    uint8_t left_right;      // Left/right indicator (0=left, 1=right)
    AppOldCommMilkMode milk_mode; // Pumping mode
    uint8_t milk_gear;       // Pumping gear (1-15)
    uint8_t custom_mode;     // Custom mode type
    uint8_t custom_gear;     // Custom gear (1-15)
    uint8_t custom_freq;     // Custom frequency (1=low, 2=medium, 3=high)
    uint16_t total_time;     // Total time (seconds)
    uint16_t mode_time;      // Mode time (seconds)
    uint8_t milk_volume;     // Milk volume (ml)
    uint8_t error_flag;      // Error status flag
    uint16_t custom_total_time;// Custom mode total time
    uint8_t travel_lock;     // Travel lock status (0=exit, 1=enter)
} AppOldCommDeviceStatus;

typedef struct app_old_protocol_handler_context {
    os_timer_id poll_timer;

} app_old_protocol_handler_context_t;

void app_old_comm_handle_request_cmd(AppOldCommCmdId cid, uint8_t cal, uint8_t *cab);
void app_old_comm_handle_response_cmd(AppOldCommCmdType ct, AppOldCommCmdId cid, uint8_t cal, uint8_t *cab);
void app_old_comm_variable_init(void);
void app_old_protocol_data_cryption(uint8_t key, uint8_t *dat, uint8_t len);
void rc4(unsigned char *key, unsigned int keyLength, unsigned char *data, unsigned int dataLength);
AppOldCommDeviceInfo* app_old_comm_dev_info_get(void);
AppOldCommDeviceStatus* app_old_comm_dev_status_get(void);
void app_old_protocol_ctx_init(void);

//auth
void app_old_comm_handle_get_triple_req(void);
void app_old_comm_handle_set_productid_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_deviceid_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_encryption_key_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_set_sn_req(uint8_t cal, uint8_t *cab);
void app_old_comm_handle_get_sn_req(void);

#ifdef __cplusplus
}
#endif

#endif 