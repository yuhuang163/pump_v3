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

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

/// 窗口进入相关.
window_enter_result_e enter_new_window_dummy_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_logo_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_home_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_charge_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_battery_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_breath_pump_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_dental_plaque_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_bt_connect_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_settings_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_settings_submenu_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_ota_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_greeting_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_factory_mode_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_popup_window_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_special_mode_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_themes_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_expert_mode_screen(window_manager_context_t *wm, uint8_t window_id);
window_enter_result_e enter_new_window_customized_key_screen(window_manager_context_t *wm, uint8_t window_id);

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


