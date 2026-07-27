#ifndef __WINDOW_SCREEN_PUMP_H__
#define __WINDOW_SCREEN_PUMP_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "windows_map_define.h" // IWYU pragma: keep
#include "input_key_code.h" // IWYU pragma: keep

/// 泵奶动画频率
#if (ROOT_UI_VERSION == ROOT_UI_V2)
#define PUMP_ANIMATION_FREQ                    (24)
#else
#define PUMP_ANIMATION_FREQ                    (20)
#define PUMP_CUSTOM_ANIMATION_FREQ             (16)
#endif
#define PUMP_MODE_ANIMATION_FREQ               (25) 
#define PUMP_SPEED_ANIMATION_FREQ              (25) 
#define PUMP_LEVEL_ANIMATION_FREQ              (20) 

#if (ROOT_UI_VERSION == ROOT_UI_V2)
static inline uint8_t pump_animation_freq_get(int res_theme_type)
{
    (void)res_theme_type;
    return PUMP_ANIMATION_FREQ;
}
#else
static inline uint8_t pump_animation_freq_get(int res_theme_type)
{
    return (res_theme_type == RES_THEME_TYPE_CUSTOM)
        ? PUMP_CUSTOM_ANIMATION_FREQ : PUMP_ANIMATION_FREQ;
}
#endif

/// 是否支持泵奶中切换韵律
#define IS_SUPPORT_PUMP_IN_MODE_SWITCH_RHYTHM  (1)

/// 判断泵奶时间
#define IS_PUMPTIME_LESS_THEN_30S(result)      ((result)->real_valid_time_sec < 30 )
#define IS_PUMPTIME_LESS_THEN_120S(result)     ((result)->real_valid_time_sec < 120)

/// 泵奶主界面档位调节展示时间
#define PUMP_LEVEL_DISPLAY_TIMEOUT             (3000) /// 3s
/// 泵奶主界面模式选择展示时间
#define PUMP_MODE_SELECT_DISPLAY_TIMEOUT       (3000) /// 3s
/// 泵奶结果显示超时时间
#define PUMP_RESULT_DISPLAY_TIMEOUT            (3000) /// 3s
/// 泵奶背光灯周期
#define PUMP_BLINK_PERIOD                      (500)
/// 泵奶背光灯闪烁次数
#define PUMP_BLINK_TIMES                       (1)
/// 泵奶控制背光灯闪烁次数
#define PUMP_LEVEL_BLINK_TIMES                 (3)
/// 泵奶完成页面允许被打断
#define IS_ALLLOW_PUMP_COMPLETE_BREAK          (0)
/// 泵奶完成提示弹窗(ID_POPUP_UI_PUMP_FINISH_MANUAL / ID_POPUP_UI_PUMP_WILL_FINISH_TIP)
/// 开始键/频率键背光闪烁，0: 常亮，1: 闪烁
#define IS_SUPPORT_PUMP_FINISH_TIP_LED_BLINK   (0)
#define IS_PUMP_FINISH_TIP_POPUP(type) \
    ((type) == ID_POPUP_UI_PUMP_FINISH_MANUAL || \
     (type) == ID_POPUP_UI_PUMP_WILL_FINISH_TIP)

/// 泵奶UI动作枚举
typedef enum {
    PUMP_HDR_ACTION_START_PUMP = 100,
    PUMP_HDR_ACTION_CHANGE_PUMP_STATUS,
    PUMP_HDR_ACTION_LOOP_SHOW,
    PUMP_HDR_ACTION_SHOW_ONCE,
    PUMP_HDR_ACTION_REGISTER_KEYEVENT,
    PUMP_HDR_ACTION_SHOW_THEMES_ANI,
    PUMP_HDR_ACTION_RECOVER_DEFAULT_ANI,
    PUMP_HDR_ACTION_RESET_PLAY_ANI_FPS,
    PUMP_HDR_ACTION_PROGRESS_START,
    PUMP_HDR_ACTION_PROGRESS_STOP,
    PUMP_HDR_ACTION_PUMP_COMPLETE,
    PUMP_HDR_ACTION_MAX
} pump_ui_hdr_action_e;

