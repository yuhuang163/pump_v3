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
    if (setting.speed >= SUCTION_SPEED_MAX) {
        setting.speed = SUCTION_SPEED_MAX - 1;
    }
    if (setting.level[pump_id] >= BREAST_PUMP_LEVEL_MAX) {
        setting.level[pump_id] = BREAST_PUMP_LEVEL_MAX - 1;
    }
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
    pump_setting_t setting;
    uint8_t update_flag = 0;
    /// 获取当前时间策略索引.
    int time_index = thiz->get_runtime_policy_index(MOTOR_TIME_POLICY);
    if (time_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        time_index = 0;
    }
    /// 获取当前模式下的设置信息. //默认所有步数的设置都是一样的
    taste_setting_t taste_setting = thiz->priv.runtime.taste_setting;
    /// 边界检查，确保参数在有效范围内
    if (setting.speed >= SUCTION_SPEED_MAX) {
        setting.speed = SUCTION_SPEED_MAX - 1;
    }
    if (setting.level[pump_id] >= BREAST_PUMP_LEVEL_MAX) {
        setting.level[pump_id] = BREAST_PUMP_LEVEL_MAX - 1;
    }
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