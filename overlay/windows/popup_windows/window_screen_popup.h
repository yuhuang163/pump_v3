#ifndef __WINDOW_SCREEN_POPUP_H__
#define __WINDOW_SCREEN_POPUP_H__

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
#define POPUP_ANIMATION_FREQ                 (25)
#else
#define POPUP_ANIMATION_FREQ                 (5)
#endif

/// pop动画展示时间
#define POPUP_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// logo UI类型枚举
typedef enum {
    POPUP_UI_TYPE_NONE = 0,
    POPUP_UI_TYPE_HOME,
    POPUP_UI_TYPE_MAX
} popup_ui_type_e;

/// logo UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_entry_extra_t extra;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    power_manager_context_t *pm;
    ui_manager_context_t *ui;
    storage_manager_context_t *st;
    key_input_context_t *key;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    popup_window_id_e popup_window_id; /// 弹出框窗口ID.
    uint16_t current_ui_type; /// 当前UI类型. popup_ui_type_e
} ui_proc_popup_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_popup_ui_animation(ui_proc_popup_t *proc);
void stop_popup_ui_animation(ui_proc_popup_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


