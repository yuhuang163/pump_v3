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

#if IS_SUPPORT_LVGL
#define HOME_ANIMATION_FREQ                 (25)
#else
#define HOME_ANIMATION_FREQ                 (5)
#endif

/// 主界面动画展示时间
#define HOME_ANIMATION_DISPLAY_TIME         (2000) /// 2s

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
    HOME_UI_TYPE_MAX
} home_ui_type_e;

/// 主界面UI结构体
typedef struct {
    window_context_t w_ctx;
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
    uint16_t current_ui_type; /// 当前UI类型. home_ui_type_e
} ui_proc_home_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);
void start_home_idle_ui_animation(ui_proc_home_t *proc);
void stop_home_common_ui(ui_proc_home_t *proc);
void home_start_key_event_handler(ui_proc_home_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


