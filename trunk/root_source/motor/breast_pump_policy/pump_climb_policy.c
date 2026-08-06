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
#define PUMP_CLIMB_MAX_COUNT        10
#define PUMP_CLIMB_TOTAL_TIME_MS    6000
#define PUMP_CLIMB_START_DIFF       2
#define PUMP_CLIMB_MIN_STEP         1

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
static void pump_climb_policy_reset(pump_climb_policy_t *climb)
{
    climb->count = 0;
}

static uint8_t pump_climb_get_max_steps(uint32_t period_ms)
{
    uint32_t max_steps;

    if (period_ms == 0) {
        return PUMP_CLIMB_MAX_COUNT;
    }

    max_steps = PUMP_CLIMB_TOTAL_TIME_MS / period_ms;
    if (max_steps < 1) {
        max_steps = 1;
    }
    if (max_steps > PUMP_CLIMB_MAX_COUNT) {
        max_steps = PUMP_CLIMB_MAX_COUNT;
    }
    return (uint8_t)max_steps;
}

static uint8_t pump_climb_get_step_size(uint8_t remaining, uint8_t steps_left)
{
    uint8_t step;

    if (steps_left == 0) {
        return remaining;
    }

    step = (remaining + steps_left - 1) / steps_left;
    if (step < PUMP_CLIMB_MIN_STEP) {
        step = PUMP_CLIMB_MIN_STEP;
    }
    return step;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int pump_climb_policy_update(struct motor_manager_context *thiz, suction_strategy_mapping_t *map,
    uint8_t pump_id, uint32_t period_ms)
{
    pump_climb_policy_t *climb = &thiz->priv.runtime.climb[pump_id];
    uint8_t target_level = map->level;
    uint8_t current_level = climb->level;
    int diff = (int)target_level - (int)current_level;

    if (target_level == 0) {
        memset(climb, 0, sizeof(*climb));
        return 0;
    }

    /// 力度降低或已接近目标，直接应用目标力度
    if (diff <= (int)PUMP_CLIMB_START_DIFF) {
        map->level = target_level;
        climb->level = target_level;
        climb->target_level = target_level;
        pump_climb_policy_reset(climb);
        return 0;
    }

    /// 实际目标力度变化时，重置爬升计数
    if (target_level != climb->target_level) {
        climb->target_level = target_level;
        climb->count = 0;
        climb->start_level = climb->level;
    }

    uint8_t max_steps = pump_climb_get_max_steps(period_ms);

    /// 超过最大步数，直接到达目标力度
    if (climb->count >= max_steps) {
        map->level = target_level;
        climb->level = target_level;
        pump_climb_policy_reset(climb);
        return 0;
    }

    uint8_t remaining = (uint8_t)diff;
    uint8_t steps_left = max_steps - climb->count;
    uint8_t step = pump_climb_get_step_size(remaining, steps_left);
    uint8_t new_level = current_level + step;

    if (new_level > target_level) {
        new_level = target_level;
    }

    climb->count++;
    map->level = new_level;
    climb->level = new_level;

    /// 到达目标力度后重置爬升计数
    if (new_level >= target_level) {
        pump_climb_policy_reset(climb);
    }

    LOG_MOTOR("climb pump:%d, count:%d/%d, step:%d, level:%d, target:%d, period:%d",
        pump_id, climb->count, max_steps, step, new_level, target_level, period_ms);
    return 0;
}
