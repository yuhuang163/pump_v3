#ifndef __WINDOW_SCREEN_CHARGE_H__
#define __WINDOW_SCREEN_CHARGE_H__

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
#if IS_SUPPORT_LVGL
#define CHARGE_ANIMATION_FREQ                    (25)
#else
#define CHARGE_ANIMATION_FREQ                    (5)
#endif

/// 充电动画更新间隔
#define CHARGE_ANIMATION_UPDATE_INTERVAL         (1000)

/// 充电动画展示时间
typedef enum {
    CHARGE_HDR_EXIT_CHARGE_POLICY = 100,
    CHARGE_HDR_SCREEN_TIMEOUT_DETECT,
    CHARGE_HDR_CHARGE_SCREEN_ON,
    CHARGE_HDR_ACTION_MAX
} charge_ui_hdr_action_e;

/// 充电动UI类型枚举
typedef enum {
    CHARGE_UI_TYPE_NONE = 0,
    CHARGE_UI_TYPE_CHARGING = 100,
    CHARGE_UI_TYPE_UNCHARGING,
    CHARGE_UI_TYPE_MAX
} charge_ui_type_e;

/// 充电动UI结构体
typedef struct {    
    uint32_t ani_timer_cnt;
    window_context_t w_ctx;
    window_manager_context_t *wm;
    ui_manager_context_t *ui;
    battery_manager_context_t *battery;
    key_input_context_t *key;
    peripheral_bus_context_t *pbus;
    storage_manager_context_t *st;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t goto_pump_window_flag; /// 是否需要跳转到泵奶窗口.
    uint16_t current_ui_type; /// 当前UI类型. charge_ui_type_e
    uint8_t is_charge_screen_on; /// 是否充电动画展示中
    uint32_t charge_screen_on_time; /// 充电动画关闭时间.
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


