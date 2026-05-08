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
    CHARGE_HDR_EXIT_CHARGE_POLICY = 100, /// 退出充电策略
    CHARGE_HDR_SCREEN_TIMEOUT_DETECT, /// 充电界面超时检测
    CHARGE_HDR_DISCHARGE_POWER_OFF, /// 拔除充电关机
    CHARGE_HDR_CHARGE_SCREEN_ON,
    CHARGE_HDR_ACTION_MAX
} charge_ui_hdr_action_e;

/// 充电动UI类型枚举
typedef enum {
    CHARGE_UI_TYPE_NONE = 0,
    CHARGE_UI_TYPE_CHARGING = 100,
    CHARGE_UI_TYPE_ADAPTER,
    CHARGE_UI_TYPE_POWER_OFF_CHARGING,
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
    led_manager_context_t *led;
    airplane_mode_context_t *airplane;
    power_manager_context_t *pm;
    ble_manager_context_t *ble;
    broadcast_obj_t receiver;
    uint32_t margic;
    uint32_t last_progress_ani_time; /// 上次进度动画时间.
    uint8_t exit_charge_window_flag; /// 需要退出充电窗口标志.
    uint16_t current_ui_type; /// 当前UI类型. charge_ui_type_e
    uint8_t is_charge_screen_on; /// 是否充电动画展示中
    uint32_t charge_screen_on_time; /// 充电动画关闭时间.
} ui_proc_charge_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void show_charging_ui(ui_proc_charge_t *proc);
void show_charge_adapter_ui(ui_proc_charge_t *proc);
void show_power_off_charging_ui(ui_proc_charge_t *proc);
void stop_charge_common_ui(ui_proc_charge_t *proc);
int switch_charge_night_light_onoff_ui(ui_proc_charge_t *proc);

#ifdef __cplusplus
}
#endif

#endif 


