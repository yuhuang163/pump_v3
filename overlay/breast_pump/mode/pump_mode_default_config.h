#ifndef __PUMP_MODE_DEFAULT_CONFIG_H__
#define __PUMP_MODE_DEFAULT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "project_default_features.h"
#include "motor_policy.h"
#include "persist_storage.h"
#include "breast_pump_manager_api.h" // IWYU pragma: keep

#ifndef REGULAR_PUMPING_MODE_LEN
#error "REGULAR_PUMPING_MODE_LEN not defined. Check project_default_features.h include."
#endif

/// 支持泵奶策略快速测试
#define IS_SUPPORT_PUMP_MODE_FAST_TEST               (0)

#if (IS_SUPPORT_PUMP_MODE_FAST_TEST == 0)
/// 常规模式默认工作时长，单位：秒
#define NORMAL_MODE_DEFAULT_WORK_TIME                (SETTING_PUMP_TIME_MAX / 1000)

/// 个性化模式按摩默认时长2分钟，单位：秒
#define CUSTOM_MODE_DEFAULT_MASSAGE_TIME             120  
/// 个性化模式吸乳默认时长28分钟，单位：秒
#define CUSTOM_MODE_DEFAULT_PUMP_TIME                1680  

/// 2+8奶量提升模式持续工作20分钟，单位：秒
#define MILK_BOOST_MODE_WORK_TIME                    1200 
/// 2+8奶量提升模式按摩默认时长2分钟，单位：秒
#define MILK_BOOST_MODE_DEFAULT_MASSAGE_TIME         120  
/// 2+8奶量提升模式吸乳默认时长8分钟，单位：秒
#define MILK_BOOST_MODE_DEFAULT_PUMP_TIME            480

/// 常规韵律模式持续工作21.5分钟，单位：秒
#define REGULAR_PUMPING_MODE_WORK_TIME               1290
/// 常规韵律模式刺激时长1.5分钟，单位：秒
#define REGULAR_PUMPING_MODE_STIMULATION_TIME        90
/// 常规韵律模式吸乳时长20分钟，单位：秒
#define REGULAR_PUMPING_MODE_PUMP_TIME               1200

/// 追加排空模式持续工作22分钟，单位：秒
#define EMPTYING_BOOST_MODE_WORK_TIME                1320
/// 追加排空模式刺激时长2分钟，单位：秒
#define EMPTYING_BOOST_MODE_STIMULATION_TIME         120
/// 追加排空模式吸乳时长15分钟，单位：秒
#define EMPTYING_BOOST_MODE_PUMP_TIME                900
/// 追加排空模式混合时长5分钟，单位：秒
#define EMPTYING_BOOST_MODE_MIX_TIME                 300 

/// 开奶模式（Lactation Kickstart）持续工作17分钟，单位：秒
#define LACTATION_START_MODE_WORK_TIME               1020
/// 开奶模式刺激时长3分钟，单位：秒
#define LACTATION_START_MODE_MASSAGE_TIME_3MIN       180
/// 开奶模式混合1时长2分钟，单位：秒
#define LACTATION_START_MODE_MASSAGE_TIME_2MIN       120
/// 开奶模式吸乳时长1分钟，单位：秒
#define LACTATION_START_MODE_PUMP_TIME_1MIN          60
/// 开奶模式吸乳时长2分钟，单位：秒
#define LACTATION_START_MODE_PUMP_TIME_2MIN          120

/// 追奶模式默认工作时长45分钟，单位：秒
#define POWER_PUMPING_MODE_WORK_TIME                 2700
/// Power Pumping模式刺激时长2分钟，单位：秒
#define POWER_PUMPING_MODE_MASSAGE_TIME_2MIN         120
/// Power Pumping模式泌乳时长13分钟，单位：秒
#define POWER_PUMPING_MODE_PUMP_TIME_13MIN           780
/// Power Pumping模式泌乳时长8分钟，单位：秒
#define POWER_PUMPING_MODE_PUMP_TIME_8MIN            480
/// Power Pumping模式暂停时长5分钟，单位：秒
#define POWER_PUMPING_MODE_PAUSE_TIME_5MIN           300