/// 泵奶循环显示类型枚举
typedef enum {
    LOOP_SHOW_SWITCH_MODE = 0,
    LOOP_SHOW_PUMPING,
    LOOP_SHOW_RESULT_RISK_WARN, /// 风险警告
    LOOP_SHOW_RESULT_RISK_COMPARE, /// 风险比较
    LOOP_SHOW_THEMES_ANI, /// 主题动画
    LOOP_SHOW_SETTING_ENTER, /// 设置入口
    LOOP_SHOW_MAX
} pump_loop_show_type_e;

/// 泵奶UI类型枚举
typedef enum {
    PUMP_UI_TYPE_NONE = 0,
    PUMP_UI_TYPE_HOME = 100,
    PUMP_UI_TYPE_PUMPING,
    PUMP_UI_TYPE_RESULT,
    PUMP_UI_TYPE_MAX
} pump_ui_type_e;

typedef enum {
    PUMP_LEVEL_DISPLAY_DEFAULT = 0,     /// 默认 UI
    PUMP_LEVEL_DISPLAY_LEFT_ADJUST,     /// 左档位调节
    PUMP_LEVEL_DISPLAY_RIGHT_ADJUST,    /// 右档位调节
    PUMP_LEVEL_DISPLAY_BOTH_ADJUST,     /// 双档位调节
} pump_level_display_type_e;

typedef enum {
    PUMP_MODE_DISPLAY_DEFAULT = 0,      /// 默认 UI
    PUMP_MODE_DISPLAY_BF_MODE,          /// 追奶模式
    PUMP_MODE_DISPLAY_NORMAL_MODE,      /// 普通模式
} pump_mode_display_type_e;

/// 泵奶信息结构体
typedef struct {    
    uint8_t common_mode; /// 普通泵奶模式.
    uint8_t status; /// 泵奶状态.
    uint8_t breastfeeding_mode; /// 追奶模式.
    uint8_t control_type; /// 控制类型. 0: 左, 1: 右. 2: 双侧.
} ui_pump_info_t;

typedef enum {
    PUMP_LIGHT_STATUS_DEFAULT = 0,      /// 默认
    PUMP_LIGHT_STATUS_PAUSE,            /// 暂停
    PUMP_LIGHT_STATUS_FINISH_TIP,       /// 完成提示闪烁
    PUMP_LIGHT_STATUS_FINISH_MANUAL,    /// 手动结束确认：仅返回键常亮
    PUMP_LIGHT_STATUS_MAX
} pump_light_status_e;

