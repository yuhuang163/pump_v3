#ifndef __RESOURCE_ID_DEFINE_H__
#define __RESOURCE_ID_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdint.h> // IWYU pragma: keep

typedef enum {
    RESOURCE_ID_NONE = 0x00, /// 无资源

    RESOURCE_ID_BLE_CONNECT = 0x01,          /// 蓝牙连接图标
    RESOURCE_ID_BLE_DISCONNECT,              /// 蓝牙断开图标

    RESOURCE_ID_HOME_BATTERY_0 = 10,         /// 首页电池电量0%
    RESOURCE_ID_HOME_BATTERY_20,             /// 首页电池电量20%
    RESOURCE_ID_HOME_BATTERY_40,             /// 首页电池电量40%
    RESOURCE_ID_HOME_BATTERY_60,             /// 首页电池电量60%
    RESOURCE_ID_HOME_BATTERY_80,             /// 首页电池电量80%
    RESOURCE_ID_HOME_BATTERY_100,            /// 首页电池电量100%

    RESOURCE_ID_HOME_CHARGING,               /// 首页充电图标
    RESOURCE_ID_HOME_UNCHARGE,               /// 首页未充电图标
    RESOURCE_ID_HOME_TIME_COLON,             /// 首页时间冒号

    RESOURCE_ID_HOME_STIMULATE_ICON = 101,   /// 首页-刺激图标
    RESOURCE_ID_HOME_LACTATION_ICON,         /// 首页-泌乳图标
    RESOURCE_ID_HOME_MIX_ICON,               /// 首页-混合图标
    RESOURCE_ID_HOME_PERSONALIZATION_ICON,   /// 首页-个性化图标
    RESOURCE_ID_HOME_PP_ICON,                /// 首页-PP图标
    RESOURCE_ID_HOME_MILK_ICON,              /// 首页-奶量图标
    RESOURCE_ID_HOME_INITIA_ICON,            /// 首页-开奶图标

    RESOURCE_ID_HOME_SPEED_ICON,             /// 首页-速度图标
   
    RESOURCE_ID_PUMP_HOME_BG = 130,          /// 泵奶首页背景

    RESOURCE_ID_PUMP_MODE_ANIMATION_START = 300,   /// 泵奶模式动画-开始

    RESOURCE_ID_PUMP_MODE_ANIMATION_END = 449,     /// 泵奶模式动画-结束

    RESOURCE_ID_TYPE_MAX
} resource_id_type_e;


/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


