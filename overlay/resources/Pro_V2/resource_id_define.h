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
    RESOURCE_ID_PUMP_TIME_COLON,             /// 首页时间冒号

    RESOURCE_ID_CHARGE_ADAPTER_IMG,         /// 首页充电适配器图标
    RESOURCE_ID_HOME_DUMMY_ICON,                  /// dummy图标

    RESOURCE_ID_HOME_IDLE_LABLE_ICON,           /// 空闲界面文本区域图标
    RESOURCE_ID_HOME_IDLE_BUTTOM_ICON,              /// 空闲界面按钮指示图标

    RESOURCE_ID_AIRPLANE_UNLOCK_BUTTON_ICON,   /// 飞行模式解锁按钮图标

    RESOURCE_ID_PUMP_SPEED_SLOW_ICON = 101,    /// 首页-慢速图标
    RESOURCE_ID_PUMP_SPEED_REGULAR_ICON,       /// 首页-常规图标
    RESOURCE_ID_PUMP_SPEED_BOOST_ICON,         /// 首页-快速图标
    RESOURCE_ID_PUMP_DASHED_ARC_ICON,          /// 首页-虚线弧线图标
   
    RESOURCE_ID_COMMON_BG = 130,            /// 泵奶首页背景

    RESOURCE_ID_START_LOGO_ICON = 140,         /// 启动LOGO

    /* 模式组 [301..400]（大图 + 小图标） */
    RESOURCE_ID_INITIATING_MODE = 201,        /// 模式-启动界面
    RESOURCE_ID_MODE_LACTATION_ICON,          /// 模式-泌乳图标
    RESOURCE_ID_MODE_MIX_ICON,                /// 模式-混合图标
    RESOURCE_ID_MODE_STIMULATE_ICON,          /// 模式-刺激图标
    RESOURCE_ID_FREQ_BOOST_ICON,              /// 频率增强图标
    RESOURCE_ID_FREQ_REGULAR_ICON,            /// 频率常规图标
    RESOURCE_ID_FREQ_COMFORT_ICON,            /// 频率舒适图标

    RESOURCE_ID_MODE_STIMULATION_PROMPT_ICON,  /// 模式-刺激确认提示图标
    RESOURCE_ID_MODE_EXPRESSION_PROMPT_ICON,    /// 模式-泌乳确认提示图标
    RESOURCE_ID_MODE_MIX_PROMPT_ICON,          /// 模式-混合确认提示图标
    RESOURCE_ID_MODE_RELAX_PROMPT_ICON,         /// 模式-放松确认提示图标

    RESOURCE_ID_PUMP_FINISH_ICON,               /// 泵奶完成图标
    RESOURCE_ID_PUMP_PAUSE_PROMPT_ICON,         /// 泵奶暂停提示图标
    RESOURCE_ID_PUMP_LEVEL_BG,     /// 泵奶档位背景图标

    RESOURCE_ID_GUIDE_QR_ICON = 250,
    RESOURCE_ID_PUMP_SETTING_GUIDE_ICON,
    RESOURCE_ID_PLAY_PAUSE_GUIDE_ICON,
    RESOURCE_ID_MODE_SEL_GUIDE_ICON, /// Mode+Star
    RESOURCE_ID_ARROW_GUIDE_ICON,
    RESOURCE_ID_EXPERT_PUMP_GUIDE_ICON,
    RESOURCE_ID_PUMP_START_GUIDE_ICON,
    RESOURCE_ID_KNOB_GUIDE_0,
    RESOURCE_ID_KNOB_GUIDE_1,

    RESOURCE_ID_EMODE_R_ICON,
    RESOURCE_ID_EMODE_L_ICON,
    RESOURCE_ID_CMODE_R_ICON,
    RESOURCE_ID_CMODE_L_ICON,
    RESOURCE_ID_STAR_ICON,
    RESOURCE_ID_HUE_ICON,
    RESOURCE_ID_HEART_ICON,
    RESOURCE_ID_EXPRESSION_ICON,
    RESOURCE_ID_MODE_CENTER_BG,
    RESOURCE_ID_MODE_VECTOR_ICON,
    RESOURCE_ID_COMMON_MODE_ANIMATION_START = 300,   /// 泵奶模式动画-开始
    RESOURCE_ID_COMMON_MODE_ANIMATION_END = 414,     /// 泵奶模式动画-结束

    RESOURCE_ID_PERSONALIZED_MODE_ANIMATION_START = 500,   /// 个性化模式动画-开始
    RESOURCE_ID_PERSONALIZED_MODE_ANIMATION_END = 619,     /// 泵奶模式动画-结束

    RESOURCE_ID_HOME_DEFAULT_ANIMATION_START = 700,   /// 首页默认动画-开始
    RESOURCE_ID_HOME_DEFAULT_ANIMATION_END = 819,     /// 首页默认动画-结束

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


