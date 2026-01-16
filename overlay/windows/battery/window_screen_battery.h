#ifndef __WINDOW_SCREEN_BATTERY_H__
#define __WINDOW_SCREEN_BATTERY_H__

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

/// 电池动画频率
#if IS_SUPPORT_LVGL
#define BATTERY_ANIMATION_FREQ                    (12)
#else
#define BATTERY_ANIMATION_FREQ                    (5)
#endif

/// 电池动画更新间隔
#define BATTERY_ANIMATION_UPDATE_INTERVAL         (1000)

/// 电池UI动作枚举
typedef enum {
    BATTERY_HDR_ACTION_REFRESH_CHARGE_UI = 0,
    BATTERY_HDR_ACTION_MAX
} battery_ui_hdr_action_e;

/// 电池UI类型枚举
typedef enum {
    BATTERY_UI_TYPE_NONE = 0,
    BATTERY_UI_TYPE_MAX
} battery_ui_type_e;

/// 电池UI结构体
typedef struct ui_proc_battery {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. battery_ui_type_e
} ui_proc_battery_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif 


