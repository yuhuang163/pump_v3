#ifndef __RESOURCE_ID_DEFINE_H__
#define __RESOURCE_ID_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/

typedef enum {
    RESOURCE_ID_NONE = 0, /// 无资源

    /* 通用组 [1..100] */
    RESOURCE_ID_BLE_CONNECT_ICON = 1,           /// 蓝牙连接图标
    RESOURCE_ID_BLE_DISCONNECT_ICON,            /// 蓝牙断开图标
    RESOURCE_ID_BLE_CONNECT_POPUP_ICON,         /// 蓝牙连接弹窗图标
    RESOURCE_ID_BLE_DISCONNECT_POPUP_ICON,      /// 蓝牙断开弹窗图标
    RESOURCE_ID_BIND_ICON,                      /// 绑定图标
    RESOURCE_ID_UNBIND_ICON,                    /// 解绑图标
    RESOURCE_ID_ALLOW_ICON,                     /// 允许图标
    RESOURCE_ID_BAN_ICON,                       /// 禁止图标
    RESOURCE_ID_WARNING_ICON,                   /// 警告图标

    RESOURCE_ID_BATT_CAP_0_ICON = 50,          /// 电池电量0%
    RESOURCE_ID_BATT_CAP_20_ICON,              /// 电池电量20%
    RESOURCE_ID_BATT_CAP_40_ICON,              /// 电池电量40%
    RESOURCE_ID_BATT_CAP_60_ICON,              /// 电池电量60%
    RESOURCE_ID_BATT_CAP_80_ICON,              /// 电池电量80%
    RESOURCE_ID_BATT_CAP_100_ICON,             /// 电池电量100%
    RESOURCE_ID_CHARGE_ICON,                   /// 电池充电图标
    RESOURCE_ID_LOW_POWER_ICON,                /// 电池低电量图标
    RESOURCE_ID_CLOCK_COLON_ICON,              /// 时钟冒号图标
 
    /* 主界面组 [100..150] */
    RESOURCE_ID_COMMON_BG = 100,               /// 通用背景
    RESOURCE_ID_START_LOGO_ICON,               /// 启动LOGO
    RESOURCE_ID_HOME_IDLE_LABLE_ICON,          /// 空闲界面文本区域图标
    RESOURCE_ID_HOME_IDLE_BUTTOM_ICON,         /// 空闲界面按钮指示图标
    RESOURCE_ID_AIRPLANE_UNLOCK_BUTTON_ICON,   /// 飞行模式解锁按钮图标
    RESOURCE_ID_HOME_LIGHT_LABLE_ICON,         /// 首页光线图标
    RESOURCE_ID_HOME_BLE_LABEL_ICON,           /// 首页蓝牙标签图标


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

    RESOURCE_ID_PUMP_PAUSE_PROMPT_ICON,        /// 泵奶暂停提示图标
    RESOURCE_ID_PUMP_LEVEL_BG,                 /// 泵奶档位背景
    RESOURCE_ID_PUMP_LEVEL_BOTH_SIDES_BG,      /// 泵奶档位双侧模式背景
    RESOURCE_ID_PUMP_LEVEL_ADD_LABEL_ICON,     /// 泵奶档位文本加号图标
    RESOURCE_ID_PUMP_LEVEL_SUB_LABEL_ICON,     /// 泵奶档位文本减号图标
    RESOURCE_ID_PUMP_LEFT_LEVEL_ICON,          /// 泵奶左档位图标
    RESOURCE_ID_PUMP_RIGHT_LEVEL_ICON,         /// 泵奶右档位图标

    /* 动画组 [300...] */
    RESOURCE_ID_PUMP_LEVEL_START = 300,   /// 泵奶档位图片-开始
    RESOURCE_ID_PUMP_LEVEL_END = 314,     /// 泵奶档位图片-结束

    RESOURCE_ID_SELECTED_LOCATE_3_START = 320,     /// 选中定位-3-图标 
    RESOURCE_ID_SELECTED_LOCATE_3_END = 322,       /// 选中定位-3-图标

    RESOURCE_ID_SELECTED_LOCATE_5_START = 330,     /// 选中定位-5-图标 
    RESOURCE_ID_SELECTED_LOCATE_5_END = 334,       /// 选中定位-5-图标

    RESOURCE_ID_SELECTED_LOCATE_6_START = 340,     /// 选中定位-6-图标 
    RESOURCE_ID_SELECTED_LOCATE_6_END = 345,       /// 选中定位-6-图标

    /* IP图标组 [1000..] */
    RESOURCE_ID_HOME_IDLE_IP = 1000,      /// dummy图标
    RESOURCE_ID_HOME_BLE_IP,              /// 首页蓝牙IP图标
    RESOURCE_ID_PUMP_FINISH_IP,           /// 泵奶完成图标
    RESOURCE_ID_CHARGE_ADAPTER_IP,        /// 首页充电适配器图标
    RESOURCE_ID_LOW_POWER_IP,             /// 低电量页面人物图标
    RESOURCE_ID_CHARGE_PAGE_IP,           /// 充电页背景

    RESOURCE_ID_YAWN_ANIM_IP_START = 1100,   /// 打哈欠动画-开始
    RESOURCE_ID_YAWN_ANIM_IP_END = 1180,     /// 打哈欠动画-结束

    RESOURCE_ID_SHAKE_BOTTLE_ANIM_IP_START = 1200,   /// 摇奶瓶动画-开始
    RESOURCE_ID_SHAKE_BOTTLE_ANIM_IP_END = 1280,     /// 摇奶瓶动画-结束

    RESOURCE_ID_SUCTION_ANIM_IP_START = 1300,   /// 吸奶动画-开始
    RESOURCE_ID_SUCTION_ANIM_IP_END = 1326,     /// 吸奶动画-结束

    RESOURCE_ID_HD_ANIM_IP_START = 1500,   /// 首页默认动画-开始
    RESOURCE_ID_HD_ANIM_IP_END = 1619,     /// 首页默认动画-结束
    
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


