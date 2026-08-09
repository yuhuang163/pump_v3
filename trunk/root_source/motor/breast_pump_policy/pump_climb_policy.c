/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "breast_pump_policy.h"

#if IS_SUPPORT_CLIMB_POLICY

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
    climb->from_current = 0;
    climb->allow_down = 0;
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

static uint8_t pump_climb_get_cur_level(motor_policy_runtime_t *runtime, uint8_t pump_id)
{
    uint8_t cur = runtime->climb[pump_id].level;

    if (cur == 0 && runtime->pump_level[pump_id] > 0) {
        cur = runtime->pump_level[pump_id];
    }
    return cur;
}

static void pump_climb_mark_from_current(motor_policy_runtime_t *runtime, uint8_t allow_down)
{
    pump_climb_policy_t *climb = runtime->climb;
    uint8_t next_mode_type = SUCTION_MODE_IDLE;
    uint8_t is_short = 0;

    if (runtime->timed_policy && runtime->timed_policy->policy.core.type == MOTOR_PARAM_TABLE) {
        next_mode_type = runtime->timed_policy->policy.core.un.mapping.mode_type;
        is_short = (next_mode_type == SUCTION_MODE_SHORT || next_mode_type == SUCTION_MODE_MIX_SHORT);
    }

    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        uint8_t cur = pump_climb_get_cur_level(runtime, i);

        /// 方案4：切到刺激时，起点不超过刺激上限 5
        if (allow_down && is_short && cur > BREAST_PUMP_LEVEL_5) {
            cur = BREAST_PUMP_LEVEL_5;
        }

        climb[i].count = 0;
        climb[i].target_level = 0; /// 强制按新目标重新决策
        climb[i].level = cur;
        climb[i].start_level = cur;
        climb[i].from_current = 1;
        climb[i].allow_down = allow_down;
        LOG_MOTOR("rhythm climb: from cur pump:%d level:%d allow_down:%d short:%d",
            i, cur, allow_down, is_short);
    }
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void pump_climb_policy_mark_auto(pump_climb_policy_t *climb)
{
    /// 启停/手动切模式：清空状态，下一拍从低档开始爬升
    memset(climb, 0, sizeof(pump_climb_policy_t) * BREAST_PUMP_NUM);
}

void pump_climb_policy_mark_rhythm_switch(motor_policy_runtime_t *runtime)
{
    pump_climb_policy_t *climb = runtime->climb;

#if PUMP_CLIMB_RHYTHM_SWITCH_POLICY == PUMP_CLIMB_RHYTHM_SWITCH_NONE
    /// 方案1：不缓冲，下一拍直接落到新目标
    for (uint8_t i = 0; i < BREAST_PUMP_NUM; i++) {
        climb[i].count = 0;
        climb[i].target_level = 0;
        climb[i].level = 0xFF;
        climb[i].start_level = 0xFF;
        climb[i].from_current = 0;
        climb[i].allow_down = 0;
    }
    LOG_MOTOR("rhythm climb: none");
#elif PUMP_CLIMB_RHYTHM_SWITCH_POLICY == PUMP_CLIMB_RHYTHM_SWITCH_FROM_1
    /// 方案2：从 1 开始缓冲
    memset(climb, 0, sizeof(pump_climb_policy_t) * BREAST_PUMP_NUM);
    LOG_MOTOR("rhythm climb: from 1");
#elif PUMP_CLIMB_RHYTHM_SWITCH_POLICY == PUMP_CLIMB_RHYTHM_SWITCH_FROM_CUR
    /// 方案3：从当前实际档位继续；升档爬升，降档直落
    pump_climb_mark_from_current(runtime, 0);
#else
    /// 方案4：同方案3，且吸乳高档→刺激时从 5 向下缓冲到目标
    pump_climb_mark_from_current(runtime, 1);
#endif
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

    /// 已到目标
    if (diff == 0) {
        pump_climb_apply_direct(climb, map, target_level);
        return 0;
    }

    /// 降档：默认直落；方案4（allow_down）时大跨度向下缓冲
    if (diff < 0) {
        if (!climb->allow_down) {
            pump_climb_apply_direct(climb, map, target_level);
            return 0;
        }

        uint8_t abs_diff = (uint8_t)(-diff);
        if (target_level != climb->target_level) {
            /// 小幅降档仍直接落地
            if (abs_diff <= PUMP_CLIMB_START_DIFF) {
                pump_climb_apply_direct(climb, map, target_level);
                return 0;
            }
            climb->target_level = target_level;
            climb->count = 0;
            climb->start_level = current_level;
        }

        if (climb->count >= PUMP_CLIMB_MAX_COUNT) {
            pump_climb_apply_direct(climb, map, target_level);
            return 0;
        }

        uint8_t total_diff = climb->start_level > climb->target_level ?
            (climb->start_level - climb->target_level) : abs_diff;
        uint8_t step = pump_climb_get_step_size(total_diff, period_ms);
        uint8_t new_level = (current_level > step) ? (current_level - step) : target_level;

        if (new_level < target_level) {
            new_level = target_level;
        }

        climb->count++;
        map->level = new_level;
        climb->level = new_level;

        if (new_level <= target_level) {
            pump_climb_policy_reset(climb);
        }

        LOG_MOTOR("climb down pump:%d, count:%d/%d, step:%d, level:%d, target:%d, period:%d",
            pump_id, climb->count, PUMP_CLIMB_MAX_COUNT, step, new_level, target_level, period_ms);
        return 0;
    }

    /// 升档：普通路径差值 ≤ 2 不爬升；韵律续爬（from_current）则升档即爬
    if (target_level != climb->target_level) {
        uint8_t start_diff = climb->from_current ? 0 : PUMP_CLIMB_START_DIFF;
        if (diff <= (int)start_diff) {
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

    LOG_MOTOR("climb pump:%d, count:%d/%d, step:%d, level:%d, target:%d, period:%d, from_cur:%d",
        pump_id, climb->count, PUMP_CLIMB_MAX_COUNT, step, new_level, target_level, period_ms,
        climb->from_current);
    return 0;
}

#endif /* IS_SUPPORT_CLIMB_POLICY */
