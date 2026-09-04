#ifndef __WINDOW_SCREEN_POWER_ON_GUIDE_H__
#define __WINDOW_SCREEN_POWER_ON_GUIDE_H__

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

/// power_on_guide界面动画频率
#if IS_SUPPORT_LVGL
#define POWER_ON_GUIDE_ANIMATION_FREQ                 (25)
#else
#define POWER_ON_GUIDE_ANIMATION_FREQ                 (5)
#endif

/// power_on_guide界面动画展示时间
#define POWER_ON_GUIDE_ANIMATION_DISPLAY_TIME         (2000) /// 2s

/// power_on_guide UI动作枚举
typedef enum {
    POWER_ON_GUIDE_HDR_NIGHT_LIGHT_ONOFF = 100,
    POWER_ON_GUIDE_HDR_NIGHT_LIGHT_LEVEL,
    POWER_ON_GUIDE_HDR_NIGHT_LIGHT_ADAPTIVE,
    POWER_ON_GUIDE_HDR_ACTION_MAX
} power_on_guide_ui_hdr_action_e;

/// power_on_guide UI类型枚举
typedef enum {
    POWER_ON_GUIDE_UI_TYPE_NONE = 0,
    POWER_ON_GUIDE_UI_TYPE_HOME,
    POWER_ON_GUIDE_UI_TYPE_MAX
} power_on_guide_ui_type_e;

/// power_on_guide UI结构体
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
    uint32_t enter_power_on_guide_time; /// 进入power_on_guide时间.
    uint32_t last_progress_ani_time; /// 上次动画进度时间.
    uint16_t current_ui_type; /// 当前UI类型. power_on_guide_ui_type_e
} ui_proc_power_on_guide_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


