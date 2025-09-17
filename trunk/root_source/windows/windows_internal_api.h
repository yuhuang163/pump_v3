#ifndef __WINDOWS_INTERNAL_API_H__
#define __WINDOWS_INTERNAL_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include <stdio.h> // IWYU pragma: keep
#include <stdlib.h> // IWYU pragma: keep
#include "windows_manager_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "windows_map_define.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

/// 窗口调度相关.
int dispatch_new_window(window_context_t *ctx);
void window_dispatch_lock(const char *tag);
void window_dispatch_unlock(const char *tag);
int go_back_to_history_wrapper(void *wm, window_entry_extra_t *__extra);
uint8_t to_valid_window_id(void *disp, uint8_t window_id);

/// 历史记录相关.
void window_flush_thread(const void *args);

/// flush 相关.
int post_msg_to_flush(const char *tag, uint8_t *msg);
void local_flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t *color_p);
void local_clear(uint16_t color);
void window_flush_post_init(window_manager_context_t *wm);
void window_flush_perpare_init(window_manager_context_t *wm);

/// 状态相关.
void set_is_playing_rt_ani(bool flag);
bool is_window_backlight_on(void);
bool is_on_window_dispatch(void);
bool is_current_window(uint8_t window_id);
bool is_the_same_window(uint8_t new_window_id);
bool is_valid_window_id(uint8_t window_id); 
bool is_window_deployed(void);
void set_window_deployed_flag(void);
bool window_system_is_ready(void);
void window_boot_completed(void);
void set_window_private_context(uint8_t window_id, void *context);
void *get_window_private_context(uint8_t window_id);
const void *get_window_entry_extra(void);
void set_window_backlight(uint16_t val);
void set_system_is_restting_device(bool val);
uint8_t get_system_is_restting_device(void);
int set_window_switch_forbid(int forbid);
void clear_window_go_back_history_flag(void);
void on_perpare_window_state_init(window_manager_context_t *wm);

#ifdef __cplusplus
}
#endif

#endif 


