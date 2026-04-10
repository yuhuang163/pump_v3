#ifndef __WINDOW_SCREEN_SYSTEM_H__
#define __WINDOW_SCREEN_SYSTEM_H__

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
#define SYSTEM_ANIMATION_FREQ                 (25)
#else
#define SYSTEM_ANIMATION_FREQ                 (5)
#endif

/// 系统动画展示时间
#define SYSTEM_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// 系统UI类型枚举
typedef enum {
    SYSTEM_UI_TYPE_NONE = 0,
    SYSTEM_UI_TYPE_POWER_OFF,
    SYSTEM_UI_TYPE_FACTORY_RESET_CONFIRM, /// 工厂重置确认界面
    SYSTEM_UI_TYPE_FACTORY_RESET, /// 工厂重置界面
    SYSTEM_UI_TYPE_MAX
} system_ui_type_e;

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
    key_input_context_t *key;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. factory_reset_ui_type_e
    uint8_t display_fac_rst_confirm_progress; /// 显示工厂重置确认进度.
    uint8_t rsv[13];
} ui_proc_system_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void system_factory_reset_handle(ui_proc_system_t *proc);
void factory_reset_ui_hdr_remove(void);

void system_power_off_handle(ui_proc_system_t *proc);
void power_off_ui_hdr_remove(void);

void system_factory_reset_confirm_handle(ui_proc_system_t *proc);
void system_factory_reset_confirm_keyevent_handler(ui_proc_system_t *proc, uint16_t keycode, uint16_t keytype);
void factory_reset_confirm_ui_hdr_remove(void);

void stop_system_ui_animation(ui_proc_system_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