/// Relax模式默认工作时长10分钟，单位：秒
#define RELAX_MODE_WORK_TIME                         600
#else
/// 常规模式默认工作时长，单位：秒
#define NORMAL_MODE_DEFAULT_WORK_TIME               (60)

/// 个性化模式按摩默认时长2分钟，单位：秒
#define CUSTOM_MODE_DEFAULT_MASSAGE_TIME             12  
/// 个性化模式吸乳默认时长28分钟，单位：秒
#define CUSTOM_MODE_DEFAULT_PUMP_TIME                168  

/// 2+8奶量提升模式持续工作20分钟，单位：秒
#define MILK_BOOST_MODE_WORK_TIME                    120 
/// 2+8奶量提升模式按摩默认时长2分钟，单位：秒
#define MILK_BOOST_MODE_DEFAULT_MASSAGE_TIME         12  
/// 2+8奶量提升模式吸乳默认时长8分钟，单位：秒
#define MILK_BOOST_MODE_DEFAULT_PUMP_TIME            48

/// 常规韵律模式持续工作21.5分钟，单位：秒
#define REGULAR_PUMPING_MODE_WORK_TIME               129
/// 常规韵律模式刺激时长1.5分钟，单位：秒
#define REGULAR_PUMPING_MODE_STIMULATION_TIME        9
/// 常规韵律模式吸乳时长20分钟，单位：秒
#define REGULAR_PUMPING_MODE_PUMP_TIME               120

/// 追加排空模式持续工作22分钟，单位：秒
#define EMPTYING_BOOST_MODE_WORK_TIME                132
/// 追加排空模式刺激时长2分钟，单位：秒
#define EMPTYING_BOOST_MODE_STIMULATION_TIME         12
/// 追加排空模式吸乳时长15分钟，单位：秒
#define EMPTYING_BOOST_MODE_PUMP_TIME                90
/// 追加排空模式混合时长5分钟，单位：秒
#define EMPTYING_BOOST_MODE_MIX_TIME                 30 

/// 开奶模式（Lactation Kickstart）持续工作17分钟，单位：秒
#define LACTATION_START_MODE_WORK_TIME               102
/// 开奶模式刺激时长3分钟，单位：秒
#define LACTATION_START_MODE_MASSAGE_TIME_3MIN       18
/// 开奶模式混合1时长2分钟，单位：秒
#define LACTATION_START_MODE_MASSAGE_TIME_2MIN       12
/// 开奶模式吸乳时长1分钟，单位：秒
#define LACTATION_START_MODE_PUMP_TIME_1MIN          6
/// 开奶模式吸乳时长2分钟，单位：秒
#define LACTATION_START_MODE_PUMP_TIME_2MIN          12

/// 追奶模式默认工作时长45分钟，单位：秒
#define POWER_PUMPING_MODE_WORK_TIME                 270
/// Power Pumping模式刺激时长2分钟，单位：秒
#define POWER_PUMPING_MODE_MASSAGE_TIME_2MIN         12
/// Power Pumping模式泌乳时长13分钟，单位：秒
#define POWER_PUMPING_MODE_PUMP_TIME_13MIN           78
/// Power Pumping模式泌乳时长8分钟，单位：秒
#define POWER_PUMPING_MODE_PUMP_TIME_8MIN            48
/// Power Pumping模式暂停时长5分钟，单位：秒
#define POWER_PUMPING_MODE_PAUSE_TIME_5MIN           30
/// Relax模式默认工作时长10分钟，单位：秒
#define RELAX_MODE_WORK_TIME                         60
#endif

/// 无效工作时长，子模式不需要指定工作时长
#define INVALID_WORK_TIME                            0  

/// 吸乳模式默认周期 单位：毫秒
#define PUMP_MODE_DEFAULT_PERIOD                     90 

/// 自定义模式策略缓存最大32个
#define CUSTOM_MODE_MOTOR_POLICY_NUMBER              32

/// 刺激
static const motor_policy_cfg_t stimulation_default[STIMULATION_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 吸乳
static const motor_policy_cfg_t pump_default[PUMP_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 混合1
static const motor_policy_cfg_t mix_default[MIX_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_SHORT},
        {5, PUMP_MODE_DEFAULT_PERIOD}}
       },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
       },
};

