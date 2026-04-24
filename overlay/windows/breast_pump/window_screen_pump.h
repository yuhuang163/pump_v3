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
#if (ROOT_UI_VERSION == ROOT_UI_V2)
#define PUMP_ANIMATION_FREQ                    (24)
#else
#define PUMP_ANIMATION_FREQ                    (16)
#endif
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
#define PUMP_SUB_UI_DISPLAY_TIME               (5000) /// 5s
/// 泵奶运行时长更新间隔
#define PUMP_RUNNING_TIME_UPDATE_INTERVAL      (1000) /// 1s
/// 泵奶模式切换超时时间
#define PUMP_MODE_CHANGE_TIMEOUT               (2000) /// 2s
/// 泵奶UI显示超时时间
#define PUMP_MODE_DISPLAY_TIMEOUT              (10000) /// 10s
#define PUMPING_MODE_DISPLAY_TIMEOUT           (5000) /// 5s
/// 泵奶速度显示超时时间
#define PUMP_SPEED_DISPLAY_TIMEOUT             (3000) /// 3s
#define PUMP_SPEED_SYNC_TIMEOUT                (1000) /// 3s 
/// 泵奶档位显示超时时间
#define PUMP_LEVEL_DISPLAY_TIMEOUT             (3000) /// 3s 
#define PUMP_LEVEL_SYNC_TIMEOUT                (1000) /// 3s 
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
    PUMP_UI_TYPE_BOTH_LEVEL,
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
    uint8_t ui_show_state; /// 当前UI显示状态.
    uint8_t user_pause; /// 用户是否暂停.
    uint8_t goto_charge_ui_flag; /// 是否需要跳转到充电界面.
    uint8_t is_mode_changed_by_app; /// 是否是从app中切换模式.
    uint8_t is_mode_changed_by_device; /// 是否是从设备中切换模式.
    uint8_t ui_normal_mode_index; /// UI普通模式索引.
    uint8_t ui_bf_mode_index; /// UI追奶模式索引.
    uint16_t current_ui_type; /// 当前UI类型
    uint16_t start_ui_type;   
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    ui_pump_info_t info; /// 当前泵奶信息.
    pump_setting_t setting; /// 当前泵奶设置信息.
    int8_t level_change; /// 当前档位变化值.
    // int8_t level_adjust_too_large; /// 档位范围调整过大值.
    uint8_t level_sync_flag; /// 档位同步标志.
    uint8_t speed_sync_flag; /// 速度同步标志.
    uint8_t mode_sync_flag; /// 追奶模式同步标志.
#if IS_SUPPORT_AIR_LEAKAGE_WARNING
    uint8_t is_air_leakage_popup_show; /// 是否显示空气泄漏弹窗提示.
    uint8_t hide_air_leakage_popup_wm; /// 是否隐藏空气泄漏弹窗提示.
#endif
    uint8_t forbid_start_pump; /// 是否禁止开始泵奶.
    uint8_t goto_pump_home_ui; /// 跳转到泵奶主界面标志.
    uint8_t pump_will_finish; /// 泵奶即将完成标志.
    uint16_t popup_ui_type; /// 当前弹窗类型. popup_ui_id_e
} ui_proc_pump_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/// 显示泵奶主界面
void show_pump_home_ui(ui_proc_pump_t *proc);
/// 普通模式UI相关
void pump_mode_key_click_handler(ui_proc_pump_t *proc);
void switch_pump_common_mode_ui(ui_proc_pump_t *proc, bool is_increase);
/// 追奶模式UI相关
void breastfeeding_mode_key_click_handler(ui_proc_pump_t *proc);
void switch_breastfeeding_mode_ui(ui_proc_pump_t *proc, bool is_increase);
/// 速度UI相关
void pump_speed_key_click_handler(ui_proc_pump_t *proc);
void switch_pump_speed_ui(ui_proc_pump_t *proc, bool is_increase);
/// 档位UI相关
void switch_pump_level_ui(ui_proc_pump_t *proc, bool is_increase);
void switch_pump_control_type_ui(ui_proc_pump_t *proc, uint8_t control_type);
void switch_pump_both_control_type_ui(ui_proc_pump_t *proc);
void switch_pump_both_level_ui(ui_proc_pump_t *proc, bool is_increase);
/// 泵奶状态UI相关
void switch_pump_status_ui(ui_proc_pump_t *proc);
void show_pump_result_ui(ui_proc_pump_t *proc);
void pump_result_go_to_home_window(ui_proc_pump_t *proc);
/// 移除UI相关
void pump_result_ui_hdr_remove(void);
void pump_level_ui_hdr_remove(void);
void pump_both_level_ui_hdr_remove(void);
void pump_speed_ui_hdr_remove(void);
void pump_mode_ui_hdr_remove(void);
void breastfeeding_mode_ui_hdr_remove(void);
void pump_ui_change_hdr_remove(ui_proc_pump_t *proc);
void stop_pump_ui_animation(ui_proc_pump_t *proc);
/// 按键处理相关
void pump_power_key_handler(ui_proc_pump_t *proc);
/// 泵奶完成事件处理
void pump_complete_event_handler(ui_proc_pump_t *proc);
/// 弹窗类型更新
void pump_popup_ui_type_update(ui_proc_pump_t *proc, popup_ui_id_e type);
/// 子UI退出处理
void pump_sub_ui_exit_handler(ui_proc_pump_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


