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

    /* 主界面组 [100..150] */
    RESOURCE_ID_COMMON_BG = 100,               /// 通用背景
    RESOURCE_ID_START_LOGO_ICON,               /// 启动LOGO
    RESOURCE_ID_HOME_WIDTH_LABLE,              /// 主页宽标签
    RESOURCE_ID_HOME_START_BUTTON_ICON,        /// 空闲界面开始按钮指示图标
    RESOURCE_ID_AIRPLANE_UNLOCK_BUTTON_ICON,   /// 飞行模式解锁按钮图标
    RESOURCE_ID_HOME_NARROW_LABLE,             /// 主页窄标签
    RESOURCE_ID_HOME_BLE_LABEL,                /// 主页蓝牙标签"

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
    RESOURCE_ID_PUMP_SPEED_FAST_ICON,          /// 泵奶首页-快速图标
    RESOURCE_ID_REGULAR_PUMPING_BG,            /// 泵奶首页-常规泵奶背景
    RESOURCE_ID_EXPERT_PUMPING_BG,             /// 泵奶首页-专家泵奶背景
    RESOURCE_ID_PUMP_SIMULATION_ICON,          /// 泵奶首页-刺激图标
    RESOURCE_ID_PUMP_EXPRESSION_ICON,          /// 泵奶首页-泌乳图标
    RESOURCE_ID_PUMP_MIXED_ICON,               /// 泵奶首页-混合图标
    RESOURCE_ID_PUMP_MILK_BOOST_ICON,          /// 泵奶首页-奶量提升图标
    RESOURCE_ID_PUMP_POWER_PUMPING_ICON,       /// 泵奶首页-pp追奶图标
    RESOURCE_ID_PUMP_CUSTOMIZE_ICON,           /// 泵奶首页-自定义图标

    RESOURCE_ID_SPEED_PAGE_FAST_ICON,          /// 频率页-快频率图标
    RESOURCE_ID_SPEED_PAGE_REGULAR_ICON,       /// 频率页-常规频率图标
    RESOURCE_ID_SPEED_PAGE_SLOW_ICON,          /// 频率页-慢频率图标

    RESOURCE_ID_LAST_USED_MODE_ICON,           /// 上一次使用模式图标
    RESOURCE_ID_NEW_MODE_STEPS_ICON,           /// 新模式未确认图标
    RESOURCE_ID_MODE_LACTATION_ICON,           /// 模式-泌乳图标
    RESOURCE_ID_MODE_MIX_ICON,                 /// 模式-混合图标
    RESOURCE_ID_MODE_STIMULATE_ICON,           /// 模式-刺激图标
    RESOURCE_ID_MODE_RELAX_ICON,               /// 模式-放松图标

    RESOURCE_ID_MODE_CUSTOM_ICON,              /// 韵律-专家模式图标
    RESOURCE_ID_MODE_EXPERT_ICON,              /// 韵律-个性化模式图标

    RESOURCE_ID_PUMP_PAUSE_ICON,               /// 泵奶暂停图标
    RESOURCE_ID_PUMP_LEVEL_BG,                 /// 泵奶档位背景
    RESOURCE_ID_PUMP_LEVEL_BOTH_SIDES_BG,      /// 泵奶档位双侧模式背景
    RESOURCE_ID_PUMP_LEVEL_ADD_ICON,           /// 泵奶档位文本加号图标
    RESOURCE_ID_PUMP_LEVEL_SUB_ICON,           /// 泵奶档位文本减号图标
    RESOURCE_ID_PUMP_LEFT_LEVEL_ICON,          /// 泵奶左档位图标
    RESOURCE_ID_PUMP_RIGHT_LEVEL_ICON,         /// 泵奶右档位图标
    RESOURCE_ID_SELECT_LABEL,                  /// 选中标签图标
    RESOURCE_ID_STIMULATION_WAVE_ICON,         /// 刺激波形图标
    RESOURCE_ID_LACTATION_WAVE_ICON,           /// 泌乳波形图标
    RESOURCE_ID_MIX_WAVE_ICON,                 /// 混合波形图标
    
    /* 动画组 [300...] */
    RESOURCE_ID_PUMP_LEVEL_START = 300,   /// 泵奶档位图片-开始
    RESOURCE_ID_PUMP_LEVEL_END = 314,     /// 泵奶档位图片-结束

    /* IP图标组 [1000..] */
    RESOURCE_ID_HOME_IDLE_IP = 1000,      /// dummy图标
    RESOURCE_ID_HOME_BLE_IP,              /// 首页蓝牙IP图标
    RESOURCE_ID_PUMP_FINISH_IP,           /// 泵奶完成图标
    RESOURCE_ID_CHARGE_ADAPTER_IP,        /// 首页充电适配器图标
   
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


