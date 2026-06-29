#ifndef __WINDOWS_MAP_DEFINE_H__
#define __WINDOWS_MAP_DEFINE_H__


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "windows_manager_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep

#define CONCAT_MACRO(a, b) a##b

#define CONCAT_MACRO_3(a, b, c) a##b##c

#define UI_STRING_IFY(name) (#name)

#define PAGE_CONTEXT_NAME(name) CONCAT_MACRO(page_context_, name)

#define PAGE_HANDLE_NAME(name) CONCAT_MACRO_3(enter_new_window_, name, _screen)

#define PAGE_CONTEXT_NAME_STR(name) UI_STRING_IFY(name)

typedef enum {
    HOME_SUB_UI_TYPE_NONE = 0,
    HOME_SUB_UI_TYPE_PUMP_RESULT = 1,
    HOME_SUB_UI_TYPE_MAX
} home_sub_ui_type_e;

typedef enum {
    WINDOW_RUN_STATE_PAUSED = 100,
    WINDOW_RUN_STATE_RUNNING = 101,
} window_run_state_e;

/// 窗口广播投递参数
typedef struct {
    void *context;
    uint32_t code;
    int32_t arg1;
    int32_t arg2;
    broadcast_priv_data_t priv_data;
} window_broadcast_args_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

/// 窗口进入相关.
window_enter_result_e enter_new_window_dummy_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_settings_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_pump_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_guide_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_home_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_charge_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_battery_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_logo_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_ota_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_popup_window_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_airplane_mode_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_product_test_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_factory_reset_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_power_off_screen(window_manager_context_t *wm, uint8_t window_id);

/// lvgl 相关.
#if LV_USE_DEMO_BENCHMARK
window_enter_result_e enter_new_window_lvgl_benchmark(window_manager_context_t *wm, uint8_t window_id);
#endif

/// 窗口名称相关.
const char *to_window_name(uint8_t window_id);

/// 窗口状态列表相关.
void get_window_state_hdr_list(window_context_handle_t **list, uint16_t *list_len);
#ifdef __cplusplus
}
#endif

#endif 


