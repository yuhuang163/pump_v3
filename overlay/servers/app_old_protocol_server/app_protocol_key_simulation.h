#ifndef __APP_PROTOCOL_KEY_SIMULATION_H__
#define __APP_PROTOCOL_KEY_SIMULATION_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

void app_start_key_simulation(void);
void app_mode_key_simulation(void);
void app_frequency_key_simulation(void);
void app_breastfeeding_key_simulation(void);
void app_left_control_key_simulation(void);
void app_right_control_key_simulation(void);
void app_encoder_key_simulation(void);  

#ifdef __cplusplus
}
#endif
#endif 