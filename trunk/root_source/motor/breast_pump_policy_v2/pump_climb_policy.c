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
    pump_climb_policy_t *climb = &thiz->priv.runtime.climb;
    if (climb->onoff == 0) {
        return 0;
    } 

    if (get_timestamp_ms(climb->start_time) > 1000) {
        climb->count++;
        if (climb->count > 3) {
            climb->onoff = 0;
            climb->count = 0;
            climb->start_time = 0;
            return 0;
        }
        LOG_MOTOR("climb count: %d, level: %d", climb->count, map->level);
        climb->start_time = get_timestamp();
    }

    if (map->level == 0) {
        return 0;    /// 如果力度为0，则不进行爬升
    }

    if (climb->count == 1) {
        map->level = map->level > BREAST_PUMP_LEVEL_4 ? (map->level - 3) : BREAST_PUMP_LEVEL_1;
    } else if (climb->count == 2) {
        map->level = map->level > BREAST_PUMP_LEVEL_3 ? (map->level - 2) : BREAST_PUMP_LEVEL_1;
    } else {
        map->level = map->level > BREAST_PUMP_LEVEL_2 ? (map->level - 1) : BREAST_PUMP_LEVEL_1;
    }
    return 0;
}