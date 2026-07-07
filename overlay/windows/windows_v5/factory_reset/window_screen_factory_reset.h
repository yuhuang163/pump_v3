#ifndef __WINDOW_SCREEN_FACTORY_RESET_H__
#define __WINDOW_SCREEN_FACTORY_RESET_H__

#include <stdint.h>
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
#define FACTORY_RESET_ANIMATION_FREQ                 (25)
#else
#define FACTORY_RESET_ANIMATION_FREQ                 (5)
#endif

/// 工厂重置动画展示时间
#define FACTORY_RESET_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// 工厂重置确认页 return 键/UI 闪烁周期
#define FACTORY_RESET_BLINK_PERIOD                   (500)

/// 系统UI类型枚举
typedef enum {
    FACTORY_RESET_UI_TYPE_NONE = 0,
    FACTORY_RESET_UI_TYPE_FACTORY_RESET_CONFIRM, /// 工厂重置确认界面
    FACTORY_RESET_UI_TYPE_FACTORY_RESET, /// 工厂重置界面
    FACTORY_RESET_UI_TYPE_MAX
} factory_reset_ui_type_e;

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
    battery_manager_context_t *battery;
    resource_manager_context_t *res;
    airplane_mode_context_t *airplane;

    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. factory_reset_ui_type_e
    uint8_t popup_ui_type; /// 当前弹窗类型. popup_ui_id_e
    uint8_t state_blink_visible; /// 状态闪烁可见: 0: 隐藏相, 1: 显示相.
    uint8_t confirm_return_blink_visible; /// 确认页 return 灯上次闪烁相位.
    uint32_t led_blink_time; /// 上次LED闪烁时间.
    uint16_t led_blink_period; /// LED闪烁周期.
    uint8_t led_blink_status; /// LED闪烁状态.
    uint8_t rsv[11];
} ui_proc_factory_reset_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void factory_reset_handle(ui_proc_factory_reset_t *proc, uint16_t delay_ms);
void factory_reset_ui_hdr_remove(void);
void show_factory_reset_confirm_ui(ui_proc_factory_reset_t *proc);
void factory_reset_confirm_ui_hdr_remove(void);
void factory_reset_confirm_led_restore(ui_proc_factory_reset_t *proc);
void stop_factory_reset_ui_animation(ui_proc_factory_reset_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


