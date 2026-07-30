/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_policy.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int pump_user_policy_update(struct motor_manager_context *thiz, suction_strategy_mapping_t *map, uint8_t pump_id) 
{ 
    pump_setting_t setting;
    uint8_t update_flag = 0;
    /// 获取当前时间策略索引.
    uint8_t time_index = thiz->get_runtime_policy_index(MOTOR_TIME_POLICY);
    /// 获取当前模式下的设置信息.
    READ_PUMP_SETTING_INFO(thiz->priv.pump_ctx, time_index, setting); 
    /// 边界检查，确保参数在有效范围内
#if IS_USE_SPECTRA_SUCTION_CURVE
    /// 刺激/吸乳模式均为5档速度
    uint8_t speed_max = get_suction_speed_max_by_mode(map->mode_type);
    if (setting.speed >= speed_max) {
        setting.speed = speed_max - 1;
    }
#else
    if (setting.speed >= SUCTION_SPEED_MAX) {
        setting.speed = SUCTION_SPEED_MAX - 1;
    }
#endif
    if (setting.level[pump_id] >= BREAST_PUMP_LEVEL_MAX) {
        setting.level[pump_id] = BREAST_PUMP_LEVEL_MAX - 1;
    }
#if IS_USE_SPECTRA_SUCTION_CURVE
    if ((map->mode_type == SUCTION_MODE_SHORT || map->mode_type == SUCTION_MODE_MIX_SHORT) &&
        setting.level[pump_id] > BREAST_PUMP_LEVEL_5) {
        setting.level[pump_id] = BREAST_PUMP_LEVEL_5;
    }
#endif
    /// 更新泵奶速度.
    if (map->speed != setting.speed) {
        map->speed = setting.speed;
        update_flag = 1;
    }
    /// 更新泵奶力度.
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (map->level != setting.level[pump_id]) {
            map->level = setting.level[pump_id];
            update_flag = 1;
        }
    }
    return update_flag;
}

int taste_mode_user_policy_update(struct motor_manager_context *thiz, suction_strategy_mapping_t *map, uint8_t pump_id)
{
    uint8_t update_flag = 0;
    /// 获取当前时间策略索引.
    int time_index = thiz->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    /// 获取当前模式下的设置信息. //默认所有步数的设置都是一样的
    taste_setting_t taste_setting = thiz->priv.runtime.taste_setting;
    /// 边界检查，确保参数在有效范围内
#if IS_USE_SPECTRA_SUCTION_CURVE
    /// 刺激/吸乳模式均为5档速度
    uint8_t speed_max = get_suction_speed_max_by_mode(map->mode_type);
    if (taste_setting.speed >= speed_max) {
        taste_setting.speed = speed_max - 1;
    }
#else
    if (taste_setting.speed >= SUCTION_SPEED_MAX) {
        taste_setting.speed = SUCTION_SPEED_MAX - 1;
    }
#endif
    if (taste_setting.level[pump_id] >= BREAST_PUMP_LEVEL_MAX) {
        taste_setting.level[pump_id] = BREAST_PUMP_LEVEL_MAX - 1;
    }
#if IS_USE_SPECTRA_SUCTION_CURVE
    if ((map->mode_type == SUCTION_MODE_SHORT || map->mode_type == SUCTION_MODE_MIX_SHORT) &&
        taste_setting.level[pump_id] > BREAST_PUMP_LEVEL_5) {
        taste_setting.level[pump_id] = BREAST_PUMP_LEVEL_5;
    }
#endif
    /// 更新泵奶速度.
    if (map->speed != taste_setting.speed) {
        map->speed = taste_setting.speed;
        update_flag = 1;
    }
    /// 更新泵奶力度.
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (map->level != taste_setting.level[pump_id]) {
            map->level = taste_setting.level[pump_id];
            update_flag = 1;
        }
    }
    return update_flag;
}