/// 混合2
static const motor_policy_cfg_t mix_default2[MIX2_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_SHORT},
        {2, PUMP_MODE_DEFAULT_PERIOD}}
       },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
       },
};

/// 3 cozy flow 常规韵律（默认）：由高频轻吸变为低频深吸，模仿婴儿自然的吸吮动作
/// 21.5 min：刺激 1.5min + 吸乳 20min
static const motor_policy_cfg_t regular_pumping_default[REGULAR_PUMPING_MODE_LEN] = {
    {MOTOR_TIME_POLICY, REGULAR_PUMPING_MODE_STIMULATION_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, REGULAR_PUMPING_MODE_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 4 Milk Boost 高效泌乳：Most suitable for daily use after successful lactation
/// ≈ 20 min：刺激 2min + 吸乳 8min + 刺激 2min + 吸乳 8min
static const motor_policy_cfg_t milk_boost_default[MILK_BOOST_MODE_LEN] = {
    {MOTOR_TIME_POLICY, MILK_BOOST_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_BOOST_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_BOOST_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_BOOST_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 5 Power Pumping 密集追奶：Increase milk supply, no more than once a day
/// ≈ 45 min：刺激 2min + 吸乳 13min + 暂停 5min + 刺激 2min + 吸乳 8min + 暂停 5min + 刺激 2min + 吸乳 8min
static const motor_policy_cfg_t pp_milk_track_default[PP_MODE_LEN] = {
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_MASSAGE_TIME_2MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_PUMP_TIME_13MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_PAUSE_TIME_5MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_0, SUCTION_MODE_IDLE},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_MASSAGE_TIME_2MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_PUMP_TIME_8MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_PAUSE_TIME_5MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_0, SUCTION_MODE_IDLE},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_MASSAGE_TIME_2MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, POWER_PUMPING_MODE_PUMP_TIME_8MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 6 Lactation Kickstart 启动泌乳：启动、建立和维持充足的乳汁分泌
/// 17 min：刺激（中频 1档）3min + 混合1（中频 1档）2min + 吸乳（中频 1档）1min
///       + 刺激 3min + 混合1 2min + 吸乳 1min + 刺激 3min + 吸乳 2min
static const motor_policy_cfg_t lactation_start_default[LACTATION_START_MODE_LEN] = {
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_MASSAGE_TIME_3MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_MASSAGE_TIME_2MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_SHORT},
        {5, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_PUMP_TIME_1MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_MASSAGE_TIME_3MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_MASSAGE_TIME_2MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_SHORT},
        {5, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_PUMP_TIME_1MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_MASSAGE_TIME_3MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_PUMP_TIME_2MIN, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 7 Emptying Boost 追加排空：有效将后乳（Hindmilk）排空，更安心投入后续生活
/// 22 min：刺激 2min+吸乳 5min+混合 15min；Level 3
static const motor_policy_cfg_t emptying_boost_default[EMPTYING_BOOST_MODE_LEN] = {
    {MOTOR_TIME_POLICY, EMPTYING_BOOST_MODE_STIMULATION_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, EMPTYING_BOOST_MODE_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, EMPTYING_BOOST_MODE_MIX_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_SHORT},
        {5, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_MIX_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },   
};

/// 8 个性化默认
static const motor_policy_cfg_t custom_default[CUSTOM_MODE_LEN] = {
    {MOTOR_TIME_POLICY, CUSTOM_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, CUSTOM_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 9 个性化试用模式
static const motor_policy_cfg_t custom_trial_default[CUSTOM_TRIAL_MODE_LEN] = {
    {MOTOR_TIME_POLICY, CUSTOM_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, CUSTOM_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_1, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// Relax 放松模式：0档休息模式，帮助放松和恢复
/// 10 min：休息 10min（0档）
static const motor_policy_cfg_t relax_default[RELAX_MODE_LEN] = {
    {MOTOR_TIME_POLICY, RELAX_MODE_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_0, SUCTION_MODE_IDLE},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

#ifdef __cplusplus
}
#endif

#endif 
