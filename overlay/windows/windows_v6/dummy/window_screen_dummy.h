#ifndef __WINDOW_SCREEN_DUMMY_H__
#define __WINDOW_SCREEN_DUMMY_H__

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

/// dummy 动画频率
#if IS_SUPPORT_LVGL
#define DUMMY_ANIMATION_FREQ                        (25)
#else
#define DUMMY_ANIMATION_FREQ                        (5)
#endif

/// 长按开机引导页静默超时后直接关机
#define DUMMY_POWER_ON_HOLD_IDLE_TIMEOUT_MS         (5000)

/// dummy UI动作枚举
typedef enum {
    DUMMY_HDR_NIGHT_LIGHT_ONOFF = 100,
    DUMMY_HDR_NIGHT_LIGHT_LEVEL,
    DUMMY_HDR_NIGHT_LIGHT_ADAPTIVE,
    DUMMY_HDR_ACTION_MAX
} dummy_ui_hdr_action_e;

/// dummy UI类型枚举
typedef enum {
    DUMMY_UI_TYPE_NONE = 0,
    DUMMY_UI_TYPE_HOME,
    DUMMY_UI_TYPE_POWER_ON_HOLD, /// V3 插电/短按未满足开机时长：长按开机引导
    DUMMY_UI_TYPE_MAX
} dummy_ui_type_e;

/// dummy UI结构体
typedef struct {
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    power_manager_context_t *pm;
    key_input_context_t *key;
    tests_manager_context_t *tests;
    led_manager_context_t *led;
    ble_manager_context_t *ble;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t enter_dummy_time; /// 进入dummy时间.
    uint32_t last_progress_ani_time; /// 上次动画进度时间.
    uint16_t current_ui_type; /// 当前UI类型. dummy_ui_type_e
    uint8_t power_on_hold_holding; /// 长按开机引导：电源键按下中.
    uint8_t rsv[7];
} ui_proc_dummy_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
#if IS_SUPPORT_POWER_ON_GUIDE_UI
void show_dummy_power_on_hold_ui(ui_proc_dummy_t *proc);
void stop_dummy_ui_animation(ui_proc_dummy_t *proc);
void dummy_power_on_hold_ui_hdr_remove(void);
#endif

#ifdef __cplusplus
}
#endif

#endif 
