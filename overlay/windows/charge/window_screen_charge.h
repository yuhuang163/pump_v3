#ifndef __WINDOW_SCREEN_CHARGE_H__
#define __WINDOW_SCREEN_CHARGE_H__

#include <stdint.h>
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

/// 充电动画频率
#define CHARGE_ANIMATION_FREQ                    (12)
/// 充电动画更新间隔
#define CHARGE_ANIMATION_UPDATE_INTERVAL         (1000)

typedef enum {
    HDR_ACTION_REFRESH_CHARGE_UI = 0,
    HDR_ACTION_MAX
} charge_ui_hdr_action_e;

typedef enum {
    CHARGE_UI_TYPE_CHARGING = 100,
    CHARGE_UI_TYPE_UNCHARGING,
    CHARGE_UI_TYPE_MAX
} charge_ui_type_e;

typedef struct ui_proc_charge {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t goto_pump_window_flag; /// 是否需要跳转到泵奶窗口.
} ui_proc_charge_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_charging_ui(ui_proc_charge_t *proc);
void stop_charging_ui(ui_proc_charge_t *proc);
int switch_charge_night_light_onoff_ui(ui_proc_charge_t *proc);
#ifdef __cplusplus
}
#endif

#endif 


