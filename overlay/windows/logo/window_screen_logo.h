#ifndef __WINDOW_SCREEN_LOGO_H__
#define __WINDOW_SCREEN_LOGO_H__

#include "peripheral_bus_api.h"
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

typedef struct ui_proc_logo {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
} ui_proc_logo_t;

/// logo动画展示时间
#define LOGO_ANIMATION_DISPLAY_TIME      (2000) /// 2s

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


