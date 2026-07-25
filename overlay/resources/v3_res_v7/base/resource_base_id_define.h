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
    BASE_RES_ID_CLOCK_COLON_ICON = 1, /// 时钟冒号图标
    BASE_RES_ID_BLE_DISCONNECT_ICON, /// 蓝牙断开图标
    BASE_RES_ID_SUCCESS_ICON, /// 成功图标
    BASE_RES_ID_PLAY_ICON, /// 播放图标
    BASE_RES_ID_PAUSE_ICON, /// 暂停图标
    BASE_RES_ID_POWER_OFF_ICON, /// 电源图标
    BASE_RES_ID_POWER_KEY_ICON, /// 电源按键图标

    /* 主界面组 [99..150] */
    BASE_RES_ID_START_LOGO_ICON, /// 启动LOGO图标
    BASE_RES_ID_HOME_TEXT_MASK, /// 主页文案蒙层

    /* 引导组 [150..200] */

    /* 模式相关组 [200..300] */
    BASE_RES_ID_SIMULATION_MODE_BG = 200, /// 刺激模式背景
    BASE_RES_ID_EXPRESSION_MODE_BG, /// 吸吮模式背景
    BASE_RES_ID_MIXED_MODE_BG, /// 混合模式背景
    BASE_RES_ID_PROGRAM_MODE_BG, /// 韵律模式背景

    BASE_RES_ID_SIMULATION_ICON, /// 刺激模式图标
    BASE_RES_ID_EXPRESSION_ICON, /// 吸吮模式图标
    BASE_RES_ID_MIXED_ICON, /// 混合模式图标
    BASE_RES_ID_PROGRAM_ICON, /// 韵律模式图标
    BASE_RES_ID_SPEED_ICON, /// 速度图标

    /* 波形组 [300..400] */
    BASE_RES_ID_STIMULATION_WAVE = 300, /// 刺激波形
    BASE_RES_ID_LACTATION_WAVE, /// 泌乳波形
    BASE_RES_ID_MIX_WAVE, /// 混合波形
    BASE_RES_ID_MILK_BOOST_WAVE, /// 2+8奶量提升波形
    BASE_RES_ID_POWER_PUMPING_WAVE, /// 泵奶模式波形

    /* 动画组 [1000..] */
    BASE_RES_ID_LONG_SUCTION_ANIM_START = 1000, /// 长吸动画-开始
    BASE_RES_ID_LONG_SUCTION_ANIM_END = 1059, /// 长吸动画-结束 
    BASE_RES_ID_SHORT_SUCTION_ANIM_START = 1100, /// 短吸动画-开始
    BASE_RES_ID_SHORT_SUCTION_ANIM_END = 1159, /// 短吸动画-结束 
    BASE_RES_ID_MIXED_SUCTION_ANIM_START = 1200, /// 混合吸吮动画-开始
    BASE_RES_ID_MIXED_SUCTION_ANIM_END = 1259, /// 混合吸吮动画-结束 
    BASE_RES_ID_RELAX_SUCTION_ANIM_START = 1300, /// 放松吸吮动画-开始
    BASE_RES_ID_RELAX_SUCTION_ANIM_END = 1359, /// 放松吸吮动画-结束 
    BASE_RES_ID_GUIDE_ANIM_START = 1400, /// 引导动画-开始
    BASE_RES_ID_GUIDE_ANIM_END = 1468, /// 引导动画-结束 

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
/// 引导动画帧数
#define BASE_GUIDE_ANIM_NUM \
    (BASE_RES_ID_GUIDE_ANIM_END - BASE_RES_ID_GUIDE_ANIM_START + 1)

#ifdef __cplusplus
}
#endif

#endif
