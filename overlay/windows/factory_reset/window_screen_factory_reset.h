#ifndef __WINDOW_SCREEN_FACTORY_RESET_H__
#define __WINDOW_SCREEN_FACTORY_RESET_H__

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

/// 工厂重置动画频率
#if IS_SUPPORT_LVGL
#define FACTORY_RESET_ANIMATION_FREQ                 (25)
#else
#define FACTORY_RESET_ANIMATION_FREQ                 (5)
#endif

/// 工厂重置动画展示时间
#define FACTORY_RESET_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// 工厂重置UI类型枚举
typedef enum {
    FACTORY_RESET_UI_TYPE_NONE = 0,
    FACTORY_RESET_UI_TYPE_HOME,
    FACTORY_RESET_UI_TYPE_MAX
} factory_reset_ui_type_e;

/// 工厂重置UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    power_manager_context_t *pm;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. factory_reset_ui_type_e
} ui_proc_factory_reset_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_factory_reset_ui_animation(ui_proc_factory_reset_t *proc);
void stop_factory_reset_ui_animation(ui_proc_factory_reset_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


