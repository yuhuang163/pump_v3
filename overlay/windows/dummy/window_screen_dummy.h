#ifndef __WINDOW_SCREEN_DUMMY_H__
#define __WINDOW_SCREEN_DUMMY_H__

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

typedef enum {
    HDR_ACTION_NIGHT_LIGHT_ONOFF = 100,
    HDR_ACTION_NIGHT_LIGHT_LEVEL,
    HDR_ACTION_NIGHT_LIGHT_ADAPTIVE,
    HDR_ACTION_MAX
} dummy_ui_hdr_action_e;

typedef struct ui_proc_dummy {
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    broadcast_obj_t receiver;
    uint32_t margic; /// 窗口标识.
    uint32_t enter_dummy_time; /// 进入dummy时间.
} ui_proc_dummy_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
int switch_dummy_night_light_onoff_ui(ui_proc_dummy_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


