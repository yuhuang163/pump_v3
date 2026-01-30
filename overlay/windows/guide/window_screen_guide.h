#ifndef __WINDOW_SCREEN_GUIDE_H__
#define __WINDOW_SCREEN_GUIDE_H__

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
#define GUIDE_ANIMATION_FREQ                 (25)
#else
#define GUIDE_ANIMATION_FREQ                 (5)
#endif

/// 主界面动画展示时间
#define GUIDE_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// 主界面UI动作枚举
typedef enum {
    GUIDE_HDR_ACTION_MAX
} guide_ui_hdr_action_e;

/// 主界面UI类型枚举
typedef enum {
    GUIDE_UI_TYPE_NONE = 0,
    GUIDE_UI_TYPE_HOME,
    GUIDE_UI_TYPE_MAX
} guide_ui_type_e;

typedef enum {
    GUIDE_PAGE_GUIDE_QRCODE,
    // GUIDE_PAGE_GUIDE_PHONE,
    GUIDE_PAGE_GUIDE_CHASING_MILK,
    GUIDE_PAGE_GUIDE_ADJUST_LEVEL,
    GUIDE_PAGE_GUIDE_MILK_QUANTITY,
    GUIDE_PAGE_MAX,
} guide_page_index_t;

/// 主界面UI结构体
typedef struct {
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    ble_manager_context_t *ble;
    battery_manager_context_t *battery;
    key_input_context_t *key;
    uint32_t margic; /// 窗口标识.
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. guide_ui_type_e
    uint16_t page_index; /// 当前页面索引.
} ui_proc_guide_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_guide_ui_animation(ui_proc_guide_t *proc);
void stop_guide_ui_animation(ui_proc_guide_t *proc);
void guide_start_key_event_handler(ui_proc_guide_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


