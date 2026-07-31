#ifndef __BREAST_PUMP_MODE_H__
#define __BREAST_PUMP_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
const motor_policy_cfg_t *get_breast_pump_mode_para(int mode);
int breast_pump_mode_parameter_output(motor_policy_manager_t *obj, char *name);
int breast_pump_mode_parameter_deinit(motor_policy_manager_t *obj);
motor_policy_manager_t *breast_pump_mode_parameter_init(const motor_policy_cfg_t *cfg, uint8_t cfg_num, char *name);
motor_policy_manager_t *pump_mode_parameter_init_by_steps_info(pump_mode_steps_info_t *steps_info, char *name);

const motor_policy_manager_t *pump_mode_policy_get(void *user_data);
int pump_mode_parameter_init(void);

const motor_policy_manager_t *mix_mode_policy_get(void *user_data);
int mix_mode_parameter_init(void);

const motor_policy_manager_t *lactation_start_mode_policy_get(void *user_data);
int lactation_start_mode_parameter_init(void);

const motor_policy_manager_t *custom_mode_policy_get(void *user_data);
int custom_mode_parameter_init(void);
int custom_mode_update_by_steps_info(pump_mode_steps_info_t *steps_info);

const motor_policy_manager_t *custom_trial_mode_policy_get(void *user_data);
int custom_trial_mode_parameter_init(void);
int custom_trial_mode_update_by_steps_info(pump_mode_steps_info_t *steps_info);

const motor_policy_manager_t *pp_milk_track_mode_policy_get(void *user_data);
int pp_milk_track_mode_parameter_init(void);

const motor_policy_manager_t *stimulation_mode_policy_get(void *user_data);
int stimulation_mode_parameter_init(void);

const motor_policy_manager_t *milk_boost_mode_policy_get(void *user_data);
int milk_boost_mode_parameter_init(void);

const motor_policy_manager_t *regular_pumping_mode_policy_get(void *user_data);
int regular_pumping_mode_parameter_init(void);

const motor_policy_manager_t *emptying_boost_mode_policy_get(void *user_data);
int emptying_boost_mode_parameter_init(void);

const motor_policy_manager_t *relax_mode_policy_get(void *user_data);
int relax_mode_parameter_init(void);

int breast_pump_mode_work_time_get(int mode);
int breast_pump_mode_time_step_number_get(int mode);
void breast_pump_setting_reset (sys_pump_info_t *pump_info, int mode);
void breast_pump_mode_steps_info_reset(sys_pump_mode_info_t *mode_info, int mode);
int breast_pump_mode_step_target_time_get(int mode, int step);
void policy_manager_obj_lock(void);
void policy_manager_obj_unlock(void);
void pump_setting_update_ram(breast_pump_manager_context_t *ctx,
    pump_setting_t *src, uint8_t step_idx);
void pump_setting_store(breast_pump_manager_context_t *ctx, int mode, uint8_t step_idx);
void pump_setting_sync_by_mode_type(breast_pump_manager_context_t *ctx,
    pump_setting_t *setting, uint8_t step_index, int mode, bool is_need_store);

#ifdef __cplusplus
}
#endif

#endif 







