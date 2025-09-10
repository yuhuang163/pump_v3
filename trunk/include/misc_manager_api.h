#ifndef __MISC_MANAGER_API_H__
#define __MISC_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h"
// #include "system_persist_data.h"

#define EMPTY_BAT_PERCENTAGE                                  5
#define LOW_BAT_PERCENTAGE                                    20

// 描述充电相关
typedef void (*on_charge_state_change_fptr)(void *wm, charge_state_e new_state);
typedef void (*on_battery_state_change_fptr)(void *wm, int is_low_battery);

// 描述电池相关
typedef struct {
// private
    uint8_t charge_ui_has_show;
    uint8_t percentage;
    uint8_t low_battery;
    uint8_t battery_ui_has_show;   // 0 未显示；1 正在显示；2 本次唤醒提醒过；
    on_battery_state_change_fptr m_battery_listener_cb;
    on_charge_state_change_fptr m_charge_listener_cb;
    rt_device_node_t *device;

// public
    uint8_t (*is_charge_plug_in)(void);
    void (*set_charge_ui_show_flag)(int flag);
    uint8_t (*show_charge_ui_if_need)(void *wm, int is_fg);
    void (*set_state_change_listener)(on_charge_state_change_fptr cb);

    uint8_t (*is_low_battery)(void);
    uint8_t (*is_empty_battery)(void);
    uint8_t (*get_battery_percentage)(void);
    void (*set_battery_ui_show_flag)(int flag);
    uint8_t (*show_battery_ui_if_need)(void *wm, int is_fg);
    void (*set_state_battery_listener)(on_battery_state_change_fptr cb);
} battery_context_t;

// 描述misc杂项管理上下文对象
typedef struct misc_manager_context {
// public:
    battery_context_t *(*get_battery_obj)(void);
    void (*set_breath_pump_head_ui_show_flag)(int flag);
    int (*get_breath_pump_head_used_day)(sys_persist_info_t *info, int *days);
    int (*show_breath_pump_head_ui_if_need)(void);
    sys_board_config_t *(*get_sys_board_config)(void);

// private:
    thread_context_t misc_work_thd;
    os_timer_id misc_timer;
    battery_context_t battery;
    // board信息.
    sys_board_config_t board_cfg;
    // breath pump head usd day.
    int breath_pump_head_ui_show; 
    int breath_pump_head_used_time;
} misc_manager_context_t;

#ifdef __cplusplus
}
#endif

#endif 


