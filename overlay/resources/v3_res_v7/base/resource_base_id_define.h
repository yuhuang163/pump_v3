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
    BASE_RES_ID_HOUR_COLON_ICON, /// 小时冒号图标
    BASE_RES_ID_BLE_CONNECT_ICON, /// 蓝牙连接图标
    BASE_RES_ID_BLE_CONNECT_TIP_ICON, /// 蓝牙连接提示图标
    BASE_RES_ID_BLE_DISCONNECT_ICON, /// 蓝牙断开图标
    BASE_RES_ID_BLE_DISCONNECT_TIP_ICON, /// 蓝牙断开提示图标
    BASE_RES_ID_SUCCESS_ICON, /// 成功图标
    BASE_RES_ID_ERROR_ICON, /// 错误图标
    BASE_RES_ID_WARNING_ICON, /// 警告图标
    BASE_RES_ID_QUANDARY_ICON, /// 疑问图标
    BASE_RES_ID_DOWNLOAD_ICON, /// 下载图标
    BASE_RES_ID_PLAY_ICON, /// 播放图标
    BASE_RES_ID_PLAY_MIN_ICON, /// 播放小图标
    BASE_RES_ID_PAUSE_ICON, /// 暂停图标
    BASE_RES_ID_PAUSE_MIN_ICON, /// 暂停小图标
    BASE_RES_ID_POWER_OFF_ICON, /// 电源图标
    BASE_RES_ID_POWER_KEY_ICON, /// 电源按键图标
    BASE_RES_ID_TIMEOUT_ICON, /// 超时图标
    BASE_RES_ID_ROTARY_KEY_ICON, /// 旋钮按键图标
    BASE_RES_ID_TRAVEL_LOCK_ICON, /// 旅行锁锁定图标
    BASE_RES_ID_CHARGE_ERROR_ICON, /// 充电错误图标
    BASE_RES_ID_ADD_ICON, /// 加号图标
    BASE_RES_ID_STIMULATION_KEY_ICON, /// 刺激按键图标
    BASE_RES_ID_EXPRESSION_KEY_ICON, /// 吸吮按键图标

    /* 主界面组 [150..200] */
    BASE_RES_ID_START_LOGO_ICON = 150, /// 启动LOGO图标
    BASE_RES_ID_HOME_IDLE_ICON, /// 主页空闲图标

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
    BASE_RES_ID_PUMP_TIME_LABEL, /// 泵奶时间标签
    BASE_RES_ID_MODE_SELECT_TIP, /// 模式选择页提示文案图片
    BASE_RES_ID_TEXT_SHINE_MASK, /// 文案扫光蒙层

    /* 动画组 [1000..] */
    BASE_RES_ID_STIMULATION_ANIM_START = 1000, /// 刺激动画-开始
    BASE_RES_ID_STIMULATION_ANIM_END = 1059, /// 刺激动画-结束 
    BASE_RES_ID_EXPRESSION_ANIM_START = 1100, /// 吸吮动画-开始
    BASE_RES_ID_EXPRESSION_ANIM_END = 1159, /// 吸吮动画-结束 
    BASE_RES_ID_PROGRAM_STIMULATION_ANIM_START = 1200, /// 韵律刺激动画-开始
    BASE_RES_ID_PROGRAM_STIMULATION_ANIM_END = 1259, /// 韵律刺激动画-结束 
    BASE_RES_ID_PROGRAM_EXPRESSION_ANIM_START = 1300, /// 韵律吸吮动画-开始
    BASE_RES_ID_PROGRAM_EXPRESSION_ANIM_END = 1359, /// 韵律吸吮动画-结束 
    BASE_RES_ID_PROGRAM_RELAX_ANIM_START = 1400, /// 韵律放松动画-开始
    BASE_RES_ID_PROGRAM_RELAX_ANIM_END = 1459, /// 韵律放松动画-结束 
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
