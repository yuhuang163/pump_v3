#ifndef __POWER_MANAGER_HOOK_H__
#define __POWER_MANAGER_HOOK_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/**@brief Firmware information define*/
typedef enum {
    HDR_ACTION_POWER_ON_DETECT, /// 电源开机检测.
    HDR_ACTION_POWER_ON_INVALID, /// 电源开机无效.
    HDR_ACTION_POWER_ON,
    HDR_ACTION_POWER_ON_BY_CHARGING, /// 电源开机充电.
    HDR_ACTION_POWER_ON_GUIDE, /// V3 插电/短按引导长按开机（进 dummy）.
    HDR_ACTION_POWER_OFF, /// 电源关机.
    HDR_ACTION_SYS_STANDBY, /// 待机状态.
    HDR_ACTION_SYS_GO_AHEAD, /// 系统继续前进.
    HDR_ACTION_MAX
} power_manager_hook_action_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 
