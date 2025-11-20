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

    thiz->priv.pump_ctx->get_breast_pump_setting(&setting);
    
    /// 边界检查，确保参数在有效范围内
    if (setting.speed >= SUCTION_SPEED_MAX) {
        setting.speed = SUCTION_SPEED_MAX - 1;
    }
    if (setting.level[pump_id] >= BREAST_PUMP_LEVEL_MAX) {
        setting.level[pump_id] = BREAST_PUMP_LEVEL_MAX - 1;
    }
    
    if (map->speed != setting.speed) {
        map->speed = setting.speed;
        update_flag = 1;
    }

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        if (map->level != setting.level[pump_id]) {
            map->level = setting.level[pump_id];
            update_flag = 1;
        }
    }
    return update_flag;
}