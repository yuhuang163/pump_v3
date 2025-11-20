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
int pump_climb_policy_update(struct motor_manager_context *thiz, suction_strategy_mapping_t *map) 
{
    static uint8_t climb_status = 0;
    if (thiz->priv.runtime.is_climb_policy == 0) {
        return 0;
    } 

    /// 爬升策略启动3秒内生效
    uint32_t run_time = get_timestamp_ms(thiz->priv.runtime.manager_start_time);
    if (run_time < 1000) {
        map->level = map->level > BREAST_PUMP_LEVEL_4 ? map->level - 3 : BREAST_PUMP_LEVEL_1;
        if (climb_status != 1) {
            climb_status = 1;
            LOG_MOTOR("climb status: %d, level: %d", climb_status, map->level);
        }
    } else if (run_time < 2000) {
        map->level = map->level > BREAST_PUMP_LEVEL_3 ? map->level - 2 : BREAST_PUMP_LEVEL_1;
        if (climb_status != 2) {
            climb_status = 2;
            LOG_MOTOR("climb status: %d, level: %d", climb_status, map->level);
        }
    } else if (run_time < 3000) {
        map->level = map->level > BREAST_PUMP_LEVEL_2 ? map->level - 1 : BREAST_PUMP_LEVEL_1;
        if (climb_status != 3) {
            climb_status = 3;
            LOG_MOTOR("climb status: %d, level: %d", climb_status, map->level);
        }
    } else {
        thiz->priv.runtime.is_climb_policy = 0;
        climb_status = 0;
        return 0;
    }
    return 0;
}