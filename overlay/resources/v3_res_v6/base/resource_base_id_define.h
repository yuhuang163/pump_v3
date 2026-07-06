#ifndef __RESOURCE_BASE_ID_DEFINE_H__
#define __RESOURCE_BASE_ID_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/// base 主题资源 ID 区间 [0 .. 10000]
#define BASE_RES_ID_RANGE_MIN                     0
#define BASE_RES_ID_RANGE_MAX                     10000

typedef enum {
    BASE_RES_ID_NONE = 0, /// 无资源

    /* 通用组 [1..100] */
    BASE_RES_ID_BLE_CONNECT_ICON = 1, /// 蓝牙连接图标
    BASE_RES_ID_BLE_DISCONNECT_ICON, /// 蓝牙断开图标
    BASE_RES_ID_BLE_CONNECT_POPUP_ICON, /// 蓝牙连接弹窗图标
    BASE_RES_ID_BLE_DISCONNECT_POPUP_ICON, /// 蓝牙断开弹窗图标
    BASE_RES_ID_BIND_ICON, /// 绑定图标
    BASE_RES_ID_UNBIND_ICON, /// 解绑图标
    BASE_RES_ID_ALLOW_ICON, /// 允许图标
    BASE_RES_ID_BAN_ICON, /// 禁止图标
    BASE_RES_ID_WARNING_ICON, /// 警告图标
    BASE_RES_ID_START_BUTTON_ICON, /// 开始按钮指示图标
    BASE_RES_ID_RETURN_BUTTON_ICON, /// 返回按钮指示图标
    BASE_RES_ID_MODE_BUTTON_ICON, /// 模式按钮图标
    BASE_RES_ID_PROGRAM_BUTTON_ICON, /// 程序按钮图标
  
    BASE_RES_ID_CLOCK_COLON_ICON = 99, /// 时钟冒号图标
    /* 主界面组 [99..150] */
    BASE_RES_ID_COMMON_BG = 100, /// 通用背景
    BASE_RES_ID_START_LOGO_ICON, /// 启动LOGO
    BASE_RES_ID_HOME_WIDTH_LABLE, /// 主页宽标签
    BASE_RES_ID_HOME_BIG_LABLE, /// 主页大标签
    BASE_RES_ID_HOME_NARROW_LABLE, /// 主页窄标签
    BASE_RES_ID_AIRPLANE_UNLOCK_BUTTON_ICON, /// 飞行模式解锁按钮图标
    BASE_RES_ID_HOME_BLE_LABEL, /// 主页蓝牙标签

    /* 引导组 [150..200] */
    BASE_RES_ID_GUIDE_PAGE1_IMG = 150, /// 引导-页1图片
    BASE_RES_ID_GUIDE_PAGE2_IMG, /// 引导-页2图片
    BASE_RES_ID_GUIDE_PAGE3_IMG, /// 引导-页3图片
    BASE_RES_ID_GUIDE_PAGE4_IMG, /// 引导-页4图片
    BASE_RES_ID_GUIDE_PAGE5_IMG, /// 引导-页5图片
    BASE_RES_ID_GUIDE_PAGE6_IMG, /// 引导-页6图片
    BASE_RES_ID_SYS_THEME_IMG, /// 系统主题引导背景
    BASE_RES_ID_IP_THEME_IMG, /// IP主题引导背景
    BASE_RES_ID_THEME_SELECT_ICON, /// 主题选择引导图标
    BASE_RES_ID_THEME_UNSELECT_ICON, /// 主题未选择引导图标

    /* 模式相关组 [200..300]（大图 + 小图标） */
    BASE_RES_ID_HOME_IDLE_ICON = 200, /// 空闲界面图标
    BASE_RES_ID_PUMP_PAUSE_ICON, /// 泵奶暂停图标
    BASE_RES_ID_PUMP_LEVEL_SEPARATOR, /// 泵奶力度分割符
    BASE_RES_ID_PUMP_LEVEL_BG, /// 泵奶力度背景
    BASE_RES_ID_PUMP_LEVEL_BIG_BG, /// 泵奶力度大背景
    BASE_RES_ID_PUMP_LEVEL_INDICATOR, /// 泵奶力度指示符
    BASE_RES_ID_PUMP_LEVEL_BIG_INDICATOR, /// 泵奶力度大指示符
    BASE_RES_ID_PUMP_RHYTHM_ICON, /// 泵奶节奏图标
    BASE_RES_ID_PUMP_RHYTHM_WHITE_ICON, /// 泵奶节奏白色图标
    BASE_RES_ID_PUMP_RELAX_ICON, /// 泵奶放松图标

    BASE_RES_ID_STIMULATION_WAVE = 300, /// 刺激波形
    BASE_RES_ID_LACTATION_WAVE, /// 泌乳波形
    BASE_RES_ID_MIX_WAVE, /// 混合波形
    BASE_RES_ID_RELAX_WAVE, /// 放松波形
    BASE_RES_ID_MILK_BOOST_WAVE, /// 2+8奶量提升波形
    BASE_RES_ID_POWER_PUMPING_WAVE, /// 泵奶模式波形
    BASE_RES_ID_CUSTOM_WAVE, /// 个性化模式波形

    /* 动画组 [1000..] */
    BASE_RES_ID_LONG_SUCTION_ANIM_START = 1000, /// 长吸动画-开始
    BASE_RES_ID_LONG_SUCTION_ANIM_END = 1059, /// 长吸动画-结束 
    BASE_RES_ID_SHORT_SUCTION_ANIM_START = 1100, /// 短吸动画-开始
    BASE_RES_ID_SHORT_SUCTION_ANIM_END = 1159, /// 短吸动画-结束 
    BASE_RES_ID_MIXED_SUCTION_ANIM_START = 1200, /// 混合吸吮动画-开始
    BASE_RES_ID_MIXED_SUCTION_ANIM_END = 1259, /// 混合吸吮动画-结束 
    BASE_RES_ID_RELAX_SUCTION_ANIM_START = 1300, /// 放松吸吮动画-开始
    BASE_RES_ID_RELAX_SUCTION_ANIM_END = 1359, /// 放松吸吮动画-结束 

    BASE_RES_ID_TYPE_MAX
} base_res_id_type_e;

/// 长吸动画帧数
#define BASE_LONG_SUCTION_ANIM_NUM \
    (BASE_RES_ID_LONG_SUCTION_ANIM_END - BASE_RES_ID_LONG_SUCTION_ANIM_START + 1)
/// 短吸动画帧数
#define BASE_SHORT_SUCTION_ANIM_NUM \
    (BASE_RES_ID_SHORT_SUCTION_ANIM_END - BASE_RES_ID_SHORT_SUCTION_ANIM_START + 1)
/// 混合吸吮动画帧数
#define BASE_MIXED_SUCTION_ANIM_NUM \
    (BASE_RES_ID_MIXED_SUCTION_ANIM_END - BASE_RES_ID_MIXED_SUCTION_ANIM_START + 1)
/// 放松吸吮动画帧数
#define BASE_RELAX_SUCTION_ANIM_NUM \
    (BASE_RES_ID_RELAX_SUCTION_ANIM_END - BASE_RES_ID_RELAX_SUCTION_ANIM_START + 1)

#ifdef __cplusplus
}
#endif

#endif
