#ifndef __APP_PROTOCOL_BROADCAST_TO_WINDOW_H__
#define __APP_PROTOCOL_BROADCAST_TO_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

typedef enum {
    OLD_BROADCAST_LEFT_CT       = 0x00,
    OLD_BROADCAST_RIGHT_CT      = 0x01,
    OLD_BROADCAST_ALL_CT        = 0x02,
} app_broadcast_l_r_ctrl_e;

typedef struct {
    uint8_t lr_ct;
    uint8_t pump_mode;    
} app_broadcast_pump_mode;

typedef struct {
    uint8_t lr_ct;
    uint8_t pump_level;    
} app_broadcast_pump_level;

typedef struct {
    uint8_t lr_ct;
    uint8_t pump_speed;    
} app_broadcast_pump_speed;

typedef struct {
    uint8_t lr_ct;
    uint8_t pump_status;    
} app_broadcast_pump_status;

void app_broadcast_pump_mode_to_window(uint8_t mode, app_broadcast_l_r_ctrl_e lr_ct);
void app_broadcast_pump_level_to_window(uint8_t level, app_broadcast_l_r_ctrl_e lr_ct);
void app_broadcast_pump_speed_to_window(uint8_t speed, app_broadcast_l_r_ctrl_e lr_ct);
void app_broadcast_pump_status_to_window(uint8_t status, app_broadcast_l_r_ctrl_e lr_ct);

#ifdef __cplusplus
}
#endif

#endif 
