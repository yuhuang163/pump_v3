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

#if IS_SUPPORT_LVGL
#define HOME_ANIMATION_FREQ                 (25)
#else
#define HOME_ANIMATION_FREQ                 (5)
#endif

/// 主界面动画展示时间
#define HOME_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// 主界面绑定二维码展示时长
#define HOME_BIND_QRCODE_DISPLAY_TIMEOUT    (3000) /// 3s

/// 模式选择界面动画展示时间
#define HOME_MODE_SELECT_DISPLAY_TIME       (10000) /// 10s

/// 档位调节界面动画展示时间
#define HOME_PUMP_LEVEL_DISPLAY_TIMEOUT     (3000) /// 3s

/// 速度调节界面动画展示时间
#define HOME_PUMP_SPEED_DISPLAY_TIMEOUT     (3000) /// 3s

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
    HOME_UI_TYPE_BIND_QRCODE,
    HOME_UI_TYPE_BF_MODE_SELECT,
    HOME_UI_TYPE_NORMAL_MODE_SELECT,
    HOME_UI_TYPE_LEVEL_ADJUST,
    HOME_UI_TYPE_SPEED_ADJUST,
    HOME_UI_TYPE_PUMP_RESULT,
    HOME_UI_TYPE_MAX
} home_ui_type_e;

typedef enum {
    HOME_UI_DISPLAY_DEFAULT = 0,       /// 默认 UI
    HOME_UI_DISPLAY_RECORD,            /// 显示记录 UI
    HOME_UI_DISPLAY_CLOCK,             /// 显示时钟 UI
    HOME_UI_DISPLAY_CLOCK_AND_RECORD,  /// 显示时钟和记录 UI
    HOME_UI_DISPLAY_CLOCK_LIGHT_TIP,   /// 显示时钟夜灯提示
    HOME_UI_DISPLAY_MAX
} home_page_display_type_e;

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
    broadcast_obj_t receiver;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t ui_bf_mode_index; /// 追奶模式索引.
    uint8_t ui_normal_mode_index; /// 普通模式索引.
    uint16_t current_ui_type; /// 当前UI类型. home_ui_type_e
    uint16_t popup_ui_type; /// 当前弹窗类型. home_popup_ui_type_e
    pump_setting_t setting; /// 当前泵奶设置信息.
    uint8_t control_type; /// 当前控制类型.
    int8_t level_change; /// 当前档位变化值.
    uint8_t level_sync_flag; /// 档位同步标志.
    uint8_t speed_sync_flag; /// 速度同步标志.
    uint8_t pump_priming;
    home_page_display_type_e home_display_type;
    uint32_t pump_record_time;
} ui_proc_home_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);
void start_home_idle_ui_animation(ui_proc_home_t *proc);
void stop_home_common_ui(ui_proc_home_t *proc);
void home_start_key_event_handler(ui_proc_home_t *proc);

/// 模式选择界面按键处理
void switch_home_pump_normal_mode(ui_proc_home_t *proc, bool is_increase);
void home_pump_normal_mode_ui_hdr_remove(void);
void home_pump_normal_mode_key_click_handler(ui_proc_home_t *proc);
/// 追奶模式选择界面按键处理
void switch_home_pump_bf_mode(ui_proc_home_t *proc, bool is_increase);
void home_pump_bf_mode_ui_hdr_remove(void);
void home_pump_bf_mode_key_click_handler(ui_proc_home_t *proc);
/// 档位调节界面按键处理
void switch_home_pump_level_control_type(ui_proc_home_t *proc, uint8_t control_type);
void switch_home_pump_level(ui_proc_home_t *proc, bool is_increase);
void home_pump_level_ui_hdr_remove(void);
/// 速度调节界面按键处理
void switch_home_pump_speed(ui_proc_home_t *proc, bool is_increase);
void home_pump_speed_ui_hdr_remove(void);
void home_pump_speed_key_click_handler(ui_proc_home_t *proc);
/// 绑定二维码展示
void start_home_bind_ui_animation(ui_proc_home_t *proc);
void home_bind_ui_hdr_remove(void);
/// 弹窗类型更新
void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type);
/// 主界面UI切换
void home_ui_change_hdr_remove(ui_proc_home_t *proc);
/// 泵奶结果界面处理
void show_home_pump_result_ui(ui_proc_home_t *proc);
void home_pump_result_ui_hdr_remove(void);

#ifdef __cplusplus
}
#endif

#endif 


