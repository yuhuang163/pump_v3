#ifndef __WINDOW_SCREEN_POWER_OFF_H__
#define __WINDOW_SCREEN_POWER_OFF_H__

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

/// 系统动画频率
#if IS_SUPPORT_LVGL
#define POWER_OFF_ANIMATION_FREQ                 (25)
#else
#define POWER_OFF_ANIMATION_FREQ                 (5)
#endif

/// 系统动画展示时间
#define POWER_OFF_ANIMATION_DISPLAY_TIME         (2000) /// 2s

typedef enum {
    POWER_OFF_HDR_EXIT_CHARGE_POLICY = 100, /// 退出充电策略
    POWER_OFF_HDR_DISCHARGE_POWER_OFF, /// 拔除充电关机
    POWER_OFF_HDR_DISPLAY_CHARGING, /// 显示充电中
    POWER_OFF_HDR_DISPLAY_CHARGE_ERROR, /// 显示充电错误
    POWER_OFF_HDR_ACTION_MAX
} power_off_ui_hdr_action_e;

/// 系统UI类型枚举
typedef enum {
    POWER_OFF_UI_TYPE_NONE = 0,
    POWER_OFF_UI_TYPE_POWER_OFF,
    POWER_OFF_UI_TYPE_CHARGING,
    POWER_OFF_UI_TYPE_CHARGE_ERROR,
    POWER_OFF_UI_TYPE_MAX
} power_off_ui_type_e;

/// 系统UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_entry_extra_t extra;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    power_manager_context_t *pm;
    led_manager_context_t *led;
    breast_pump_manager_context_t *pump;
    battery_manager_context_t *battery;
    ble_manager_context_t *ble;
    key_input_context_t *key;
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. factory_reset_ui_type_e
    uint8_t rsv[6];
} ui_proc_power_off_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void power_off_handle(ui_proc_power_off_t *proc);
void start_power_off_ui_animation(ui_proc_power_off_t *proc);
void stop_power_off_common_ui(ui_proc_power_off_t *proc);
void stop_power_off_ui_animation(ui_proc_power_off_t *proc);
void show_power_off_charging_ui(ui_proc_power_off_t *proc);
void show_power_off_charge_error_ui(ui_proc_power_off_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


