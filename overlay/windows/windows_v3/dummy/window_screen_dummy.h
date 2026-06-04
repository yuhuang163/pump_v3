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

/// dummy界面动画频率
#if IS_SUPPORT_LVGL
#define DUMMY_ANIMATION_FREQ                 (25)
#else
#define DUMMY_ANIMATION_FREQ                 (5)
#endif

/// dummy界面动画展示时间
#define DUMMY_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// dummy UI动作枚举
typedef enum {
    DUMMY_HDR_NIGHT_LIGHT_ONOFF = 100,
    DUMMY_HDR_NIGHT_LIGHT_LEVEL,
    DUMMY_HDR_NIGHT_LIGHT_ADAPTIVE,
    DUMMY_HDR_ACTION_MAX
} dummy_ui_hdr_action_e;

/// dummy UI类型枚举
typedef enum {
    DUMMY_UI_TYPE_NONE = 0,
    DUMMY_UI_TYPE_HOME,
    DUMMY_UI_TYPE_MAX
} dummy_ui_type_e;

/// dummy UI结构体
typedef struct {
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    power_manager_context_t *pm;
    key_input_context_t *key;
    tests_manager_context_t *tests;
    led_manager_context_t *led;
    ble_manager_context_t *ble;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic; /// 窗口标识.
    uint8_t window_run_state; /// WM是否运行.
    uint32_t enter_dummy_time; /// 进入dummy时间.
    uint32_t last_progress_ani_time; /// 上次动画进度时间.
    uint16_t current_ui_type; /// 当前UI类型. dummy_ui_type_e
} ui_proc_dummy_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


