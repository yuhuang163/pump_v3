#ifndef __WINDOW_SCREEN_HOME_H__
#define __WINDOW_SCREEN_HOME_H__

#include <stdint.h>
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

#if (ROOT_UI_VERSION == ROOT_UI_V3 || ROOT_UI_VERSION == ROOT_UI_V2)
#define HOME_ANIMATION_FREQ                    (24)
#else
#define HOME_ANIMATION_FREQ                    (25)
#endif

#define HOME_PUMP_MODE_ANIMATION_FREQ          (25) 
#define HOME_PUMP_SPEED_ANIMATION_FREQ         (25) 
#define HOME_PUMP_LEVEL_ANIMATION_FREQ         (20) 

/// 主界面动画展示时间
#define HOME_ANIMATION_DISPLAY_TIME            (2000) /// 2s

/// 主界面绑定二维码展示时长
#define HOME_BIND_QRCODE_DISPLAY_TIMEOUT       (3000) /// 3s

/// 模式选择界面动画展示时间
#define HOME_MODE_SELECT_DISPLAY_TIME          (5000) /// 5s

/// 档位调节界面动画展示时间
#define HOME_PUMP_LEVEL_DISPLAY_TIMEOUT        (5000) /// 5s

/// 泵奶结果显示超时时间
#define HOME_PUMP_RESULT_DISPLAY_TIMEOUT       (3000) /// 3s

/// 夜灯调节界面动画展示时间
#define HOME_NIGHT_LIGHT_ADJUST_SYNC_TIMEOUT   (3000) /// 3s

/// UTC 校准失效时间
#define HOME_UTC_CALIB_EXPIRE_INTERVAL_SEC      (20 * 24 * 3600)
/// 主界面状态闪烁半周期(ms)，与泵奶界面同频
#define HOME_BLINK_PERIOD                       (500)

/// 主界面UI动作枚举
typedef enum {
    HOME_HDR_ACTION_PUMP_MODE_SELECT,
    HOME_HDR_ACTION_MAX
} home_ui_hdr_action_e;

/// 主界面UI类型枚举
typedef enum {
    HOME_UI_TYPE_NONE = 0,
    HOME_UI_TYPE_HOME,
    HOME_UI_TYPE_HOME_IDLE,
    HOME_UI_TYPE_BF_MODE_SELECT,
    HOME_UI_TYPE_NORMAL_MODE_SELECT,
    HOME_UI_TYPE_PUMP_RESULT,
    HOME_UI_TYPE_NIGHT_LIGHT_ADJUST, /// 夜灯调节
    HOME_UI_TYPE_MAX
} home_ui_type_e;

typedef enum {
    HOME_UI_DISPLAY_DEFAULT = 0,       /// 默认 UI
    HOME_UI_DISPLAY_RECORD,            /// 显示记录 UI
    HOME_UI_DISPLAY_CLOCK_AND_RECORD,  /// 显示时钟和记录 UI
    HOME_UI_DISPLAY_CLOCK,             /// 显示时钟 UI
    HOME_UI_DISPLAY_CLOCK_LIGHT_TIP,   /// 显示时钟夜灯提示
    HOME_UI_DISPLAY_LOW_POWER,         /// 显示低电量 UI
    HOME_UI_DISPLAY_CLOCK_LOW_POWER,   /// 显示带时钟低电量 UI
    HOME_UI_DISPLAY_SIMULATE_MODE_DESC,      /// 显示刺激模式描述
    HOME_UI_DISPLAY_EXPRESSION_MODE_DESC,    /// 显示表达模式描述
    HOME_UI_DISPLAY_MIXED_MODE_DESC,         /// 显示混合模式描述
    HOME_UI_DISPLAY_MILK_BOOST_DESC,         /// 显示2+8奶量提升模式描述
    HOME_UI_DISPLAY_POWER_PUMPING_DESC,      /// 显示泵奶模式描述
    HOME_UI_DISPLAY_CUSTOMIZE_DESC,          /// 显示个性化模式描述
    HOME_UI_DISPLAY_MAX
} home_page_display_type_e;

typedef enum {
    HOME_PUMP_LEVEL_DISPLAY_DEFAULT = 0,     /// 默认 UI
    HOME_PUMP_LEVEL_DISPLAY_LEFT_ADJUST,     /// 左档位调节
    HOME_PUMP_LEVEL_DISPLAY_RIGHT_ADJUST,    /// 右档位调节
    HOME_PUMP_LEVEL_DISPLAY_BOTH_ADJUST,     /// 双档位调节
} home_pump_level_display_type_e;

