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

static void pump_climb_apply_direct(pump_climb_policy_t *climb, suction_strategy_mapping_t *map, uint8_t target_level)
{
    map->level = target_level;
    climb->level = target_level;
    climb->target_level = target_level;
    climb->start_level = target_level;
    pump_climb_policy_reset(climb);
}

/**
 * 按 6 秒总时长与 period_ms 计算固定步进值，最小为 1。
 * step = max(1, ceil(总差值 * period_ms / 6000))
 */
static uint8_t pump_climb_get_step_size(uint8_t total_diff, uint32_t period_ms)
{
    uint32_t step;

    if (total_diff == 0) {
        return PUMP_CLIMB_MIN_STEP;
    }
    if (period_ms == 0) {
        return PUMP_CLIMB_MIN_STEP;
    }

    step = ((uint32_t)total_diff * period_ms + PUMP_CLIMB_TOTAL_TIME_MS - 1) / PUMP_CLIMB_TOTAL_TIME_MS;
    if (step < PUMP_CLIMB_MIN_STEP) {
        step = PUMP_CLIMB_MIN_STEP;
    }
    if (step > total_diff) {
        step = total_diff;
    }
    return (uint8_t)step;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void pump_climb_policy_mark_auto(pump_climb_policy_t *climb)
{
    /// 自动调档：清空状态，下一拍从低档开始爬升
    memset(climb, 0, sizeof(pump_climb_policy_t) * BREAST_PUMP_NUM);
}

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

    /// 降档或已到目标：直接落地
    if (diff <= 0) {
        pump_climb_apply_direct(climb, map, target_level);
        return 0;
    }

    /// 新目标：差值 ≤ 2 不启动爬升，直接落地
    if (target_level != climb->target_level) {
        if (diff <= (int)PUMP_CLIMB_START_DIFF) {
            pump_climb_apply_direct(climb, map, target_level);
            return 0;
        }
        climb->target_level = target_level;
        climb->count = 0;
        climb->start_level = current_level;
    }

    /// 超过最大步数，直接到达目标力度
    if (climb->count >= PUMP_CLIMB_MAX_COUNT) {
        pump_climb_apply_direct(climb, map, target_level);
        return 0;
    }

    uint8_t total_diff = climb->target_level > climb->start_level ?
        (climb->target_level - climb->start_level) : (uint8_t)diff;
    uint8_t step = pump_climb_get_step_size(total_diff, period_ms);
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
        pump_id, climb->count, PUMP_CLIMB_MAX_COUNT, step, new_level, target_level, period_ms);
    return 0;
}
