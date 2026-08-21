#ifndef __WINDOW_SCREEN_AIRPLANE_MODE_H__
#define __WINDOW_SCREEN_AIRPLANE_MODE_H__

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

/// 飞行模式动画频率
#if IS_SUPPORT_LVGL
#define AIRPLANE_MODE_ANIMATION_FREQ                    (25)
#else
#define AIRPLANE_MODE_ANIMATION_FREQ                    (5)
#endif
/// 飞行模式超时时间
#if IS_SUPPORT_PUMP_MODE_FAST_TEST
#define AIRPLANE_MODE_TIMEOUT_TIME                      (20 * 1000) /// 5秒
#else
#define AIRPLANE_MODE_TIMEOUT_TIME                      (5 * 60 * 1000) /// 5分钟
#endif
/// 飞行模式显示超时时间
#define AIRPLANE_MODE_DISPLAY_TIMEOUT                   (2000) /// 2s

/// 飞行模式UI动作枚举
typedef enum {
    AIRPLANE_MODE_HDR_ACTION_REFRESH_CHARGE_UI = 0,
    AIRPLANE_MODE_HDR_ACTION_MAX
} airplane_mode_ui_hdr_action_e;

/// 飞行模式UI类型枚举
typedef enum {
    AIRPLANE_MODE_UI_TYPE_NONE = 0,
    AIRPLANE_MODE_UI_TYPE_HOME,
    AIRPLANE_MODE_UI_TYPE_POWER_OFF_HOLD, /// 长按 power/start 关机确认
    AIRPLANE_MODE_UI_TYPE_MAX
} airplane_mode_ui_type_e;

/// 飞行模式UI结构体
typedef struct ui_proc_airplane_mode {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    battery_manager_context_t *battery;
    ble_manager_context_t *ble;
    power_manager_context_t *pm;
    key_input_context_t *key;
    breast_pump_manager_context_t *pump;
    led_manager_context_t *led;
    airplane_mode_context_t *airplane;
    resource_manager_context_t *res;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t window_run_state; /// WM是否运行.
    uint16_t current_ui_type; /// 当前UI类型. airplane_mode_ui_type_e
    uint32_t power_off_idle_time; /// 关机倒计时起始时间.
    uint8_t popup_ui_type; /// 当前弹窗类型. popup_ui_id_e
    uint8_t power_off_hold_holding; /// 关机确认页是否处于按住倒计时
} ui_proc_airplane_mode_t;

/// 飞行模式背光灯状态结构体
typedef struct {    
  uint8_t id; /// 背光灯索引
  uint8_t level; /// 背光灯状态
} airplane_mode_led_status_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_airplane_mode_ui(ui_proc_airplane_mode_t *proc);
void stop_airplane_mode_ui(ui_proc_airplane_mode_t *proc);
void airplane_mode_led_policy(ui_proc_airplane_mode_t *proc, uint8_t status);
void airplane_mode_pump_policy(ui_proc_airplane_mode_t *proc);

void airplane_mode_ui_hdr_remove(void);

/// 长按 power/start 关机确认页
void show_airplane_mode_power_off_hold_ui(ui_proc_airplane_mode_t *proc);
void airplane_mode_power_off_hold_ui_exit(ui_proc_airplane_mode_t *proc);
void airplane_mode_power_off_hold_dismiss(ui_proc_airplane_mode_t *proc);
void airplane_mode_power_off_hold_key_long_2s_handler(ui_proc_airplane_mode_t *proc);
void airplane_mode_power_off_hold_key_up_handler(ui_proc_airplane_mode_t *proc);
#if IS_SUPPORT_LOW_POWER_FORBID_START_TIP
void airplane_mode_forbid_start_tip_sync(ui_proc_airplane_mode_t *proc);
uint8_t airplane_mode_forbid_start_key_is_blocked(ui_proc_airplane_mode_t *proc,
    uint16_t keycode, uint16_t keytype);
#endif
#ifdef __cplusplus
}
#endif

#endif 