/// 泵奶UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    breast_pump_manager_context_t *pump;
    battery_manager_context_t *battery;
    ble_manager_context_t *ble;
    motor_manager_context_t *motor;
    power_manager_context_t *pm;
    key_input_context_t *key;
    algo_manager_context_t *algo;
    led_manager_context_t *led;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic;
    uint8_t window_run_state; /// WM是否运行.
    uint8_t exit_current_ui; /// 退出当前UI标志.
    uint16_t current_ui_type; /// 当前UI类型
    uint16_t start_ui_type;   
    uint16_t popup_ui_type; /// 当前弹窗类型. popup_ui_id_e
    uint8_t finish_manual_holding; /// 手动结束弹窗是否已进入长按倒计时
    uint8_t finish_manual_ignore_click; /// 手动结束弹窗松手后忽略紧随的 CLICK
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    /// 泵奶信息
    uint8_t pump_will_finish; /// 泵奶即将完成标志.
    // ui_pump_info_t info; /// 当前泵奶信息.
    breast_pump_result_t pump_result; /// 泵奶结果.
    uint8_t pump_status; /// 泵奶状态.
    /// 模式相关
    uint8_t ui_mode_index; /// 当前模式索引.
    uint8_t pump_mode_display_type; /// 当前泵奶模式显示类型
    uint32_t pump_mode_adjust_time; /// 泵奶模式调节显示时间
    /// 档位调节相关
    uint8_t level_adjust_pump_num;   /// 档位调节泵数
    uint8_t level_adjust_pump_index; /// 档位调节泵索引
    uint8_t pump_level_display_type; /// 当前泵奶档位调节类型
    uint32_t level_adjust_time; /// 档位调节显示时间
    pump_setting_t setting; /// 当前泵奶设置信息.
    /// 速度相关
    uint8_t pump_speed; /// 当前泵奶速度.
    uint8_t pump_speed_display_type; /// 当前泵奶速度显示类型.
    uint32_t speed_adjust_time; /// 速度调节显示时间.
    /// 灯光相关
    uint8_t pump_light_status; /// 泵奶灯光状态.
    uint8_t state_blink_visible; /// 状态闪烁可见: 0: 隐藏相, 1: 显示相.
    uint8_t finish_tip_blink_visible; /// 完成提示闪烁可见: 0: 隐藏相, 1: 显示相.
    uint8_t pump_pause_blink_state; /// 泵奶暂停闪烁状态: 0: 不闪烁, 1: 闪烁.
    uint32_t blink_start_time; /// 闪烁起始时间
} ui_proc_pump_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 显示泵奶主界面
void show_pump_home_ui(ui_proc_pump_t *proc);

/// 模式调节
int pump_mode_adjust_handler(ui_proc_pump_t *proc, bool is_bf_mode);
void pump_mode_adjust_timeout_handler(ui_proc_pump_t *proc);
int pump_mode_adjust_exit_handler(ui_proc_pump_t *proc);

/// 档位调节
int pump_level_adjust_type_update(ui_proc_pump_t *proc, uint8_t is_left);
int pump_level_adjust_handler(ui_proc_pump_t *proc, bool is_increase);
void pump_level_adjust_timeout_handler(ui_proc_pump_t *proc);
int pump_level_adjust_exit_handler(ui_proc_pump_t *proc);

/// 泵奶状态UI相关
void switch_pump_status_ui(ui_proc_pump_t *proc);
/// 移除UI相关
void pump_ui_change_hdr_remove(ui_proc_pump_t *proc);
/// 停止泵奶UI动画
void stop_pump_ui_animation(ui_proc_pump_t *proc);
/// 泵奶完成事件处理
void pump_complete_event_handler(ui_proc_pump_t *proc);
/// 弹窗类型更新
void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type);
/// 子UI退出处理
void pump_sub_ui_exit_handler(ui_proc_pump_t *proc);
/// 电源按键事件处理
void pump_power_key_event_handler(ui_proc_pump_t *proc);
/// 取消按键事件处理
void pump_cancel_key_event_handler(ui_proc_pump_t *proc);
/// 取消按键按下/抬起（手动结束确认长按倒计时）
void pump_cancel_key_down_handler(ui_proc_pump_t *proc);
void pump_cancel_key_up_handler(ui_proc_pump_t *proc);
/// 手动结束确认弹窗下按任意键退出弹窗
void pump_finish_manual_popup_dismiss(ui_proc_pump_t *proc);
/// 开始按键事件处理
void pump_start_key_event_handler(ui_proc_pump_t *proc);
/// 模式按键事件处理
void pump_mode_key_event_handler(ui_proc_pump_t *proc);
/// 追奶模式按键事件处理
void pump_breastfeeding_key_event_handler(ui_proc_pump_t *proc);
/// 左档位按键事件处理
void pump_level_control_key_event_handler(ui_proc_pump_t *proc, uint8_t is_left);
/// 旋钮按键事件处理
void pump_knob_event_cb(ui_proc_pump_t *proc, uint8_t is_left);

#ifdef __cplusplus
}
#endif

#endif 


