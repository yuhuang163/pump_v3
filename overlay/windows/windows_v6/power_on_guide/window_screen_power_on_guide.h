#ifndef __WINDOW_SCREEN_POWER_ON_GUIDE_H__
#define __WINDOW_SCREEN_POWER_ON_GUIDE_H__

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

/// 开机引导动画频率
#if IS_SUPPORT_LVGL
#define POWER_ON_GUIDE_ANIMATION_FREQ                        (25)
#else
#define POWER_ON_GUIDE_ANIMATION_FREQ                        (5)
#endif

/// 长按开机引导页静默超时后直接关机
#define POWER_ON_GUIDE_HOLD_IDLE_TIMEOUT_MS         (5000)
/// PD检查重试次数.
#define POWER_ON_GUIDE_PD_CHECK_RETRY_MAX           (3)
#define POWER_ON_GUIDE_PD_CHECK_RETRY_DELAY_MS      (500)

/// 开机引导 UI动作枚举
typedef enum {
    POWER_ON_GUIDE_HDR_NIGHT_LIGHT_ONOFF = 100,
    POWER_ON_GUIDE_HDR_NIGHT_LIGHT_LEVEL,
    POWER_ON_GUIDE_HDR_NIGHT_LIGHT_ADAPTIVE,
    POWER_ON_GUIDE_HDR_ACTION_MAX
} power_on_guide_ui_hdr_action_e;

#define POWER_ON_GUIDE_LOW_POWER_TASK_NAME      "power_on_guide_low_power"

/// 开机引导 UI类型枚举
typedef enum {
    POWER_ON_GUIDE_UI_TYPE_NONE = 0,
    POWER_ON_GUIDE_UI_TYPE_HOME,
    POWER_ON_GUIDE_UI_TYPE_POWER_ON_HOLD, /// V3 插电/短按未满足开机时长：长按开机引导
    POWER_ON_GUIDE_UI_TYPE_LOW_POWER,     /// 低电/空电提醒充电，不可长按开机
    POWER_ON_GUIDE_UI_TYPE_MAX
} power_on_guide_ui_type_e;

/// 开机引导 UI结构体
typedef struct {
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    power_manager_context_t *pm;
    key_input_context_t *key;
    tests_manager_context_t *tests;
    led_manager_context_t *led;
    ble_manager_context_t *ble;
#if IS_SUPPORT_POWER_ON_GUIDE_UI
    battery_manager_context_t *battery;
#endif
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t enter_power_on_guide_time; /// 进入开机引导时间.
    uint32_t last_progress_ani_time; /// 上次动画进度时间.
    uint16_t current_ui_type; /// 当前UI类型. power_on_guide_ui_type_e
    uint8_t power_on_hold_holding; /// 长按开机引导：电源键按下中.
    uint8_t pd_check_retry_count; /// PD检查重试次数.
    uint8_t rsv[7];
} ui_proc_power_on_guide_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
#if IS_SUPPORT_POWER_ON_GUIDE_UI
void show_power_on_guide_power_on_hold_ui(ui_proc_power_on_guide_t *proc);
void show_power_on_guide_low_power_ui(ui_proc_power_on_guide_t *proc);
void stop_power_on_guide_ui_animation(ui_proc_power_on_guide_t *proc);
void power_on_guide_power_on_hold_ui_hdr_remove(void);
void power_on_guide_low_power_ui_hdr_remove(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
