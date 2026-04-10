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

    /* 通用组 [1..100] */
    RESOURCE_ID_BLE_CONNECT_ICON = 0x01,        /// 蓝牙连接图标
    RESOURCE_ID_BLE_DISCONNECT_ICON,            /// 蓝牙断开图标
    RESOURCE_ID_BLE_DISCONNECT_POPUP_ICON,      /// 蓝牙断开弹窗图标
    RESOURCE_ID_ALLOW_ICON,                     /// 允许图标
    RESOURCE_ID_BAN_ICON,                       /// 禁止图标
    RESOURCE_ID_WARNING_ICON,                   /// 警告图标

    /* 主界面组 [100..150] */
    RESOURCE_ID_COMMON_BG = 100,               /// 通用背景
    RESOURCE_ID_START_LOGO_ICON,               /// 启动LOGO
    RESOURCE_ID_HOME_IDLE_LABLE_ICON,          /// 空闲界面文本区域图标
    RESOURCE_ID_HOME_IDLE_BUTTOM_ICON,         /// 空闲界面按钮指示图标
    RESOURCE_ID_AIRPLANE_UNLOCK_BUTTON_ICON,   /// 飞行模式解锁按钮图标
    RESOURCE_ID_HOME_DUMMY_ICON,               /// dummy图标
    RESOURCE_ID_CHARGE_ADAPTER_IMG,            /// 首页充电适配器图标
    RESOURCE_ID_HOME_BLE_LABEL_ICON,           /// 首页蓝牙标签图标
    RESOURCE_ID_HOME_BLE_IP_ICON,              /// 首页蓝牙IP图标

    /* 引导组 [150..200] */
    RESOURCE_ID_GUIDE_PAGE1_IMG = 150,        /// 引导-页1图片
    RESOURCE_ID_GUIDE_PAGE2_IMG,              /// 引导-页2图片
    RESOURCE_ID_GUIDE_PAGE3_IMG,              /// 引导-页3图片
    RESOURCE_ID_GUIDE_PAGE4_IMG,              /// 引导-页4图片
    RESOURCE_ID_GUIDE_PAGE5_IMG,              /// 引导-页5图片
    RESOURCE_ID_GUIDE_PAGE6_IMG,              /// 引导-页6图片

    /* 模式相关组 [301..400]（大图 + 小图标） */
    RESOURCE_ID_PUMP_SPEED_SLOW_ICON = 200,    /// 泵奶首页-慢速图标
    RESOURCE_ID_PUMP_SPEED_REGULAR_ICON,       /// 泵奶首页-常规图标
    RESOURCE_ID_PUMP_SPEED_BOOST_ICON,         /// 泵奶首页-快速图标
    RESOURCE_ID_REGULAR_PUMPING_BG,            /// 泵奶首页-常规泵奶背景
    RESOURCE_ID_EXPERT_PUMPING_BG,             /// 泵奶首页-专家泵奶背景

    RESOURCE_ID_FREQ_PAGE_BOOST_ICON,          /// 频率页-频率增强图标
    RESOURCE_ID_FREQ_PAGE_REGULAR_ICON,        /// 频率页-频率常规图标
    RESOURCE_ID_FREQ_PAGE_COMFORT_ICON,        /// 频率页-频率舒适图标

    RESOURCE_ID_LAST_USED_MODE_ICON,           /// 上一次使用模式图标
    RESOURCE_ID_NEW_MODE_STEPS_PROMPT_ICON,    /// 新模式未确认图标
    RESOURCE_ID_MODE_LACTATION_ICON,           /// 模式-泌乳图标
    RESOURCE_ID_MODE_MIX_ICON,                 /// 模式-混合图标
    RESOURCE_ID_MODE_STIMULATE_ICON,           /// 模式-刺激图标

    RESOURCE_ID_MODE_CUSTOM_ICON,              /// 韵律-专家模式图标
    RESOURCE_ID_MODE_EXPERT_ICON,              /// 韵律-个性化模式图标

    RESOURCE_ID_PUMP_FINISH_ICON,              /// 泵奶完成图标
    RESOURCE_ID_PUMP_PAUSE_PROMPT_ICON,        /// 泵奶暂停提示图标
    RESOURCE_ID_PUMP_LEVEL_BG,                 /// 泵奶档位背景
    RESOURCE_ID_PUMP_LEVEL_BOTH_SIDES_BG,      /// 泵奶档位双侧模式背景
    RESOURCE_ID_PUMP_LEVEL_ADD_LABEL_ICON,     /// 泵奶档位文本加号图标
    RESOURCE_ID_PUMP_LEVEL_SUB_LABEL_ICON,     /// 泵奶档位文本减号图标

    /* 动画组 [300...] */
    RESOURCE_ID_COMMON_MODE_ANIMATION_START = 300,   /// 泵奶模式动画-开始
    RESOURCE_ID_COMMON_MODE_ANIMATION_END = 414,     /// 泵奶模式动画-结束

    RESOURCE_ID_PERSONALIZED_MODE_ANIMATION_START = 500,   /// 个性化模式动画-开始
    RESOURCE_ID_PERSONALIZED_MODE_ANIMATION_END = 619,     /// 泵奶模式动画-结束

    RESOURCE_ID_HOME_DEFAULT_ANIMATION_START = 700,   /// 首页默认动画-开始
    RESOURCE_ID_HOME_DEFAULT_ANIMATION_END = 819,     /// 首页默认动画-结束

    RESOURCE_ID_PUMP_LEVEL_START = 900,   /// 泵奶档位图片-开始
    RESOURCE_ID_PUMP_LEVEL_END = 914,     /// 泵奶档位图片-结束

    RESOURCE_ID_SELECTED_LOCATE_3_START = 920,     /// 选中定位-3-图标 
    RESOURCE_ID_SELECTED_LOCATE_3_END = 922,       /// 选中定位-3-图标

    RESOURCE_ID_SELECTED_LOCATE_5_START = 930,     /// 选中定位-5-图标 
    RESOURCE_ID_SELECTED_LOCATE_5_END = 934,       /// 选中定位-5-图标

    RESOURCE_ID_SELECTED_LOCATE_6_START = 940,     /// 选中定位-6-图标 
    RESOURCE_ID_SELECTED_LOCATE_6_END = 945,       /// 选中定位-6-图标

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


