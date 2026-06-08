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
#define BATTERY_ANIMATION_FREQ                    (25)
#else
#define BATTERY_ANIMATION_FREQ                    (5)
#endif


/// 电池UI动作枚举
typedef enum {
    BATTERY_HDR_ACTION_LOW_POWER_POWER_OFF = 100,
    BATTERY_HDR_ACTION_LOW_POWER_EXIT = 101,
    BATTERY_HDR_ACTION_MAX
} battery_ui_hdr_action_e;

/// 电池UI类型枚举
typedef enum {
    BATTERY_UI_TYPE_NONE = 0,
    BATTERY_UI_TYPE_LOW_POWER = 100,
    BATTERY_UI_TYPE_MAX
} battery_ui_type_e;

/// 电池UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    power_manager_context_t *pm;
    battery_manager_context_t *battery;
    airplane_mode_context_t *airplane;
    broadcast_obj_t receiver;
    window_broadcast_args_t broadcast_args; /// 广播投递参数.
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint16_t current_ui_type; /// 当前UI类型. battery_ui_type_e
    uint8_t window_run_state; /// WM是否运行.
} ui_proc_battery_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_battery_low_power_ui(ui_proc_battery_t *proc);
void stop_battery_low_power_ui(ui_proc_battery_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


