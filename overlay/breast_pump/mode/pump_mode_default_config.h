#ifndef __PUMP_MODE_DEFAULT_CONFIG_H__
#define __PUMP_MODE_DEFAULT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "motor_policy.h"
#include "persist_storage.h"

/// 常规模式默认工作时长30分钟，单位：秒
#define NORMAL_MODE_DEFAULT_WORK_TIME                1800
/// 无效工作时长，子模式不需要指定工作时长
#define INVALID_WORK_TIME                            0  
/// 吸乳模式默认周期 单位：毫秒
#define PUMP_MODE_DEFAULT_PERIOD                     90 

/// 个性化模式按摩默认时长2分钟，单位：秒
#define CUSTOM_MODE_DEFAULT_MASSAGE_TIME             120  
/// 个性化模式吸乳默认时长28分钟，单位：秒
#define CUSTOM_MODE_DEFAULT_PUMP_TIME                1680  

/// 2+8奶量提升模式持续工作20分钟，单位：秒
#define MILK_BOOST_MODE_WORK_TIME                    1800 
/// 2+8奶量提升模式按摩默认时长2分钟，单位：秒
#define MILK_BOOST_MODE_DEFAULT_MASSAGE_TIME         120  
/// 2+8奶量提升模式吸乳默认时长8分钟，单位：秒
#define MILK_BOOST_MODE_DEFAULT_PUMP_TIME            480 

/// 开奶模式持续工作30分钟，单位：秒
#define LACTATION_START_MODE_WORK_TIME               1800 
/// 开奶模式按摩默认时长2分钟，单位：秒
#define LACTATION_START_MODE_DEFAULT_MASSAGE_TIME    120  
/// 开奶模式吸乳默认时长8分钟，单位：秒
#define LACTATION_START_MODE_DEFAULT_PUMP_TIME       480 

/// 追奶模式吸乳时长20分钟，单位：秒
#define MILK_TRACK_MODE_PUMP_TIME_0                  1200 
/// 追奶模式吸乳时长10分钟，单位：秒
#define MILK_TRACK_MODE_PUMP_TIME_1                  600  
/// 追奶模式休息时长10分钟，单位：秒
#define MILK_TRACK_MODE_IDLE_TIME                    600  

/// 自定义模式策略缓存最大32个
#define CUSTOM_MODE_MOTOR_POLICY_NUMBER              32

/// 刺激
static const motor_policy_cfg_t stimulation_default[STIMULATION_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_6, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 吸乳
static const motor_policy_cfg_t pump_default[PUMP_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_9, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 混合1
static const motor_policy_cfg_t mix_default[MIX_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_6, SUCTION_MODE_SHORT},
        {5, PUMP_MODE_DEFAULT_PERIOD}}
       },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_6, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
       },
};

/// 混合2
static const motor_policy_cfg_t mix_default2[MIX2_MODE_LEN] = {
    {MOTOR_TIME_POLICY, NORMAL_MODE_DEFAULT_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_6, SUCTION_MODE_SHORT},
        {2, PUMP_MODE_DEFAULT_PERIOD}}
       },
    {MOTOR_TIME_SUB_POLICY, INVALID_WORK_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_6, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
       },
};

/// 个性化默认
static const motor_policy_cfg_t custom_default[CUSTOM_MODE_LEN] = {
    {MOTOR_TIME_POLICY, CUSTOM_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_6, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, CUSTOM_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_LOW, BREAST_PUMP_LEVEL_9, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// PP追奶
static const motor_policy_cfg_t pp_milk_track_default[PP_MODE_LEN] = {
    {MOTOR_TIME_POLICY, MILK_TRACK_MODE_PUMP_TIME_0, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_9, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_TRACK_MODE_IDLE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_7, SUCTION_MODE_IDLE},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_TRACK_MODE_PUMP_TIME_1, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_6, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_TRACK_MODE_IDLE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_5, SUCTION_MODE_IDLE},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_TRACK_MODE_PUMP_TIME_1, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_9, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },    
};

/// 2+8奶量提升
static const motor_policy_cfg_t milk_boost_default[MILK_BOOST_MODE_LEN] = {
    {MOTOR_TIME_POLICY, MILK_BOOST_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_6, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, MILK_BOOST_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_HIGH, BREAST_PUMP_LEVEL_9, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

/// 开奶
static const motor_policy_cfg_t lactation_start_default[LACTATION_START_MODE_LEN] = {
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_DEFAULT_MASSAGE_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_6, SUCTION_MODE_SHORT},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
    {MOTOR_TIME_POLICY, LACTATION_START_MODE_DEFAULT_PUMP_TIME, {
        MOTOR_PARAM_TABLE, {SUCTION_SPEED_MID, BREAST_PUMP_LEVEL_9, SUCTION_MODE_LONG},
        {1, PUMP_MODE_DEFAULT_PERIOD}}
        },
};

#ifdef __cplusplus
}
#endif

#endif 
