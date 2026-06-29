#ifndef __WINDOW_SCREEN_THEME_GUIDE_H__
#define __WINDOW_SCREEN_THEME_GUIDE_H__

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
#include "resource_manager_api.h" // IWYU pragma: keep

#if IS_SUPPORT_LVGL
#define THEME_GUIDE_ANIMATION_FREQ                 (25)
#else
#define THEME_GUIDE_ANIMATION_FREQ                 (5)
#endif

typedef enum {
    THEME_GUIDE_SELECT_SIMPLE = 0,
    THEME_GUIDE_SELECT_IP = 1,
    THEME_GUIDE_SELECT_MAX,
} theme_guide_select_e;

typedef enum {
    THEME_GUIDE_UI_TYPE_NONE = 0,
    THEME_GUIDE_UI_TYPE_HOME,
    THEME_GUIDE_UI_TYPE_MAX
} theme_guide_ui_type_e;

typedef struct {
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    battery_manager_context_t *battery;
    led_manager_context_t *led;
    key_input_context_t *key;
    storage_manager_context_t *storage;
    uint32_t margic;
    uint8_t window_run_state;
    uint32_t last_progress_ani_time;
    uint16_t current_ui_type;
    uint8_t selected_index;
    uint8_t is_operated;
    uint8_t rsv[14];
} ui_proc_theme_guide_t;

static inline res_theme_type_e theme_guide_proc_get_selected_theme_type(const ui_proc_theme_guide_t *proc)
{
    if (proc == NULL || proc->selected_index != THEME_GUIDE_SELECT_IP) {
        return RES_THEME_TYPE_BASE;
    }
    return RES_THEME_TYPE_CUSTOM;
}

void start_theme_guide_ui_animation(ui_proc_theme_guide_t *proc);
void stop_theme_guide_ui_animation(ui_proc_theme_guide_t *proc);
void theme_guide_knob_event_handler(ui_proc_theme_guide_t *proc, bool is_left);
void theme_guide_key_event_handler(ui_proc_theme_guide_t *proc, uint16_t keycode);

#ifdef __cplusplus
}
#endif

#endif
