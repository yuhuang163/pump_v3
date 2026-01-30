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
#if IS_SUPPORT_LVGL
#define PUMP_ANIMATION_FREQ                    (25)
#define PUMP_SPEED_ANIMATION_FREQ              (20) 
#define PUMP_RESULT_ANIMATION_FREQ             (20) 
#define PUMP_MODE_ANIMATION_FREQ               (20) 
#define PUMP_LEVEL_ANIMATION_FREQ              (20) 
#else
#define PUMP_ANIMATION_FREQ                    (5)
#define PUMP_SPEED_ANIMATION_FREQ              (5) 
#define PUMP_RESULT_ANIMATION_FREQ             (5) 
#define PUMP_MODE_ANIMATION_FREQ               (5) 
#define PUMP_LEVEL_ANIMATION_FREQ              (5) 
#endif

/// 判断泵奶时间
#define IS_PUMPTIME_LESS_THEN_30S(result)      ((result)->real_valid_time_sec < 30 )
#define IS_PUMPTIME_LESS_THEN_120S(result)     ((result)->real_valid_time_sec < 120)
/// 循环显示超时时间
#define LOOP_SHOW_TIMEOUT                      (5000) /// 5s
/// 泵奶子UI显示时间
#define PUMP_SUB_UI_DISPLAY_TIME               (2000) /// 4s
/// 泵奶运行时长更新间隔
#define PUMP_RUNNING_TIME_UPDATE_INTERVAL      (1000) /// 1s
/// 泵奶模式切换超时时间
#define PUMP_MODE_CHANGE_TIMEOUT               (2000) /// 2s

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
    PUMP_UI_TYPE_LEVEL,
    PUMP_UI_TYPE_SPEED,
    PUMP_UI_TYPE_BREASTFEEDING_MODE,
    PUMP_UI_TYPE_COMMON_MODE,
    PUMP_UI_TYPE_MAX
} pump_ui_type_e;

/// 泵奶信息结构体
typedef struct {    
    uint8_t common_mode; /// 普通泵奶模式.
    uint8_t status; /// 泵奶状态.
    uint8_t breastfeeding_mode; /// 追奶模式.
    uint8_t control_type; /// 控制类型. 0: 左, 1: 右. 2: 双侧.
} ui_pump_info_t;

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
    uint32_t margic;
    breast_pump_result_t pump_result; /// 泵奶结果.
    uint8_t need_start_pump; /// 是否需要启动泵奶.
    uint8_t ui_show_state; /// 当前UI显示状态.
    uint8_t user_pause; /// 用户是否暂停.
    uint8_t goto_charge_ui_flag; /// 是否需要跳转到充电界面.
    uint8_t is_mode_changed_by_app; /// 是否是从app中切换模式.
    uint8_t is_mode_changed_by_device; /// 是否是从设备中切换模式.
    uint8_t current_ui_mode; /// 当前UI模式索引.
    uint16_t current_ui_type; /// 当前UI类型. pump_ui_type_e
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    ui_pump_info_t info; /// 当前泵奶信息.
    pump_setting_t setting; /// 当前泵奶设置信息.
#if IS_SUPPORT_AIR_LEAKAGE_WARNING
    uint8_t is_air_leakage_popup_show; /// 是否显示空气泄漏弹窗提示.
    uint8_t hide_air_leakage_popup_wm; /// 是否隐藏空气泄漏弹窗提示.
#endif
} ui_proc_pump_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_pump_home_ui(ui_proc_pump_t *proc);
int switch_pump_common_mode_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, breast_pump_info_t *pump_info);
int switch_pump_night_light_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump);
int switch_pump_status_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, int status);
int switch_pump_frequency_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, breast_pump_info_t *pump_info);
int switch_breastfeeding_mode_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, breast_pump_info_t *pump_info);
int switch_pump_control_type_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, uint8_t control_type);
int switch_pump_level_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, breast_pump_info_t *pump_info, uint8_t is_left);
void show_pump_progress_time_policy(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump);
void pump_result_ui_hdr_remove(void);
void pump_level_ui_hdr_remove(void);
void pump_speed_ui_hdr_remove(void);
void pump_mode_ui_hdr_remove(void);
void breastfeeding_mode_ui_hdr_remove(void);
void stop_pump_ui_animation(ui_proc_pump_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


