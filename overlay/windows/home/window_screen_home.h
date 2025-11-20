#ifndef __WINDOW_SCREEN_HOME_H__
#define __WINDOW_SCREEN_HOME_H__

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
    // ui_page_handle_t bridge;
    uint32_t margic; /// 窗口标识.
} ui_proc_home_t;


typedef enum {
    HDR_ACTION_MAX
} home_ui_hdr_action_e;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void start_home_ui_animation(ui_proc_home_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


