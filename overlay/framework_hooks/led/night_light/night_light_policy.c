/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "night_light_policy.h"

#if IS_SUPPORT_NIGHT_LIGHT

/*
* GLOBAL FUNCTION DEFINITIONS — 仅档位/协议；
*****************************************************************************************
*/
uint8_t night_light_policy_default_gear(void)
{
    return NIGHT_LIGHT_GEAR_L2;
}

uint8_t night_light_policy_safety_gear(void)
{
    return NIGHT_LIGHT_GEAR_L2;
}

/// 亮度转换为档位
uint8_t night_light_policy_level_to_gear(uint8_t level)
{
    if (level == 0) {
        return NIGHT_LIGHT_GEAR_L0;
    }
    if (level <= NIGHT_LIGHT_LEVEL_GEAR_L1) {
        return NIGHT_LIGHT_GEAR_L1;
    }
    if (level <= NIGHT_LIGHT_LEVEL_GEAR_L2) {
        return NIGHT_LIGHT_GEAR_L2;
    }
    return NIGHT_LIGHT_GEAR_L3;
}

/// 档位转换为亮度
uint8_t night_light_policy_gear_to_level(uint8_t gear)
{
    switch (gear) {
        case NIGHT_LIGHT_GEAR_L1:
            return NIGHT_LIGHT_LEVEL_GEAR_L1;
        case NIGHT_LIGHT_GEAR_L2:
            return NIGHT_LIGHT_LEVEL_GEAR_L2;
        case NIGHT_LIGHT_GEAR_L3:
            return NIGHT_LIGHT_LEVEL_GEAR_L3;
        case NIGHT_LIGHT_GEAR_L0:
        default:
            return NIGHT_LIGHT_LEVEL_OFF;
    }
}

#endif /* IS_SUPPORT_NIGHT_LIGHT */
