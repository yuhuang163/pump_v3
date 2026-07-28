#ifndef __WINDOW_SCREEN_HOME_H__
#define __WINDOW_SCREEN_HOME_H__

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

#define HOME_ANIMATION_FREQ                    (25)

/// 主界面绑定二维码展示时长
#define HOME_BIND_QRCODE_DISPLAY_TIMEOUT       (3000) /// 3s

/// 档位调节界面动画展示时间
#define HOME_PUMP_LEVEL_DISPLAY_TIMEOUT        (5000) /// 5s

/// 泵奶结果显示超时时间
#define HOME_PUMP_RESULT_DISPLAY_TIMEOUT       (3000) /// 3s

/// 模式切换成功页展示时长
#define HOME_SETTINGS_SUCCESS_DISPLAY_TIMEOUT  (1500) /// 1.5s

/// 夜灯调节界面动画展示时间
#define HOME_NIGHT_LIGHT_ADJUST_SYNC_TIMEOUT   (3000) /// 3s

/// UTC 校准失效时间
#define HOME_UTC_CALIB_EXPIRE_INTERVAL_SEC      (20 * 24 * 3600)

/// 主界面状态闪烁半周期(ms)，与泵奶界面同频
#define HOME_BLINK_PERIOD                       (500)

/// 主界面UI类型枚举
typedef enum {
    HOME_UI_TYPE_NONE = 0,
    HOME_UI_TYPE_HOME,
    HOME_UI_TYPE_HOME_IDLE,
    HOME_UI_TYPE_PUMP_RESULT,
    HOME_UI_TYPE_SETTINGS_SUCCESS, /// 模式切换成功
    HOME_UI_TYPE_NIGHT_LIGHT_ADJUST, /// 夜灯调节
    HOME_UI_TYPE_MAX
} home_ui_type_e;

typedef enum {
    HOME_MODE_PAGE_INDEX_NONE = 0,                   /// 无模式选择
    HOME_MODE_PAGE_INDEX_STIMULATION_SELECT = 1,     /// 刺激模式选择
    HOME_MODE_PAGE_INDEX_EXPRESSION_SELECT = 2,      /// 吸乳模式选择
    HOME_MODE_PAGE_INDEX_MILK_BOOST_SELECT = 3,      /// 奶量提升选择
    HOME_MODE_PAGE_INDEX_POWER_PUMPING_SELECT = 4,   /// Power Pumping选择
    HOME_MODE_PAGE_INDEX_CUSTOMIZE_SELECT = 5,       /// 个性化选择
} home_mode_page_index_e;

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
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t window_run_state; /// WM是否运行.
    uint16_t current_ui_type; /// 当前UI类型. home_ui_type_e
    uint16_t popup_ui_type; /// 当前弹窗类型. home_popup_ui_type_e
    
    /// 模式相关
    uint16_t ui_mode_page_index; /// 当前UI模式页面索引.

    /// 档位调节相关
    pump_setting_t setting; /// 当前泵奶设置信息.

    /// 夜灯调节相关
    uint8_t night_light_gear; /// 夜灯调节亮度等级
    uint32_t pump_record_time;

    /// 灯光相关
    uint8_t home_light_status; /// 主界面灯光状态.
} ui_proc_home_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);
void start_home_idle_ui_animation(ui_proc_home_t *proc);
void stop_home_common_ui(ui_proc_home_t *proc);

/// 模式选择界面按键处理
int home_mode_adjust_handler(ui_proc_home_t *proc, bool is_left);
int home_mode_confirm_handler(ui_proc_home_t *proc);
int home_mode_set_handler(ui_proc_home_t *proc, uint8_t mode_index);
void home_mode_page_index_clamp(ui_proc_home_t *proc);

/// 档位调节功能处理

/// 弹窗类型更新
void home_popup_ui_type_update(ui_proc_home_t *proc, popup_ui_id_e type);

/// 主界面UI切换
void home_ui_change_hdr_remove(ui_proc_home_t *proc);

/// 泵奶结果界面处理
void show_home_pump_result_ui(ui_proc_home_t *proc);
void home_pump_result_ui_exit(ui_proc_home_t *proc);

/// 模式切换成功页
void show_home_settings_success_ui(ui_proc_home_t *proc);
void home_settings_success_ui_exit(ui_proc_home_t *proc);

/// 夜灯调节界面处理
void show_home_night_light_adjust_ui(ui_proc_home_t *proc);
void switch_home_night_light_adjust(ui_proc_home_t *proc, bool is_increase);
void home_night_light_adjust_ui_exit(ui_proc_home_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


