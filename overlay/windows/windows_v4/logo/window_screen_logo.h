#ifndef __WINDOW_SCREEN_LOGO_H__
#define __WINDOW_SCREEN_LOGO_H__

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

/// logo动画频率
#if IS_SUPPORT_LVGL
#define LOGO_ANIMATION_FREQ                 (25)
#else
#define LOGO_ANIMATION_FREQ                 (5)
#endif

/// logo动画展示时间
#define LOGO_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// logo UI类型枚举
typedef enum {
    LOGO_UI_TYPE_NONE = 0,
    LOGO_UI_TYPE_HOME,
    LOGO_UI_TYPE_MAX
} logo_ui_type_e;

/// logo UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    power_manager_context_t *pm;
    ui_manager_context_t *ui;
    airplane_mode_context_t *airplane;
    battery_manager_context_t *battery;
    led_manager_context_t *led;
    ble_manager_context_t *ble;
    key_input_context_t *key;
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. logo_ui_type_e
#if IS_SUPPORT_LOGO_BG_COLOR_TEST
    uint8_t test_page_count;
#endif
} ui_proc_logo_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_logo_ui_animation(ui_proc_logo_t *proc);
void stop_logo_ui_animation(ui_proc_logo_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