typedef enum {
    HOME_PUMP_MODE_DISPLAY_DEFAULT = 0,      /// 默认 UI
    HOME_PUMP_MODE_DISPLAY_BF_MODE,          /// 追奶模式
    HOME_PUMP_MODE_DISPLAY_NORMAL_MODE,      /// 普通模式
} home_pump_mode_display_type_e;

typedef enum {
    HOME_LIGHT_STATUS_DEFAULT = 0,         /// 默认
    HOME_LIGHT_STATUS_MODE_SELECT,         /// 模式选择闪烁
} home_light_status_e;

/// 主界面UI结构体
typedef struct {
    window_context_t w_ctx;
    window_entry_extra_t extra;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    ble_manager_context_t *ble;
    battery_manager_context_t *battery;
    key_input_context_t *key;
    led_manager_context_t *led;
    breast_pump_manager_context_t *pump;
    airplane_mode_context_t *airplane_mode;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t window_run_state; /// WM是否运行.
    uint16_t current_ui_type; /// 当前UI类型. home_ui_type_e
    uint16_t popup_ui_type; /// 当前弹窗类型. home_popup_ui_type_e
    uint8_t exit_current_ui; /// 退出当前UI标志.
    uint8_t pump_priming; /// 泵奶准备标志.
    /// 模式相关
    uint8_t ui_mode_index; /// 当前模式索引.
    uint8_t pump_mode_display_type; /// 当前泵奶模式显示类型
    uint32_t pump_mode_adjust_time; /// 泵奶模式调节显示时间
    /// 档位调节相关
    uint8_t level_adjust_pump_num;   /// 档位调节泵数
    uint8_t level_adjust_pump_index; /// 档位调节泵索引
    uint8_t pump_level_display_type; /// 当前泵奶档位调节类型
    pump_setting_t setting; /// 当前泵奶设置信息.
    uint32_t level_adjust_time; /// 档位调节显示时间
    /// 夜灯调节相关
    uint8_t night_light_gear; /// 夜灯调节亮度等级
    home_page_display_type_e home_display_type;
    uint32_t pump_record_time;
    /// 灯光相关
    uint8_t home_light_status; /// 主界面灯光状态.
    uint8_t state_blink_visible; /// 状态闪烁可见: 0: 隐藏相, 1: 显示相.
    uint8_t mode_select_blink_visible; /// 模式选择 start 灯上次闪烁相位.
    uint32_t blink_start_time; /// 闪烁起始时间
} ui_proc_home_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);
void start_home_idle_ui_animation(ui_proc_home_t *proc);
void stop_home_common_ui(ui_proc_home_t *proc);

/// 模式选择界面按键处理
int home_pump_mode_adjust_handler(ui_proc_home_t *proc, bool is_bf_mode);
void home_pump_mode_adjust_enter(ui_proc_home_t *proc, bool is_bf_mode);
void home_pump_mode_adjust_timeout_handler(ui_proc_home_t *proc);
void home_pump_mode_adjust_ui_exit(ui_proc_home_t *proc);
void home_pump_mode_adjust_exit_handler(ui_proc_home_t *proc);

/// 档位调节功能处理
int home_pump_level_adjust_handler(ui_proc_home_t *proc, bool is_increase);
int home_pump_level_adjust_type_update(ui_proc_home_t *proc, uint8_t is_left);
void home_pump_level_adjust_timeout_handler(ui_proc_home_t *proc);
void home_pump_level_adjust_exit_handler(ui_proc_home_t *proc);

/// 弹窗类型更新
void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type);

/// 主界面UI切换
void home_ui_change_hdr_remove(ui_proc_home_t *proc);

/// 泵奶结果界面处理
void show_home_pump_result_ui(ui_proc_home_t *proc);
void home_pump_result_ui_exit(ui_proc_home_t *proc);

/// 夜灯调节界面处理
void show_home_night_light_adjust_ui(ui_proc_home_t *proc);
void switch_home_night_light_adjust(ui_proc_home_t *proc, bool is_increase);
void home_night_light_adjust_ui_exit(ui_proc_home_t *proc);

/// 取消按键处理
void home_cancel_key_event_handler(ui_proc_home_t *proc);

/// 主界面UI显示类型更新
void home_page_display_type_update(ui_proc_home_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


