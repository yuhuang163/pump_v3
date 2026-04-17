#ifndef __POPUP_SUB_UI_H__
#define __POPUP_SUB_UI_H__

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
#include "window_screen_popup.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_low_power_ui_animation(ui_proc_popup_t *proc);
void stop_low_power_ui_animation(ui_proc_popup_t *proc);

void start_system_will_power_off_ui_animation(ui_proc_popup_t *proc);
void stop_system_will_power_off_ui_animation(ui_proc_popup_t *proc);

void start_count_down_ui_animation(ui_proc_popup_t *proc);
void stop_count_down_ui_animation(ui_proc_popup_t *proc);

void popup_window_exit_handler_cb(ui_proc_popup_t *proc);

void start_ble_status_ui_animation(ui_proc_popup_t *proc, uint16_t popup_ui_type);
#ifdef __cplusplus
}
#endif

#endif 


